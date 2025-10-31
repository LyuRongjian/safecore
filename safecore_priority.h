#ifndef SAFECORE_PRIORITY_H
#define SAFECORE_PRIORITY_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_PRIORITY_ENABLED == 1

/* === 优先级队列接口 === */
void sc_priority_init(void);
int sc_priority_publish_raw(const uint8_t *event_data, size_t size);
void sc_priority_process(void);

/* === 优先级发布宏 === */
#define SC_PUBLISH_EMERGENCY(evt_ptr) ({ \
    SC_STATIC_ASSERT(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event_too_large"); \
    ((sc_event_t*)(evt_ptr))->priority = SC_PRIORITY_EMERGENCY; \
    sc_priority_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))); \
})

#define SC_PUBLISH_STANDARD(evt_ptr) ({ \
    SC_STATIC_ASSERT(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event_too_large"); \
    ((sc_event_t*)(evt_ptr))->priority = SC_PRIORITY_STANDARD; \
    sc_priority_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))); \
})

#define SC_PUBLISH_LOW(evt_ptr) ({ \
    SC_STATIC_ASSERT(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event_too_large"); \
    ((sc_event_t*)(evt_ptr))->priority = SC_PRIORITY_LOW; \
    sc_priority_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))); \
})

/* === 调试接口 === */
uint8_t sc_priority_get_queue_depth(uint8_t priority);
void sc_priority_get_stats(uint8_t *depths, uint32_t *dropped);

#endif /* SAFECORE_PRIORITY_ENABLED */
#endif /* SAFECORE_PRIORITY_H */