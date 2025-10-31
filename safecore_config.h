// safecore_config.h
#ifndef SAFECORE_CONFIG_H
#define SAFECORE_CONFIG_H

#include <stdint.h>

// === 状态机配置 ===
#define SAFECORE_MAX_HSM_DEPTH          4
#define SAFECORE_ENTRY_EXIT_ENABLED     1

// === 事件总线配置 ===
#define SAFECORE_EVENT_PRIORITIES       3   // 0=紧急, 1=标准, 2=低优先级
#define SAFECORE_EMERGENCY_PRIORITY     0
#define SAFECORE_STANDARD_PRIORITY      1
#define SAFECORE_LOW_PRIORITY           2

#define SAFECORE_EVENT_QUEUE_SIZE       32  // 每个优先级的队列大小
#define SAFECORE_MAX_EVENT_SIZE         16  // 最大事件大小
#define SAFECORE_MAX_SUBSCRIBERS        16  // 总订阅者数

// === 过滤配置 ===
#define SAFECORE_MAX_FILTER_RULES       8   // 最大过滤规则数
#define SAFECORE_MAX_EVENT_TYPES        32  // 最大事件类型数

// === 超时与安全 ===
#define SAFECORE_MAX_PROCESS_TIME_MS    10  // 主循环处理超时（ms）
#define SAFECORE_MAX_EVENTS_PER_CYCLE   10  // 每个优先级单次最多处理事件数

// === 队列溢出策略 ===
#define SC_QUEUE_DROP_NEWEST   0
#define SC_QUEUE_DROP_OLDEST   1
#define SC_QUEUE_PANIC         2
#define SAFECORE_QUEUE_OVERFLOW_POLICY  SC_QUEUE_DROP_OLDEST

// === 日志与错误处理 ===
#if !defined(SAFECORE_LOG_ENABLED)
#define SAFECORE_LOG_ENABLED 0
#endif

#if SAFECORE_LOG_ENABLED
#include <stdio.h>
#define SC_LOG(fmt, ...) printf("[SafeCore] " fmt "\n", ##__VA_ARGS__)
#else
#define SC_LOG(...)
#endif

#ifndef SAFECORE_ON_ERROR
#define SAFECORE_ON_ERROR(msg) do { \
    extern void safecore_error_handler(const char*); \
    safecore_error_handler(msg); \
} while(0)
#endif

#endif // SAFECORE_CONFIG_H