/*
 * safecore_core.c
 * 
 * SafeCore Core Framework Implementation
 * This file implements the core functionality of the SafeCore framework,
 * including state machine processing and the event bus system for component communication.
 */
#include "safecore_core.h"
#include "safecore_port.h"
#include "safecore_config.h"
#include "safecore_module_config.h"
#include <string.h>

/* === State Machine Implementation === */

/**
 * @brief Initialize a state machine
 * 
 * This function initializes a state machine with the specified top-level state handler,
 * context pointer, and optional name.
 * 
 * @param sm Pointer to the state machine structure to initialize
 * @param top Top-level state handler function
 * @param ctx Context pointer to pass to state handlers
 * @param name Optional name for the state machine (used for debugging)
 */
void sc_sm_init(sc_state_machine_t *sm, sc_sm_handler_t top, void *ctx, const char *name) {
    /* Validate input parameters */
    if ((sm == NULL) || (top == NULL)) {
        SAFECORE_ON_ERROR("SM init: null pointer");
        return;
    }
    
    /* Initialize state machine structure */
    (void)memset(sm, 0, sizeof(*sm));
    sm->handlers[0] = top;
    sm->user_ctx = ctx;
    
    /* Set state machine name */
    if (name != NULL) {
        sm->name = name;
    } else {
        sm->name = "sm";
    }
}

/**
 * @brief Dispatch an event to a state machine
 * 
 * This function processes an event through the state machine's hierarchy,
 * starting from the current depth and moving upward to the top-level state.
 * 
 * @param sm Pointer to the state machine
 * @param e Pointer to the event to dispatch
 */
void sc_sm_dispatch(sc_state_machine_t *sm, const sc_sm_event_t *e) {
    /* Validate input parameters */
    if ((sm == NULL) || (e == NULL)) {
        SAFECORE_ON_ERROR("SM dispatch: null pointer");
        return;
    }
    
    /* Check for maximum depth violation */
    if (sm->depth >= SAFECORE_MAX_HSM_DEPTH) {
        SAFECORE_ON_ERROR("SM: max depth exceeded");
        return;
    }

    /* Copy event and set timestamp */
    sc_sm_event_t ev = *e;
    ev.timestamp = safecore_get_tick_ms();

    int depth = sm->depth;
    int i;
    
    /* Process event through state hierarchy from deepest to top */
    for (i = depth; i >= 0; i--) {
        sc_sm_handler_t handler = sm->handlers[i];
        
        /* Check for valid handler */
        if (handler == NULL) {
            SAFECORE_ON_ERROR("SM: null handler");
            return;
        }
        
        void *next = NULL;
        sc_sm_result_t r = handler(sm->contexts[i], &ev, &next);
        
        /* Handle different state return types */
        if (r == SC_SM_HANDLED) {
            /* Event was handled, no further processing needed */
            return;
        }
        
        if (r == SC_SM_TRANSITION) {
            /* Handle state transition */
            if (next == NULL) {
                SAFECORE_ON_ERROR("SM: transition to null state");
                return;
            }
            
            /* Send exit events for all current states */
            int j;
            for (j = sm->depth; j >= 0; j--) {
                if (sm->handlers[j] != NULL) {
                    sc_sm_send_exit(sm, sm->contexts[j]);
                }
            }
            
            /* Reset state machine to new state */
            sm->depth = 0;
            sm->handlers[0] = (sc_sm_handler_t)next;
            sm->contexts[0] = sm->user_ctx;
            
            /* Send entry event to new state */
            sc_sm_send_entry(sm, sm->contexts[0]);
            return;
        }
    }
}

/**
 * @brief Send an entry event to a state
 * 
 * This function sends an entry event to the specified state handler.
 * Only enabled if SAFECORE_ENTRY_EXIT_ENABLED is set to 1.
 * 
 * @param sm Pointer to the state machine
 * @param ctx Context pointer for the state
 */
void sc_sm_send_entry(sc_state_machine_t *sm, void *ctx) {
#if SAFECORE_ENTRY_EXIT_ENABLED == 1
    if ((sm != NULL) && (ctx != NULL)) {
        sc_sm_event_t e;
        e.type = SC_EVENT_ENTRY;
        e.timestamp = safecore_get_tick_ms();
        
        /* Send entry event to the current depth handler */
        if (sm->handlers[sm->depth] != NULL) {
            sm->handlers[sm->depth](ctx, &e, NULL);
        }
    }
#endif
}

/**
 * @brief Send an exit event to a state
 * 
 * This function sends an exit event to the specified state handler.
 * Only enabled if SAFECORE_ENTRY_EXIT_ENABLED is set to 1.
 * 
 * @param sm Pointer to the state machine
 * @param ctx Context pointer for the state
 */
void sc_sm_send_exit(sc_state_machine_t *sm, void *ctx) {
#if SAFECORE_ENTRY_EXIT_ENABLED == 1
    if ((sm != NULL) && (ctx != NULL)) {
        sc_sm_event_t e;
        e.type = SC_EVENT_EXIT;
        e.timestamp = safecore_get_tick_ms();
        
        /* Send exit event to the current depth handler */
        if (sm->handlers[sm->depth] != NULL) {
            sm->handlers[sm->depth](ctx, &e, NULL);
        }
    }
#endif
}

/* === Basic Event Bus Implementation === */
#if SAFECORE_BASIC_ENABLED == 1

/* === Global Variables === */
static subscriber_entry_t g_subscribers[SAFECORE_MAX_SUBSCRIBERS]; /* Array of subscribers */
static uint8_t g_subscriber_count = 0; /* Current number of subscribers */

/* === Single Queue Implementation (When Priority is Disabled) === */
#if SAFECORE_PRIORITY_ENABLED != 1
static uint8_t g_event_queue[SAFECORE_EVENT_QUEUE_SIZE][SAFECORE_MAX_EVENT_SIZE]; /* Event queue storage */
static volatile uint8_t g_head = 0; /* Queue head index */
static volatile uint8_t g_tail = 0; /* Queue tail index */

/**
 * @brief Check if the event queue is full
 * 
 * This function determines if the event queue is full by checking if the
 * next head position would collide with the tail position.
 * 
 * @return uint8_t 1 if the queue is full, 0 otherwise
 */
SAFECORE_INLINE uint8_t queue_full(void) {
    /* Using unsigned suffix to avoid integer overflow */
    return (uint8_t)((g_head + 1U) & (SAFECORE_EVENT_QUEUE_SIZE - 1U)) == g_tail;
}

/**
 * @brief Check if the event queue is empty
 * 
 * This function determines if the event queue is empty by checking if
 * head and tail positions are the same.
 * 
 * @return uint8_t 1 if the queue is empty, 0 otherwise
 */
SAFECORE_INLINE uint8_t queue_empty(void) {
    return g_head == g_tail;
}

/**
 * @brief Push an event to the queue
 * 
 * This function adds an event to the end of the queue, handling overflow
 * according to the configured policy.
 * 
 * @param data Pointer to the event data
 * @param size Size of the event data in bytes
 * @return int 0 on success, -1 on failure
 */
static inline int queue_push(const uint8_t *data, size_t size) {
    int result = -1;
    
    /* Validate input parameters */
    if ((data != NULL) && (size > 0U) && (size <= SAFECORE_MAX_EVENT_SIZE)) {
        /* Handle queue overflow according to configured policy */
        if (queue_full()) {
#if SAFECORE_QUEUE_OVERFLOW_POLICY == SAFECORE_QUEUE_DROP_OLDEST
            /* Drop oldest event by moving tail forward */
            g_tail = (uint8_t)((g_tail + 1U) % SAFECORE_EVENT_QUEUE_SIZE);
#elif SAFECORE_QUEUE_OVERFLOW_POLICY == SAFECORE_QUEUE_PANIC
            SAFECORE_ON_ERROR("Event queue overflow - PANIC");
            /* Safety shutdown - infinite loop */
            for (;;) {
                /* Safety shutdown */
            }
#endif
        }
        
        /* Copy event data to queue */
        (void)memcpy(g_event_queue[g_head], data, size);
        /* Update head pointer with wrap-around */
        g_head = (uint8_t)((g_head + 1U) % SAFECORE_EVENT_QUEUE_SIZE);
        result = 0;
    }
    
    return result;
}

/**
 * @brief Pop an event from the queue
 * 
 * This function retrieves and removes an event from the front of the queue.
 * 
 * @return const uint8_t* Pointer to the event data, or NULL if queue is empty
 */
SAFECORE_INLINE const uint8_t* queue_pop(void) {
    if (queue_empty()) return NULL;
    const uint8_t *e = g_event_queue[g_tail];
    g_tail = (g_tail + 1) % SAFECORE_EVENT_QUEUE_SIZE;
    return e;
}
#endif /* SAFECORE_PRIORITY_ENABLED != 1 */

/**
 * @brief Initialize the event bus
 * 
 * This function initializes the event bus system, resetting subscribers
 * and queue pointers.
 */
void sc_eventbus_init(void) {
    /* Clear subscriber list */
    (void)memset(g_subscribers, 0, sizeof(g_subscribers));
    g_subscriber_count = 0;
    
#if SAFECORE_PRIORITY_ENABLED != 1
    /* Reset queue pointers */
    g_head = g_tail = 0;
#endif

#if SAFECORE_FILTERS_ENABLED == 1U
    /* Initialize event filters if enabled */
    sc_filters_init();
    #endif
}

/**
 * @brief Subscribe to an event
 * 
 * This function registers a callback to receive notifications when
 * events with the specified ID are published.
 * 
 * @param event_id ID of the event to subscribe to
 * @param callback Function to call when the event is published
 * @param ctx Context pointer to pass to the callback
 * @return int 0 on success, -1 on failure
 */
int sc_eventbus_subscribe(uint8_t event_id, sc_subscriber_fn_t callback, void *ctx) {
    /* Validate input parameters and check for available slots */
    if (event_id >= SAFECORE_MAX_EVENT_TYPES || !callback || 
        g_subscriber_count >= SAFECORE_MAX_SUBSCRIBERS) {
        return -1;
    }
    
    /* Add subscriber to the list */
    g_subscribers[g_subscriber_count++] = (subscriber_entry_t){event_id, callback, ctx};
    return 0;
}

/**
 * @brief Publish an event with raw data
 * 
 * This function publishes an event using raw data, applying filters if enabled
 * and routing to the appropriate queue based on priority configuration.
 * 
 * @param event_data Pointer to the event data
 * @param size Size of the event data in bytes
 * @return int 0 on success, -1 on failure
 */
int sc_eventbus_publish_raw(const uint8_t *event_data, size_t size) {
    /* Validate input parameters */
    if (!event_data || size == 0U) return -1;
    
    /* Cast to event structure to check event ID */
    const sc_event_t *e = (const sc_event_t*)event_data;
    if (e->id >= SAFECORE_MAX_EVENT_TYPES) return -1;

#if SAFECORE_FILTERS_ENABLED == 1
    /* Apply event filtering if enabled */
    if (!sc_filters_check_event(e)) {
        SC_LOG("Event %d filtered out", e->id);
        return 0; /* Filtered out, but not an error */
    }
#endif

    /* Route to appropriate publishing mechanism based on priority configuration */
#if SAFECORE_PRIORITY_ENABLED == 1
    return sc_priority_publish_raw(event_data, size);
#else
    return queue_push(event_data, size);
#endif
}

/**
 * @brief Process events in the event bus
 * 
 * This function processes all pending events in the event bus, delivering
 * them to all matching subscribers. It also performs timeout checks.
 */
void sc_eventbus_process(void) {
    /* Record start time for timeout monitoring */
    uint32_t start = safecore_get_tick_ms();

    /* Process events using the appropriate mechanism */
#if SAFECORE_PRIORITY_ENABLED == 1
    sc_priority_process();
#else
    const uint8_t *raw;
    /* Process all events in the queue */
    while ((raw = queue_pop()) != NULL) {
        const sc_event_t *e = (const sc_event_t*)raw;
        
        /* Deliver event to all matching subscribers */
        for (uint8_t i = 0; i < g_subscriber_count; i++) {
            if (g_subscribers[i].event_id == e->id) {
                if (g_subscribers[i].callback) {
                    g_subscribers[i].callback(e, g_subscribers[i].ctx);
                } else {
                    SAFECORE_ON_ERROR("Null subscriber callback!");
                }
            }
        }
    }
#endif

    /* Check for processing timeout */
    uint32_t elapsed = safecore_get_tick_ms() - start;
    if (elapsed > SAFECORE_MAX_PROCESS_TIME_MS) {
        SAFECORE_ON_ERROR("Event processing timeout!");
    }
}

#endif /* SAFECORE_BASIC_ENABLED */