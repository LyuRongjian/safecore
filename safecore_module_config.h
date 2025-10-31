/*
 * safecore_module_config.h
 * 
 * SafeCore Module Configuration
 * This file contains configuration checks and assertions for SafeCore modules
 * and ensures proper dependencies between modules.
 */
#ifndef SAFECORE_MODULE_CONFIG_H
#define SAFECORE_MODULE_CONFIG_H

#include "safecore_config.h"

/* === Module Dependency Checks === */
/* Advanced features require basic functionality to be enabled */
#if SAFECORE_PRIORITY_ENABLED == 1 && SAFECORE_BASIC_ENABLED != 1
    #error "Priority queue requires basic framework"
#endif

#if SAFECORE_FILTERS_ENABLED == 1 && SAFECORE_BASIC_ENABLED != 1
    #error "Event filters require basic framework"
#endif

#if SAFECORE_DIAGNOSTICS_ENABLED == 1 && SAFECORE_BASIC_ENABLED != 1
    #error "Diagnostics require basic framework"
#endif

#if SAFECORE_COMM_ENABLED == 1 && SAFECORE_BASIC_ENABLED != 1
    #error "Communication bridge requires basic framework"
#endif

#if SAFECORE_SAFETY_ENABLED == 1 && SAFECORE_BASIC_ENABLED != 1
    #error "Safety mechanisms require basic framework"
#endif

/* === Automotive Configuration Checks === */
#if SAFECORE_AUTOSAR_ENABLED == 1
    #undef SAFECORE_DIAGNOSTICS_ENABLED
    #define SAFECORE_DIAGNOSTICS_ENABLED      1
    #undef SAFECORE_COMM_ENABLED
    #define SAFECORE_COMM_ENABLED             1
    #undef SAFECORE_SAFETY_ENABLED
    #define SAFECORE_SAFETY_ENABLED           1
    #undef SAFECORE_SELF_TEST_ENABLED
    #define SAFECORE_SELF_TEST_ENABLED        1
#endif

/* === Memory Optimization Configuration === */
#if SAFECORE_PRIORITY_ENABLED == 1
    #define SAFECORE_TOTAL_QUEUES             SAFECORE_EVENT_PRIORITIES
#else
    #define SAFECORE_TOTAL_QUEUES             1
#endif

/* === Compile-time Assertions === */
#if SAFECORE_MISRA_COMPLIANT == 1
    /* MISRA-C compliant static assertion implementation */
    /* This creates an array with negative size if condition is false, causing compile error */
    #define SC_STATIC_ASSERT(condition, msg) \
        extern int dummy_array_##msg[1U - 2U * ((condition) ? 0U : 1U)]
#else
    /* Use C11 _Static_assert when not enforcing MISRA compliance */
    #define SC_STATIC_ASSERT(condition, msg) \
        _Static_assert(condition, #msg)
#endif

#endif /* SAFECORE_MODULE_CONFIG_H */