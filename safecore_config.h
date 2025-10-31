#ifndef SAFECORE_CONFIG_H
#define SAFECORE_CONFIG_H

#include <stdint.h>

/* === 版本控制 === */
#define SAFECORE_VERSION_MAJOR    4
#define SAFECORE_VERSION_MINOR    0
#define SAFECORE_VERSION_PATCH    0

/* === 基础框架配置（v2.0）=== */
#define SAFECORE_BASIC_ENABLED                1   /* 基础事件总线 + 状态机 */
#define SAFECORE_MAX_HSM_DEPTH                4   /* HSM 最大深度 */
#define SAFECORE_ENTRY_EXIT_ENABLED           1   /* 状态机 entry/exit */
#define SAFECORE_EVENT_QUEUE_SIZE             32  /* 基础队列大小 */
#define SAFECORE_MAX_EVENT_SIZE               16  /* 最大事件大小 */
#define SAFECORE_MAX_SUBSCRIBERS              8   /* 最大订阅者数 */
#define SAFECORE_MAX_EVENT_TYPES              16  /* 最大事件类型数 */

/* === 优先级队列配置（v3.0）=== */
#define SAFECORE_PRIORITY_ENABLED             1   /* 多优先级队列 */
#define SAFECORE_EVENT_PRIORITIES             3   /* 0=紧急,1=标准,2=低优先级 */
#define SAFECORE_EMERGENCY_PRIORITY           0
#define SAFECORE_STANDARD_PRIORITY            1
#define SAFECORE_LOW_PRIORITY                 2
#define SAFECORE_MAX_EVENTS_PER_CYCLE         10  /* 每优先级单次最大处理数 */

/* === 事件过滤配置（v3.0）=== */
#define SAFECORE_FILTERS_ENABLED              1   /* 事件过滤器 */
#define SAFECORE_MAX_FILTER_RULES             8   /* 最大过滤规则数 */

/* === 诊断系统配置（v4.0）=== */
#define SAFECORE_DIAGNOSTICS_ENABLED          0   /* 诊断系统（汽车级） */
#define SAFECORE_MAX_DTCS                     128 /* 最大 DTC 数量 */
#define SAFECORE_DTC_STORAGE_SIZE             256 /* DTC 存储空间 */
#define SAFECORE_SELF_TEST_ENABLED            0   /* 自检功能 */

/* === AUTOSAR 兼容配置（v4.0）=== */
#define SAFECORE_AUTOSAR_ENABLED              0   /* AUTOSAR BSW 兼容 */
#define SAFECORE_AUTOSAR_VERSION_MAJOR        4
#define SAFECORE_AUTOSAR_VERSION_MINOR        4

/* === 通信协议配置（v4.0）=== */
#define SAFECORE_COMM_ENABLED                 0   /* 通信桥接 */
#define SAFECORE_CAN_ENABLED                  0   /* CAN 支持 */
#define SAFECORE_LIN_ENABLED                  0   /* LIN 支持 */
#define SAFECORE_UDS_ENABLED                  0   /* UDS 诊断支持 */

/* === 安全机制配置（v4.0）=== */
#define SAFECORE_SAFETY_ENABLED               0   /* 安全机制 */
#define SAFECORE_WATCHDOG_INTEGRATION         0   /* 看门狗集成 */
#define SAFECORE_MEMORY_PROTECTION            0   /* 内存保护 */
#define SAFECORE_RUNTIME_CHECKS               0   /* 运行时检查 */

/* === 性能与安全配置 === */
#define SAFECORE_QUEUE_OVERFLOW_POLICY        1   /* 0=丢新,1=丢旧,2=恐慌 */
#define SAFECORE_MAX_PROCESS_TIME_MS          10  /* 主循环处理超时 */
#define SAFECORE_LOG_ENABLED                  1   /* 日志输出 */

/* === MISRA-C 2012 兼容性 === */
#define SAFECORE_MISRA_COMPLIANT              1   /* MISRA-C 2012 合规 */

/* === 队列策略宏定义 === */
#define SAFECORE_QUEUE_DROP_NEWEST            0
#define SAFECORE_QUEUE_DROP_OLDEST            1
#define SAFECORE_QUEUE_PANIC                  2

/* === 安全钩子宏 === */
#ifndef SAFECORE_ON_ERROR
#define SAFECORE_ON_ERROR(msg) do { \
    extern void safecore_error_handler(const char*); \
    safecore_error_handler(msg); \
} while(0)
#endif

/* === 日志宏 === */
#if SAFECORE_LOG_ENABLED
#include <stdio.h>
#define SC_LOG(fmt, ...) printf("[SafeCore] " fmt "\n", ##__VA_ARGS__)
#else
#define SC_LOG(...)
#endif

#endif /* SAFECORE_CONFIG_H */