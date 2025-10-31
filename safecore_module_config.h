#ifndef SAFECORE_MODULE_CONFIG_H
#define SAFECORE_MODULE_CONFIG_H

#include "safecore_config.h"

/* === 模块依赖检查 === */
/* 如果启用了高级功能，必须启用基础功能 */
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

/* === 汽车级配置检查 === */
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

/* === 内存优化配置 === */
#if SAFECORE_PRIORITY_ENABLED == 1
    #define SAFECORE_TOTAL_QUEUES             SAFECORE_EVENT_PRIORITIES
#else
    #define SAFECORE_TOTAL_QUEUES             1
#endif

/* === 编译时断言 === */
#if SAFECORE_MISRA_COMPLIANT == 1
    #define SC_STATIC_ASSERT(condition, msg) \
        typedef char msg[(condition) ? 1 : -1]
#else
    #define SC_STATIC_ASSERT(condition, msg) \
        _Static_assert(condition, #msg)
#endif

#endif /* SAFECORE_MODULE_CONFIG_H */