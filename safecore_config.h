// safecore_config.h
#ifndef SAFECORE_CONFIG_H
#define SAFECORE_CONFIG_H

#include <stdint.h>

// === 状态机 ===
#define SAFECORE_MAX_HSM_DEPTH          4
#define SAFECORE_ENTRY_EXIT_ENABLED     1

// === 事件系统 ===
#define SAFECORE_MAX_EVENT_TYPES        32
#define SAFECORE_MAX_SUBSCRIBERS        16
#define SAFECORE_MAX_EVENT_SIZE         24  // 最大事件结构体字节数

// === 优先级配置（0 = 最高）===
#define SAFECORE_EVENT_PRIORITIES       3
#define SAFECORE_PRIORITY_HIGH          0
#define SAFECORE_PRIORITY_MEDIUM        1
#define SAFECORE_PRIORITY_LOW           2

// === 各优先级队列大小（可差异化）===
#ifndef SAFECORE_QUEUE_SIZE_HIGH
#define SAFECORE_QUEUE_SIZE_HIGH        8   // 高优先级：小但快
#endif
#ifndef SAFECORE_QUEUE_SIZE_MEDIUM
#define SAFECORE_QUEUE_SIZE_MEDIUM      16
#endif
#ifndef SAFECORE_QUEUE_SIZE_LOW
#define SAFECORE_QUEUE_SIZE_LOW         32  // 低优先级：大但可丢
#endif

// === 队列溢出策略（按优先级独立配置）===
#define SC_QUEUE_DROP_NEWEST   0
#define SC_QUEUE_DROP_OLDEST   1
#define SC_QUEUE_PANIC         2

#define SAFECORE_OVERFLOW_POLICY_HIGH    SC_QUEUE_DROP_OLDEST
#define SAFECORE_OVERFLOW_POLICY_MEDIUM  SC_QUEUE_DROP_OLDEST
#define SAFECORE_OVERFLOW_POLICY_LOW     SC_QUEUE_DROP_NEWEST

// === 防饿死：高优先级单次最多处理数 ===
#define SAFECORE_MAX_HIGH_EVENTS_PER_TICK  5

// === 超时监控 ===
#define SAFECORE_MAX_PROCESS_TIME_MS    10

// === 安全钩子 ===
#ifndef SAFECORE_ON_ERROR
#define SAFECORE_ON_ERROR(msg) do { \
    extern void safecore_error_handler(const char*); \
    safecore_error_handler(msg); \
} while(0)
#endif

// === 日志 ===
#if !defined(SAFECORE_LOG_ENABLED)
#define SAFECORE_LOG_ENABLED 0
#endif

#if SAFECORE_LOG_ENABLED
#include <stdio.h>
#define SC_LOG(fmt, ...) printf("[SafeCore] " fmt "\n", ##__VA_ARGS__)
#else
#define SC_LOG(...)
#endif

// 内部：队列大小数组
static const uint8_t g_queue_sizes[SAFECORE_EVENT_PRIORITIES] = {
    SAFECORE_QUEUE_SIZE_HIGH,
    SAFECORE_QUEUE_SIZE_MEDIUM,
    SAFECORE_QUEUE_SIZE_LOW
};

#endif // SAFECORE_CONFIG_H