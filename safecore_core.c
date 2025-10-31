#include "safecore_core.h"
#include "safecore_port.h"
#include "safecore_config.h"
#include "safecore_module_config.h"
#include <string.h>

/* === 状态机实现 === */
void sc_sm_init(sc_state_machine_t *sm, sc_sm_handler_t top, void *ctx, const char *name) {
    if (!sm || !top) {
        SAFECORE_ON_ERROR("SM init: null pointer");
        return;
    }
    (void)memset(sm, 0, sizeof(*sm));
    sm->handlers[0] = top;
    sm->user_ctx = ctx;
    sm->name = name ? name : "sm";
}

void sc_sm_dispatch(sc_state_machine_t *sm, const sc_sm_event_t *e) {
    if (!sm || !e) {
        SAFECORE_ON_ERROR("SM dispatch: null pointer");
        return;
    }
    if (sm->depth >= SAFECORE_MAX_HSM_DEPTH) {
        SAFECORE_ON_ERROR("SM: max depth exceeded");
        return;
    }

    sc_sm_event_t ev = *e;
    ev.timestamp = safecore_get_tick_ms();

    for (int i = sm->depth; i >= 0; i--) {
        if (!sm->handlers[i]) {
            SAFECORE_ON_ERROR("SM: null handler");
            return;
        }
        void *next = NULL;
        sc_sm_result_t r = sm->handlers[i](sm->contexts[i], &ev, &next);
        if (r == SC_SM_HANDLED) return;
        if (r == SC_SM_TRANSITION) {
            if (!next) {
                SAFECORE_ON_ERROR("SM: transition to null state");
                return;
            }
            /* Exit current path */
            for (int j = sm->depth; j >= 0; j--) {
                if (sm->handlers[j]) sc_sm_send_exit(sm, sm->contexts[j]);
            }
            sm->depth = 0;
            sm->handlers[0] = (sc_sm_handler_t)next;
            sm->contexts[0] = sm->user_ctx;
            sc_sm_send_entry(sm, sm->contexts[0]);
            return;
        }
    }
}

void sc_sm_send_entry(sc_state_machine_t *sm, void *ctx) {
#if SAFECORE_ENTRY_EXIT_ENABLED == 1
    if (!sm || !ctx) return;
    sc_sm_event_t e = { .type = SC_EVENT_ENTRY, .timestamp = safecore_get_tick_ms() };
    if (sm->handlers[sm->depth]) {
        sm->handlers[sm->depth](ctx, &e, NULL);
    }
#endif
}

void sc_sm_send_exit(sc_state_machine_t *sm, void *ctx) {
#if SAFECORE_ENTRY_EXIT_ENABLED == 1
    if (!sm || !ctx) return;
    sc_sm_event_t e = { .type = SC_EVENT_EXIT, .timestamp = safecore_get_tick_ms() };
    if (sm->handlers[sm->depth]) {
        sm->handlers[sm->depth](ctx, &e, NULL);
    }
#endif
}

/* === 基础事件总线实现 === */
#if SAFECORE_BASIC_ENABLED == 1

/* 订阅表 */
typedef struct {
    uint8_t event_id;
    sc_subscriber_fn_t callback;
    void *ctx;
} subscriber_entry_t;

static subscriber_entry_t g_subscribers[SAFECORE_MAX_SUBSCRIBERS];
static uint8_t g_subscriber_count = 0;

/* 单队列（当优先级禁用时） */
#if SAFECORE_PRIORITY_ENABLED != 1
static uint8_t g_event_queue[SAFECORE_EVENT_QUEUE_SIZE][SAFECORE_MAX_EVENT_SIZE];
static volatile uint8_t g_head = 0;
static volatile uint8_t g_tail = 0;

static inline uint8_t queue_full(void) {
    return (uint8_t)(g_head + 1) % SAFECORE_EVENT_QUEUE_SIZE == g_tail;
}

static inline uint8_t queue_empty(void) {
    return g_head == g_tail;
}

static inline int queue_push(const uint8_t *data, size_t size) {
    if (!data || size == 0 || size > SAFECORE_MAX_EVENT_SIZE) return -1;
    
    if (queue_full()) {
#if SAFECORE_QUEUE_OVERFLOW_POLICY == SAFECORE_QUEUE_DROP_OLDEST
        g_tail = (g_tail + 1) % SAFECORE_EVENT_QUEUE_SIZE;
#elif SAFECORE_QUEUE_OVERFLOW_POLICY == SAFECORE_QUEUE_PANIC
        SAFECORE_ON_ERROR("Event queue overflow - PANIC");
        while (1);
#else
        return -1; /* SAFECORE_QUEUE_DROP_NEWEST */
#endif
    }
    
    (void)memcpy(g_event_queue[g_head], data, size);
    g_head = (g_head + 1) % SAFECORE_EVENT_QUEUE_SIZE;
    return 0;
}

static inline const uint8_t* queue_pop(void) {
    if (queue_empty()) return NULL;
    const uint8_t *e = g_event_queue[g_tail];
    g_tail = (g_tail + 1) % SAFECORE_EVENT_QUEUE_SIZE;
    return e;
}
#endif /* SAFECORE_PRIORITY_ENABLED != 1 */

void sc_eventbus_init(void) {
    (void)memset(g_subscribers, 0, sizeof(g_subscribers));
    g_subscriber_count = 0;
    
#if SAFECORE_PRIORITY_ENABLED != 1
    g_head = g_tail = 0;
#endif

#if SAFECORE_FILTERS_ENABLED == 1
    sc_filters_init();
#endif
}

int sc_eventbus_subscribe(uint8_t event_id, sc_subscriber_fn_t callback, void *ctx) {
    if (event_id >= SAFECORE_MAX_EVENT_TYPES || !callback || 
        g_subscriber_count >= SAFECORE_MAX_SUBSCRIBERS) {
        return -1;
    }
    g_subscribers[g_subscriber_count++] = (subscriber_entry_t){event_id, callback, ctx};
    return 0;
}

int sc_eventbus_publish_raw(const uint8_t *event_data, size_t size) {
    if (!event_data || size == 0) return -1;
    const sc_event_t *e = (const sc_event_t*)event_data;
    if (e->id >= SAFECORE_MAX_EVENT_TYPES) return -1;

#if SAFECORE_FILTERS_ENABLED == 1
    if (!sc_filters_check_event(e)) {
        SC_LOG("Event %d filtered out", e->id);
        return 0;
    }
#endif

#if SAFECORE_PRIORITY_ENABLED == 1
    return sc_priority_publish_raw(event_data, size);
#else
    return queue_push(event_data, size);
#endif
}

void sc_eventbus_process(void) {
    uint32_t start = safecore_get_tick_ms();

#if SAFECORE_PRIORITY_ENABLED == 1
    sc_priority_process();
#else
    const uint8_t *raw;
    while ((raw = queue_pop()) != NULL) {
        const sc_event_t *e = (const sc_event_t*)raw;
        for (uint8_t i = 0; i < g_subscriber_count; i++) {
            if (g_subscribers[i].event_id == e->id) {
                if (g_subscribers[i].callback) {
                    g_subscribers[i].callback(e, g_subscribers[i].ctx);
                } else {
                    SAFECORE_ON_ERROR("Null subscriber callback!");
                }
            }
        }
    }
#endif

    /* 超时检查 */
    uint32_t elapsed = safecore_get_tick_ms() - start;
    if (elapsed > SAFECORE_MAX_PROCESS_TIME_MS) {
        SAFECORE_ON_ERROR("Event processing timeout!");
    }
}

#endif /* SAFECORE_BASIC_ENABLED */