// safecore_types.h
#ifndef SAFECORE_TYPES_H
#define SAFECORE_TYPES_H

#include <stdint.h>
#include "safecore_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 事件类型定义
typedef enum {
    SC_EVENT_INIT = 0,
    SC_EVENT_ENTRY,
    SC_EVENT_EXIT,
    SC_EVENT_TICK,
    SC_EVENT_USER_START = 16
} sc_sm_event_type_t;

typedef struct {
    sc_sm_event_type_t type;
    uint32_t timestamp;
    union {
        uint32_t u32;
        int32_t s32;
        void *ptr;
        uint8_t bytes[8];
    } data;
} sc_sm_event_t;

// 通用事件基类（必须在第一个成员）
typedef struct {
    uint8_t id;              // 事件ID
    uint8_t priority;        // 优先级 (0=紧急, 1=标准, 2=低)
    uint8_t size;            // 实际大小（用于过滤）
    uint32_t timestamp;      // 时间戳
} sc_event_t;

// 状态机结果
typedef enum {
    SC_SM_HANDLED,
    SC_SM_TRANSITION,
    SC_SM_SUPER
} sc_sm_result_t;

typedef sc_sm_result_t (*sc_sm_handler_t)(void *ctx, const sc_sm_event_t *e, void **next_state);

// 订阅者回调
typedef void (*sc_subscriber_fn_t)(const sc_event_t *e, void *ctx);

#ifdef __cplusplus
}
#endif

#endif // SAFECORE_TYPES_H