// safecore_eventbus.h
#ifndef SAFECORE_EVENTBUS_H
#define SAFECORE_EVENTBUS_H

#include "safecore_types.h"
#include "safecore_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sc_subscriber_fn_t)(const sc_event_t *e, void *ctx);

void sc_eventbus_init(void);
int sc_eventbus_subscribe(uint8_t event_id, sc_subscriber_fn_t callback, void *ctx);

// 安全发布宏（自动设置优先级 + 编译期大小检查）
#define SC_PUBLISH_EVENT(evt_ptr, prio) ({ \
    _Static_assert(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event too large!"); \
    _Static_assert((prio) < SAFECORE_EVENT_PRIORITIES, "Invalid priority!"); \
    sc_event_t *_e = (sc_event_t*)(evt_ptr); \
    _e->priority = (prio); \
    sc_eventbus_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))); \
})

#define SC_PUBLISH_HIGH(evt)   SC_PUBLISH_EVENT(evt, SAFECORE_PRIORITY_HIGH)
#define SC_PUBLISH_MEDIUM(evt) SC_PUBLISH_EVENT(evt, SAFECORE_PRIORITY_MEDIUM)
#define SC_PUBLISH_LOW(evt)    SC_PUBLISH_EVENT(evt, SAFECORE_PRIORITY_LOW)

int sc_eventbus_publish_raw(const uint8_t *event_data, size_t size);
void sc_eventbus_process(void);

#ifdef __cplusplus
}
#endif
#endif