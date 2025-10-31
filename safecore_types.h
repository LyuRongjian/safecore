#ifndef SAFECORE_TYPES_H
#define SAFECORE_TYPES_H

#include "safecore_config.h"
#include "safecore_module_config.h"
#include <stdint.h>

/* === 基础事件类型（v2.0）=== */
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

/* === 通用事件基类 === */
typedef struct {
    uint8_t id;              /* 事件ID */
#if SAFECORE_PRIORITY_ENABLED == 1
    uint8_t priority;        /* 优先级 */
#else
    uint8_t reserved1;       /* 保留 */
#endif
    uint8_t size;            /* 实际大小 */
    uint8_t reserved2;       /* 对齐填充 */
    uint32_t timestamp;      /* 时间戳 */
#if SAFECORE_PRIORITY_ENABLED == 1
    uint32_t timestampMicro; /* 微秒部分 */
#endif
} sc_event_t;

/* === 状态机相关类型 === */
typedef enum {
    SC_SM_HANDLED,
    SC_SM_TRANSITION,
    SC_SM_SUPER
} sc_sm_result_t;

typedef sc_sm_result_t (*sc_sm_handler_t)(void *ctx, const sc_sm_event_t *e, void **next_state);

/* === 订阅者回调类型 === */
typedef void (*sc_subscriber_fn_t)(const sc_event_t *e, void *ctx);

/* === 优先级相关类型（v3.0）=== */
#if SAFECORE_PRIORITY_ENABLED == 1
typedef enum {
    SC_PRIORITY_EMERGENCY = 0,
    SC_PRIORITY_STANDARD = 1,
    SC_PRIORITY_LOW = 2
} sc_priority_type_t;
#endif

/* === 过滤器类型（v3.0）=== */
#if SAFECORE_FILTERS_ENABLED == 1
typedef enum {
    SC_FILTER_TYPE_ALLOW,
    SC_FILTER_TYPE_DENY,
    SC_FILTER_TYPE_SIZE_MIN,
    SC_FILTER_TYPE_SIZE_MAX,
    SC_FILTER_TYPE_PRIORITY
} sc_filter_type_t;

typedef struct {
    uint8_t enabled;
    sc_filter_type_t type;
    uint8_t event_id;
    uint8_t param;
} sc_filter_rule_t;
#endif

/* === 诊断相关类型（v4.0）=== */
#if SAFECORE_DIAGNOSTICS_ENABLED == 1
typedef enum {
    SC_DTC_INFO = 0,
    SC_DTC_WARNING = 1,
    SC_DTC_ERROR = 2,
    SC_DTC_FATAL = 3
} sc_dtc_severity_type_t;

typedef struct {
    uint16_t dtc_id;
    uint8_t severity;
    uint8_t status;
    uint32_t timestamp;
} sc_dtc_type_t;
#endif

/* === 通信相关类型（v4.0）=== */
#if SAFECORE_COMM_ENABLED == 1
typedef struct {
    uint32_t can_id;
    uint8_t dlc;
    uint8_t data[8];
#if SAFECORE_PRIORITY_ENABLED == 1
    uint8_t priority;
#endif
} sc_can_frame_type_t;
#endif

#endif /* SAFECORE_TYPES_H */