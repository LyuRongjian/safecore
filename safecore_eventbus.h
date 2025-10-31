// safecore_eventbus.h
#ifndef SAFECORE_EVENTBUS_H
#define SAFECORE_EVENTBUS_H

#include "safecore_types.h"
#include "safecore_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 订阅接口
int sc_eventbus_subscribe(uint8_t event_id, sc_subscriber_fn_t callback, void *ctx);

// 发布宏（带编译期检查和优先级）
#define SC_PUBLISH_EMERGENCY(evt_ptr) ({ \
    _Static_assert(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event too large!"); \
    ((sc_event_t*)(evt_ptr))->priority = SAFECORE_EMERGENCY_PRIORITY; \
    sc_eventbus_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))); \
})

#define SC_PUBLISH_STANDARD(evt_ptr) ({ \
    _Static_assert(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event too large!"); \
    ((sc_event_t*)(evt_ptr))->priority = SAFECORE_STANDARD_PRIORITY; \
    sc_eventbus_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))); \
})

#define SC_PUBLISH_LOW(evt_ptr) ({ \
    _Static_assert(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event too large!"); \
    ((sc_event_t*)(evt_ptr))->priority = SAFECORE_LOW_PRIORITY; \
    sc_eventbus_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))); \
})

// 核心接口
void sc_eventbus_init(void);
int sc_eventbus_publish_raw(const uint8_t *event_data, size_t size);
void sc_eventbus_process(void);

// 调试接口
uint8_t sc_eventbus_get_queue_depth(uint8_t priority);
void sc_eventbus_get_stats(uint8_t *depths, uint32_t *dropped);

#ifdef __cplusplus
}
#endif

#endif // SAFECORE_EVENTBUS_H