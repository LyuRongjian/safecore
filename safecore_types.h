/*
 * safecore_types.h
 * 
 * SafeCore Type Definitions
 * This file contains all the fundamental data types and structures used throughout
 * the SafeCore framework, including event types, state machine types, and callback definitions.
 */
#ifndef SAFECORE_TYPES_H
#define SAFECORE_TYPES_H

#include "safecore_config.h"
#include "safecore_module_config.h"
#include <stdint.h>

/* === Basic Event Types === */
/**
 * @brief State machine event type enumeration
 * 
 * This enumeration defines the standard event types used in state machines.
 * User-defined events should start from SC_EVENT_USER_START (16).
 */
typedef enum {
    SC_EVENT_INIT = 0,     /* Initialization event */
    SC_EVENT_ENTRY,        /* State entry event */
    SC_EVENT_EXIT,         /* State exit event */
    SC_EVENT_TICK,         /* Tick event for periodic operations */
    SC_EVENT_USER_START = 16 /* Base value for user-defined events */
} sc_sm_event_type_t;

/**
 * @brief State machine event structure
 * 
 * This structure represents an event in the state machine system. It contains
 * the event type, timestamp, and a union of possible data types.
 */
typedef struct {
    sc_sm_event_type_t type;    /* Event type */
    uint32_t timestamp;         /* Event timestamp */
    union {
        uint32_t u32;           /* Unsigned 32-bit data */
        int32_t s32;            /* Signed 32-bit data */
        void *ptr;              /* Pointer data */
        uint8_t bytes[8];       /* Raw byte data */
    } data;                     /* Event data */
} sc_sm_event_t;

/* === Common Event Base Class === */
/**
 * @brief Generic event structure
 * 
 * This structure represents a generic event in the SafeCore framework.
 * It contains the basic fields common to all events.
 */
typedef struct {
    uint32_t timestamp;      /* Event timestamp */
#if SAFECORE_PRIORITY_ENABLED == 1
    uint32_t timestampMicro; /* Microsecond portion of timestamp */
#endif
    uint8_t id;              /* Event ID */
    uint8_t size;            /* Actual data size */
#if SAFECORE_PRIORITY_ENABLED == 1
    uint8_t priority;        /* Event priority */
#else
    uint8_t reserved1;       /* Reserved field */
#endif
    uint8_t reserved2;       /* Alignment padding */
} sc_event_t;

/* === State Machine Related Types === */
/**
 * @brief State machine result type enumeration
 * 
 * This enumeration defines the possible return values from state machine handlers.
 */
typedef enum {
    SC_SM_HANDLED,       /* Event was handled in the current state */
    SC_SM_TRANSITION,    /* Transition to a new state is requested */
    SC_SM_SUPER          /* Delegate to the super state */
} sc_sm_result_t;

/**
 * @brief State machine handler function type
 * 
 * This type defines the function signature for state machine state handlers.
 * 
 * @param ctx Pointer to the state machine context
 * @param e Pointer to the event being processed
 * @param next_state Pointer to store the next state if transition is requested
 * @return Result of event processing
 */
typedef sc_sm_result_t (*sc_sm_handler_t)(void *ctx, const sc_sm_event_t *e, void **next_state);

/* === Subscriber Callback Type === */
/**
 * @brief Event subscriber callback function type
 * 
 * This type defines the function signature for event subscribers.
 * 
 * @param e Pointer to the event being processed
 * @param ctx Pointer to the subscriber context
 */
typedef void (*sc_subscriber_fn_t)(const sc_event_t *e, void *ctx);

/* === Priority Related Types === */
#if SAFECORE_PRIORITY_ENABLED == 1
/**
 * @brief Priority level enumeration
 * 
 * This enumeration defines the priority levels for events and messages.
 */
typedef enum {
    SC_PRIORITY_EMERGENCY = 0,  /* Highest priority, for critical events */
    SC_PRIORITY_STANDARD = 1,   /* Normal priority level */
    SC_PRIORITY_LOW = 2         /* Lowest priority level */
} sc_priority_type_t;
#endif

/* === Filter Types === */
#if SAFECORE_FILTERS_ENABLED == 1
/**
 * @brief Filter type enumeration
 * 
 * This enumeration defines the different types of filters that can be applied.
 */
typedef enum {
    SC_FILTER_TYPE_ALLOW,       /* Allow events that match criteria */
    SC_FILTER_TYPE_DENY,        /* Deny events that match criteria */
    SC_FILTER_TYPE_SIZE_MIN,    /* Filter by minimum size */
    SC_FILTER_TYPE_SIZE_MAX,    /* Filter by maximum size */
    SC_FILTER_TYPE_PRIORITY     /* Filter by priority level */
} sc_filter_type_t;

/**
 * @brief Filter rule structure
 * 
 * This structure defines a rule for filtering events based on various criteria.
 */
typedef struct {
    uint8_t enabled;            /* Whether the filter is enabled */
    sc_filter_type_t type;      /* Type of filter */
    uint8_t event_id;           /* Event ID to filter */
    uint8_t param;              /* Additional filter parameter */
} sc_filter_rule_t;
#endif

/* === Diagnostics Related Types === */
#if SAFECORE_DIAGNOSTICS_ENABLED == 1
/**
 * @brief Diagnostic trouble code severity enumeration
 * 
 * This enumeration defines the severity levels for diagnostic trouble codes.
 */
typedef enum {
    SC_DTC_INFO = 0,            /* Informational DTC */
    SC_DTC_WARNING = 1,         /* Warning DTC */
    SC_DTC_ERROR = 2,           /* Error DTC */
    SC_DTC_FATAL = 3            /* Fatal DTC */
} sc_dtc_severity_type_t;

/**
 * @brief Diagnostic trouble code structure
 * 
 * This structure represents a diagnostic trouble code with its associated information.
 */
typedef struct {
    uint16_t dtc_id;            /* DTC identifier */
    uint8_t severity;           /* DTC severity level */
    uint8_t status;             /* DTC status flags */
    uint32_t timestamp;         /* DTC occurrence timestamp */
} sc_dtc_type_t;
#endif

/* === Communication Related Types === */
#if SAFECORE_COMM_ENABLED == 1
/**
 * @brief CAN frame structure
 * 
 * This structure represents a CAN (Controller Area Network) frame.
 */
typedef struct {
    uint32_t can_id;            /* CAN identifier */
    uint8_t dlc;                /* Data length code */
    uint8_t data[8];            /* CAN frame data */
#if SAFECORE_PRIORITY_ENABLED == 1
    uint8_t priority;           /* Frame priority */
#endif
} sc_can_frame_type_t;
#endif

#endif /* SAFECORE_TYPES_H */