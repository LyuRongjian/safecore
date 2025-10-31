/*
 * safecore_priority.c
 * 
 * SafeCore Priority Queue Implementation
 * This module implements the priority-based event queue functionality for the SafeCore framework,
 * allowing events to be processed based on their priority levels.
 */
#include "safecore_priority.h"
#include "safecore_port.h"
#include "safecore_config.h"
#include "safecore_module_config.h"
#include "safecore_core.h"
#include <string.h>

#if SAFECORE_PRIORITY_ENABLED == 1

/* Multi-priority queues */
static uint8_t g_event_queues[SAFECORE_EVENT_PRIORITIES][SAFECORE_EVENT_QUEUE_SIZE][SAFECORE_MAX_EVENT_SIZE];
static uint8_t g_event_sizes[SAFECORE_EVENT_PRIORITIES][SAFECORE_EVENT_QUEUE_SIZE];
static volatile uint8_t g_heads[SAFECORE_EVENT_PRIORITIES];
static volatile uint8_t g_tails[SAFECORE_EVENT_PRIORITIES];
static uint32_t g_dropped_events[SAFECORE_EVENT_PRIORITIES] = {0};

/* Subscriber table (obtained from core module) */
extern subscriber_entry_t g_subscribers[SAFECORE_MAX_SUBSCRIBERS];
extern uint8_t g_subscriber_count;

/**
 * @brief Check if a priority queue is full
 * 
 * Determines if the specified priority queue has reached its maximum capacity.
 * Uses modulo arithmetic to handle the circular buffer structure.
 * 
 * @param priority Priority level to check
 * @return uint8_t 1 if the queue is full, 0 otherwise
 */
SAFECORE_INLINE uint8_t queue_full(uint8_t priority) {
    /* Use unsigned suffix to avoid integer overflow */
    return ((g_heads[priority] + 1U) % SAFECORE_EVENT_QUEUE_SIZE) == g_tails[priority];
}

/**
 * @brief Check if a priority queue is empty
 * 
 * Determines if the specified priority queue contains any events.
 * 
 * @param priority Priority level to check
 * @return uint8_t 1 if the queue is empty, 0 otherwise
 */
SAFECORE_INLINE uint8_t queue_empty(uint8_t priority) {
    return g_heads[priority] == g_tails[priority];
}

/**
 * @brief Push data into a priority queue
 * 
 * Adds an event to the specified priority queue, handling overflow according to the configured policy.
 * 
 * @param priority Priority level to push to
 * @param data Pointer to the event data to copy
 * @param size Size of the event data
 * @return int 0 on success, -1 on failure (invalid parameters or queue full)
 */
SAFECORE_INLINE int queue_push(uint8_t priority, const uint8_t *data, size_t size) {
    int result = -1;
    
    if ((data != NULL) && (size > 0U) && (size <= SAFECORE_MAX_EVENT_SIZE) && (priority < SAFECORE_EVENT_PRIORITIES)) {
        if (queue_full(priority)) {
#if SAFECORE_QUEUE_OVERFLOW_POLICY == SAFECORE_QUEUE_DROP_OLDEST
            /* Drop oldest event by advancing tail */
            g_tails[priority] = (g_tails[priority] + 1) % SAFECORE_EVENT_QUEUE_SIZE;
            g_dropped_events[priority]++;
#elif SAFECORE_QUEUE_OVERFLOW_POLICY == SAFECORE_QUEUE_PANIC
            /* Panic on overflow */
            SAFECORE_ON_ERROR("Event queue overflow - PANIC");
            while (1);
#else
            /* Default policy: drop new event */
            g_dropped_events[priority]++;
            return -1;
#endif
        }

        /* Copy event data to queue */
        (void)memcpy(g_event_queues[priority][g_heads[priority]], data, size);
        g_event_sizes[priority][g_heads[priority]] = (uint8_t)size;
        /* Update head pointer (with wrap-around using modulo) */
        g_heads[priority] = (uint8_t)((g_heads[priority] + 1U) % SAFECORE_EVENT_QUEUE_SIZE);
        result = 0;
    }
    
    return result;
}

/**
 * @brief Pop data from a priority queue
 * 
 * Retrieves and removes the oldest event from the specified priority queue.
 * 
 * @param priority Priority level to pop from
 * @param out_size Pointer to store the size of the popped event
 * @return const uint8_t* Pointer to the popped event data, or NULL if the queue is empty
 */
SAFECORE_INLINE const uint8_t* queue_pop(uint8_t priority, size_t *out_size) {
    const uint8_t *e = NULL;
    
    if ((priority < SAFECORE_EVENT_PRIORITIES) && (out_size != NULL)) {
        if (!queue_empty(priority)) {
            *out_size = g_event_sizes[priority][g_tails[priority]];
            e = g_event_queues[priority][g_tails[priority]];
            /* Update tail pointer (with wrap-around using modulo) */
            g_tails[priority] = (uint8_t)((g_tails[priority] + 1U) % SAFECORE_EVENT_QUEUE_SIZE);
        }
    }
    
    return e;
}

/**
 * @brief Initialize priority queue system
 * 
 * Resets all priority queues to their initial empty state.
 * Sets all head and tail pointers to zero and initializes drop counters.
 */
void sc_priority_init(void) {
    uint8_t i;
    for (i = 0U; i < SAFECORE_EVENT_PRIORITIES; i++) {
        g_heads[i] = 0U;
        g_tails[i] = 0U;
        g_dropped_events[i] = 0U;
    }
}

/**
 * @brief Publish an event with raw data
 * 
 * Adds an event to the appropriate priority queue based on the event's priority field.
 * Applies event filtering if enabled.
 * 
 * @param event_data Pointer to the raw event data
 * @param size Size of the event data
 * @return int 0 on success, -1 on failure (invalid parameters or queue error)
 */
int sc_priority_publish_raw(const uint8_t *event_data, size_t size) {
    int result = -1;
    
    if ((event_data != NULL) && (size > 0U)) {
        const sc_event_t *e = (const sc_event_t *)event_data;
        
        if (e->id < SAFECORE_MAX_EVENT_TYPES) {
            uint8_t priority = e->priority;
            
            /* Validate priority - fallback to low priority if out of range */
            if (priority >= SAFECORE_EVENT_PRIORITIES) {
                priority = SAFECORE_LOW_PRIORITY;
            }

#if SAFECORE_FILTERS_ENABLED == 1
            /* Apply event filtering if enabled */
            if (sc_filters_check_event(e)) {
                result = queue_push(priority, event_data, size);
            } else {
                SC_LOG("Event %d filtered out", (int)e->id);
                result = 0; /* Filtered events are considered 'handled' */
            }
#else
            /* No filtering - push directly to queue */
            result = queue_push(priority, event_data, size);
#endif
        }
    }
    
    return result;
}

/**
 * @brief Process events from all priority queues
 * 
 * Processes events from all priority queues in order of priority (lowest to highest).
 * For each priority level, processes up to SAFECORE_MAX_EVENTS_PER_CYCLE events.
 * Delivers each event to all subscribers registered for that event type.
 */
void sc_priority_process(void) {
    uint8_t prio;
    
    /* Process from highest to lowest priority */
    for (prio = 0U; prio < SAFECORE_EVENT_PRIORITIES; prio++) {
        size_t size;
        const uint8_t *raw;
        uint8_t processed = 0U;

        do {
            /* Get next event from current priority queue */
            raw = queue_pop(prio, &size);
            if ((raw != NULL) && (processed < SAFECORE_MAX_EVENTS_PER_CYCLE)) {
                const sc_event_t *e = (const sc_event_t *)raw;
                uint8_t i;
                
                /* Deliver event to all matching subscribers */
                for (i = 0U; i < g_subscriber_count; i++) {
                    if (g_subscribers[i].event_id == e->id) {
                        if (g_subscribers[i].callback != NULL) {
                            g_subscribers[i].callback(e, g_subscribers[i].ctx);
                        } else {
                            SAFECORE_ON_ERROR("Null subscriber callback!");
                        }
                    }
                }
                processed++;
            }
        } while ((raw != NULL) && (processed < SAFECORE_MAX_EVENTS_PER_CYCLE));
    }
}

/**
 * @brief Get the current depth of a priority queue
 * 
 * Calculates the number of events currently in the specified priority queue.
 * Handles the circular buffer structure to correctly calculate depth.
 * 
 * @param priority Priority level to check
 * @return uint8_t Current depth of the priority queue
 */
uint8_t sc_priority_get_queue_depth(uint8_t priority) {
    uint8_t depth = 0U;
    
    if (priority < SAFECORE_EVENT_PRIORITIES) {
        /* Use unsigned arithmetic to avoid sign issues */
        if (g_heads[priority] >= g_tails[priority]) {
            depth = g_heads[priority] - g_tails[priority];
        } else {
            depth = (g_heads[priority] + SAFECORE_EVENT_QUEUE_SIZE) - g_tails[priority];
        }
    }
    
    return depth;
}

/**
 * @brief Get statistics for all priority queues
 * 
 * Retrieves depth and dropped event count for all priority levels.
 * 
 * @param depths Pointer to an array to store queue depths
 * @param dropped Pointer to an array to store dropped event counts
 */
void sc_priority_get_stats(uint8_t *depths, uint32_t *dropped) {
    uint8_t i;
    
    if (depths != NULL) {
        for (i = 0U; i < SAFECORE_EVENT_PRIORITIES; i++) {
            depths[i] = sc_priority_get_queue_depth(i);
        }
    }
    
    if (dropped != NULL) {
        for (i = 0U; i < SAFECORE_EVENT_PRIORITIES; i++) {
            dropped[i] = g_dropped_events[i];
        }
    }
}

#endif /* SAFECORE_PRIORITY_ENABLED */