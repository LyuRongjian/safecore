/*
 * safecore_config.h
 * 
 * SafeCore Configuration File
 * This file contains all configuration options for the SafeCore framework
 * allowing customization of features, sizes, and behavior.
 */
#ifndef SAFECORE_CONFIG_H
#define SAFECORE_CONFIG_H

#include <stdint.h>

/* === Version Control === */
#define SAFECORE_VERSION_MAJOR    0
#define SAFECORE_VERSION_MINOR    0
#define SAFECORE_VERSION_PATCH    4

/* === Basic Framework Configuration === */
#define SAFECORE_BASIC_ENABLED               1   /* Basic event bus + state machine */
#define SAFECORE_MAX_HSM_DEPTH               4   /* HSM maximum depth */
#define SAFECORE_ENTRY_EXIT_ENABLED          1   /* State machine entry/exit events */
#define SAFECORE_EVENT_QUEUE_SIZE            32  /* Basic queue size (must be power of 2) */
#define SAFECORE_MAX_EVENT_SIZE              16  /* Maximum event size in bytes */
#define SAFECORE_MAX_SUBSCRIBERS             8   /* Maximum number of subscribers */
#define SAFECORE_MAX_EVENT_TYPES             16  /* Maximum number of event types */

/* === Priority Queue Configuration === */
#define SAFECORE_PRIORITY_ENABLED            1   /* Multiple priority queues */
#define SAFECORE_EVENT_PRIORITIES            3   /* 0=emergency, 1=standard, 2=low priority */
#define SAFECORE_EMERGENCY_PRIORITY          0
#define SAFECORE_STANDARD_PRIORITY           1
#define SAFECORE_LOW_PRIORITY                2
#define SAFECORE_MAX_EVENTS_PER_CYCLE        10  /* Maximum processed per priority per cycle */

/* === Event Filters Configuration === */
#define SAFECORE_FILTERS_ENABLED             1   /* Event filters */
#define SAFECORE_MAX_FILTER_RULES            8   /* Maximum number of filter rules */

/* === Diagnostics System Configuration === */
#define SAFECORE_DIAGNOSTICS_ENABLED         0   /* Diagnostics system (automotive grade) */
#define SAFECORE_MAX_DTCS                    128 /* Maximum DTC count */
#define SAFECORE_DTC_STORAGE_SIZE            256 /* DTC storage size */
#define SAFECORE_SELF_TEST_ENABLED           0   /* Self-test functionality */

/* === AUTOSAR Compatibility Configuration === */
#define SAFECORE_AUTOSAR_ENABLED             0   /* AUTOSAR BSW compatibility */
#define SAFECORE_AUTOSAR_VERSION_MAJOR       4
#define SAFECORE_AUTOSAR_VERSION_MINOR       4

/* === Communication Protocol Configuration === */
#define SAFECORE_COMM_ENABLED                0   /* Communication bridge */
#define SAFECORE_CAN_ENABLED                 0   /* CAN support */
#define SAFECORE_LIN_ENABLED                 0   /* LIN support */
#define SAFECORE_UDS_ENABLED                 0   /* UDS diagnostics support */

/* === Safety Mechanisms Configuration === */
#define SAFECORE_SAFETY_ENABLED              0   /* Safety mechanisms */
#define SAFECORE_WATCHDOG_INTEGRATION        0   /* Watchdog integration */
#define SAFECORE_MEMORY_PROTECTION           0   /* Memory protection */
#define SAFECORE_RUNTIME_CHECKS              0   /* Runtime checks */

/* === Performance and Safety Configuration === */
#define SAFECORE_QUEUE_OVERFLOW_POLICY       1   /* 0=drop newest, 1=drop oldest, 2=panic */
#define SAFECORE_MAX_PROCESS_TIME_MS         10  /* Main loop processing timeout */
#define SAFECORE_LOG_ENABLED                 1   /* Log output */

/* === MISRA-C 2012 Compliance === */
#define SAFECORE_MISRA_COMPLIANT             1   /* MISRA-C 2012 compliance */

/* === Queue Policy Macros === */
#define SAFECORE_QUEUE_DROP_NEWEST           0
#define SAFECORE_QUEUE_DROP_OLDEST           1
#define SAFECORE_QUEUE_PANIC                 2

/* === Safety Hook Macros === */
#ifndef SAFECORE_ON_ERROR
#define SAFECORE_ON_ERROR(msg) do { \
    extern void safecore_error_handler(const char*); \
    safecore_error_handler(msg); \
} while(0)
#endif

/* === Logging Macros === */
#if SAFECORE_LOG_ENABLED
#include <stdio.h>
#define SC_LOG(fmt, ...) printf("[SafeCore] " fmt "\n", ##__VA_ARGS__)
#else
#define SC_LOG(...)
#endif

/* === Compiler Optimization Hints === */
#if defined(__GNUC__) || defined(__clang__)
    #define SAFECORE_ALWAYS_INLINE __attribute__((always_inline))
    #define SAFECORE_PACKED __attribute__((packed))
#else
    #define SAFECORE_ALWAYS_INLINE
    #define SAFECORE_PACKED
#endif

/* === Inline Function Hints === */
#define SAFECORE_INLINE static inline SAFECORE_ALWAYS_INLINE

/* === Compile-time Assertion for Critical Constraints === */
#include "safecore_module_config.h"

/* Ensure queue size is power of two */
SC_STATIC_ASSERT((SAFECORE_EVENT_QUEUE_SIZE & (SAFECORE_EVENT_QUEUE_SIZE - 1)) == 0, 
                 safecore_event_queue_size_must_be_power_of_two);

/* Ensure maximum event size is not zero */
SC_STATIC_ASSERT(SAFECORE_MAX_EVENT_SIZE > 0, 
                 safecore_max_event_size_must_be_greater_than_zero);

/* Ensure maximum subscribers count is not zero */
SC_STATIC_ASSERT(SAFECORE_MAX_SUBSCRIBERS > 0, 
                 safecore_max_subscribers_must_be_greater_than_zero);

/* Ensure maximum event types count is not zero */
SC_STATIC_ASSERT(SAFECORE_MAX_EVENT_TYPES > 0, 
                 safecore_max_event_types_must_be_greater_than_zero);

/* When priority is enabled, ensure priority count is within valid range */
#if SAFECORE_PRIORITY_ENABLED == 1
SC_STATIC_ASSERT(SAFECORE_EVENT_PRIORITIES > 0 && SAFECORE_EVENT_PRIORITIES <= 8, 
                 safecore_event_priorities_must_be_between_1_and_8);
#endif

/* When filters are enabled, ensure maximum filter rules count is not zero */
#if SAFECORE_FILTERS_ENABLED == 1
SC_STATIC_ASSERT(SAFECORE_MAX_FILTER_RULES > 0, 
                 safecore_max_filter_rules_must_be_greater_than_zero);
#endif

#endif /* SAFECORE_CONFIG_H */