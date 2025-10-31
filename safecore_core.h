#ifndef SAFECORE_CORE_H
#define SAFECORE_CORE_H

#include "safecore_types.h"
#include "safecore_config.h"

/* === 状态机接口 === */
typedef struct {
    sc_sm_handler_t handlers[SAFECORE_MAX_HSM_DEPTH];
    void *contexts[SAFECORE_MAX_HSM_DEPTH];
    uint8_t depth;
    void *user_ctx;
    const char *name;
} sc_state_machine_t;

void sc_sm_init(sc_state_machine_t *sm, sc_sm_handler_t top, void *ctx, const char *name);
void sc_sm_dispatch(sc_state_machine_t *sm, const sc_sm_event_t *e);
void sc_sm_send_entry(sc_state_machine_t *sm, void *ctx);
void sc_sm_send_exit(sc_state_machine_t *sm, void *ctx);

/* === 基础事件总线接口 === */
#if SAFECORE_BASIC_ENABLED == 1
void sc_eventbus_init(void);
int sc_eventbus_subscribe(uint8_t event_id, sc_subscriber_fn_t callback, void *ctx);
int sc_eventbus_publish_raw(const uint8_t *event_data, size_t size);
void sc_eventbus_process(void);

/* === 编译期检查的发布宏 === */
#define SC_PUBLISH_EVENT(evt_ptr) ({ \
    SC_STATIC_ASSERT(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event_too_large"); \
    sc_eventbus_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))); \
})
#endif

#endif /* SAFECORE_CORE_H */