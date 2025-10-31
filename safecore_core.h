/*
 * safecore_core.h
 * 
 * SafeCore Core Framework Interface
 * This file defines the core interfaces for the SafeCore framework,
 * including state machine processing and event bus functionality.
 */
#ifndef SAFECORE_CORE_H
#define SAFECORE_CORE_H

#include "safecore_types.h"
#include "safecore_config.h"

/**
 * @defgroup StateMachine State Machine Module
 * @brief Hierarchical State Machine (HSM) implementation
 * 
 * The state machine module provides a robust implementation of hierarchical
 * state machines for managing complex system behaviors in a deterministic manner.
 * @{*/
/**
 * @brief State Machine Structure
 * 
 * This structure represents a hierarchical state machine with support for
 * nested states, entry/exit actions, and event processing.
 */
typedef struct {
    sc_sm_handler_t handlers[SAFECORE_MAX_HSM_DEPTH];  /* Array of state handlers */
    void *contexts[SAFECORE_MAX_HSM_DEPTH];            /* Context pointers for states */
    uint8_t depth;                                     /* Current nesting depth */
    void *user_ctx;                                    /* User-defined context */
    const char *name;                                  /* Optional name for debugging */
} sc_state_machine_t;

/* === Subscriber Table Structure === */
/**
 * @brief Subscriber entry structure
 * 
 * This structure stores information about an event subscriber,
 * including the event ID to subscribe to, the callback function,
 * and the user context.
 */
typedef struct {
    uint8_t event_id;          /* Event ID to subscribe to */
    sc_subscriber_fn_t callback; /* Callback function for event processing */
    void *ctx;                 /* User context for the callback */
} subscriber_entry_t;

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
void sc_sm_init(sc_state_machine_t *sm, sc_sm_handler_t top, void *ctx, const char *name);
/**
 * @brief Dispatch an event to a state machine
 * 
 * This function processes an event through the state machine's hierarchy,
 * starting from the current depth and moving upward to the top-level state.
 * 
 * @param sm Pointer to the state machine
 * @param e Pointer to the event to dispatch
 */
void sc_sm_dispatch(sc_state_machine_t *sm, const sc_sm_event_t *e);
/**
 * @brief Send entry event to a state
 * 
 * This function sends an entry event to a specific state in the state machine,
 * triggering any entry actions defined for that state.
 * 
 * @param sm Pointer to the state machine
 * @param ctx Context pointer for the state
 */
void sc_sm_send_entry(sc_state_machine_t *sm, void *ctx);
/**
 * @brief Send exit event to a state
 * 
 * This function sends an exit event to a specific state in the state machine,
 * triggering any exit actions defined for that state.
 * 
 * @param sm Pointer to the state machine
 * @param ctx Context pointer for the state
 */
void sc_sm_send_exit(sc_state_machine_t *sm, void *ctx);

/** @} *//* End of StateMachine group */

/**
 * @defgroup EventBus Event Bus Module
 * @brief Event-based communication system
 * 
 * The event bus module provides a publish-subscribe mechanism for
 * inter-component communication in a decoupled and deterministic manner.
 * @{*/
#if SAFECORE_BASIC_ENABLED == 1

/**
 * @brief Initialize the event bus
 * 
 * This function initializes the event bus system, resetting subscribers
 * and queue pointers to their initial state.
 */
void sc_eventbus_init(void);
/**
 * @brief Subscribe to an event
 * 
 * This function registers a callback to receive notifications when
 * events with the specified ID are published.
 * 
 * @param event_id ID of the event to subscribe to
 * @param callback Function to call when the event is published
 * @param ctx Context pointer to pass to the callback
 * @return 0 on success, -1 on failure (invalid parameters or no slots available)
 */
int sc_eventbus_subscribe(uint8_t event_id, sc_subscriber_fn_t callback, void *ctx);
/**
 * @brief Publish an event with raw data
 * 
 * This function publishes an event using raw data, applying filters if enabled
 * and routing to the appropriate queue based on priority configuration.
 * 
 * @param event_data Pointer to the event data
 * @param size Size of the event data in bytes
 * @return 0 on success, -1 on failure (invalid parameters)
 */
int sc_eventbus_publish_raw(const uint8_t *event_data, size_t size);
/**
 * @brief Process events in the event bus
 * 
 * This function processes all pending events in the event bus, delivering
 * them to all matching subscribers. It also performs timeout checks.
 */
void sc_eventbus_process(void);

/**
 * @brief Compile-time checked event publish macro
 * 
 * This macro provides a type-safe way to publish events while performing
 * compile-time size validation to prevent oversized events.
 * 
 * @param evt_ptr Pointer to the event structure to publish
 */
#define SC_PUBLISH_EVENT(evt_ptr) \
    (SC_STATIC_ASSERT(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event_too_large"), \
     sc_eventbus_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))))

/** @} *//* End of EventBus group */
#endif /* SAFECORE_BASIC_ENABLED */

#endif /* SAFECORE_CORE_H */