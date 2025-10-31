// safecore_eventbus.c
#include "safecore_eventbus.h"
#include "safecore_port.h"
#include "safecore_config.h"
#include <string.h>

// 订阅表
typedef struct {
    uint8_t event_id;
    sc_subscriber_fn_t callback;
    void *ctx;
} subscriber_entry_t;

static subscriber_entry_t g_subscribers[SAFECORE_MAX_SUBSCRIBERS];
static uint8_t g_subscriber_count = 0;

// 多优先级队列
static uint8_t g_event_queues[SAFECORE_EVENT_PRIORITIES]
                            [SAFECORE_QUEUE_SIZE_LOW]  // 使用最大可能大小
                            [SAFECORE_MAX_EVENT_SIZE];
static uint8_t g_event_sizes[SAFECORE_EVENT_PRIORITIES][SAFECORE_QUEUE_SIZE_LOW];
static volatile uint8_t g_heads[SAFECORE_EVENT_PRIORITIES] = {0};
static volatile uint8_t g_tails[SAFECORE_EVENT_PRIORITIES] = {0};

static inline uint8_t queue_full(uint8_t prio) {
    uint8_t size = g_queue_sizes[prio];
    return (g_heads[prio] + 1U) % size == g_tails[prio];
}

static inline uint8_t queue_empty(uint8_t prio) {
    return g_heads[prio] == g_tails[prio];
}

static inline int queue_push(uint8_t prio, const uint8_t *data, size_t size) {
    if (!data || size == 0 || size > SAFECORE_MAX_EVENT_SIZE) {
        return -1;
    }
    uint8_t q_size = g_queue_sizes[prio];
    if (prio >= SAFECORE_EVENT_PRIORITIES || size > SAFECORE_MAX_EVENT_SIZE) {
        return -1;
    }

    if (queue_full(prio)) {
        // 按优先级应用不同策略
        int policy = SC_QUEUE_DROP_NEWEST;
        if (prio == SAFECORE_PRIORITY_HIGH) policy = SAFECORE_OVERFLOW_POLICY_HIGH;
        else if (prio == SAFECORE_PRIORITY_MEDIUM) policy = SAFECORE_OVERFLOW_POLICY_MEDIUM;
        else policy = SAFECORE_OVERFLOW_POLICY_LOW;

        if (policy == SC_QUEUE_DROP_OLDEST) {
            g_tails[prio] = (g_tails[prio] + 1U) % q_size;
        } else if (policy == SC_QUEUE_PANIC) {
            SAFECORE_ON_ERROR("Queue overflow - PANIC");
            while (1);
        } else {
            return -1; // DROP_NEWEST
        }
    }

    uint8_t idx = g_heads[prio];
    (void)memcpy(g_event_queues[prio][idx], data, size);
    g_event_sizes[prio][idx] = (uint8_t)size;
    g_heads[prio] = (g_heads[prio] + 1U) % q_size;
    return 0;
}

static inline const uint8_t* queue_pop(uint8_t prio, size_t *out_size) {
    if (queue_empty(prio)) return NULL;
    uint8_t idx = g_tails[prio];
    *out_size = g_event_sizes[prio][idx];
    const uint8_t *e = g_event_queues[prio][idx];
    g_tails[prio] = (g_tails[prio] + 1U) % g_queue_sizes[prio];
    return e;
}

// === API ===
void sc_eventbus_init(void) {
    (void)memset(g_subscribers, 0, sizeof(g_subscribers));
    g_subscriber_count = 0;
    for (int i = 0; i < SAFECORE_EVENT_PRIORITIES; i++) {
        g_heads[i] = g_tails[i] = 0;
    }
}

int sc_eventbus_subscribe(uint8_t event_id, sc_subscriber_fn_t callback, void *ctx) {
    if (event_id >= SAFECORE_MAX_EVENT_TYPES || !callback || 
        g_subscriber_count >= SAFECORE_MAX_SUBSCRIBERS) {
        return -1;
    }
    g_subscribers[g_subscriber_count].event_id = event_id;
    g_subscribers[g_subscriber_count].callback = callback;
    g_subscribers[g_subscriber_count].ctx = ctx;
    g_subscriber_count++;
    return 0;
}

int sc_eventbus_publish_raw(const uint8_t *event_data, size_t size) {
    if (!event_data || size == 0) return -1;
    const sc_event_t *e = (const sc_event_t*)event_data;
    if (e->id >= SAFECORE_MAX_EVENT_TYPES || e->priority >= SAFECORE_EVENT_PRIORITIES) {
        return -1;
    }
    return queue_push(e->priority, event_data, size);
}

void sc_eventbus_process(void) {
    uint32_t start = safecore_get_tick_ms();

    // 处理高优先级（带防饿死）
    uint8_t high_processed = 0;
    size_t size;
    const uint8_t *raw;
    while (high_processed < SAFECORE_MAX_HIGH_EVENTS_PER_TICK &&
           (raw = queue_pop(SAFECORE_PRIORITY_HIGH, &size)) != NULL) {
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
        high_processed++;
    }

    // 处理中优先级
    while ((raw = queue_pop(SAFECORE_PRIORITY_MEDIUM, &size)) != NULL) {
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

    // 处理低优先级
    while ((raw = queue_pop(SAFECORE_PRIORITY_LOW, &size)) != NULL) {
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

    // 超时监控
    uint32_t elapsed = safecore_get_tick_ms() - start;
    if (elapsed > SAFECORE_MAX_PROCESS_TIME_MS) {
        SAFECORE_ON_ERROR("Event processing timeout!");
    }
}