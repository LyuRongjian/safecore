// safecore_types.h
#ifndef SAFECORE_TYPES_H
#define SAFECORE_TYPES_H

#include <stdint.h>
#include "safecore_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 事件基类（必须是第一个成员）
typedef struct {
    uint8_t id;
    uint8_t priority;      // 0 = highest
    uint32_t timestamp;
} sc_event_t;

// 状态机事件（内部使用）
typedef enum {
    SC_SM_EVENT_INIT = 0,
    SC_SM_EVENT_ENTRY,
    SC_SM_EVENT_EXIT,
    SC_SM_EVENT_TICK,
    SC_SM_EVENT_USER_START = 16
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

typedef enum {
    SC_SM_HANDLED,
    SC_SM_TRANSITION,
    SC_SM_SUPER
} sc_sm_result_t;

typedef sc_sm_result_t (*sc_sm_handler_t)(void *ctx, const sc_sm_event_t *e, void **next_state);

#ifdef __cplusplus
}
#endif

#endif // SAFECORE_TYPES_H