// safecore_eventbus.c
#include "safecore_eventbus.h"
#include "safecore_filters.h"
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
static uint8_t g_event_queues[SAFECORE_EVENT_PRIORITIES][SAFECORE_EVENT_QUEUE_SIZE][SAFECORE_MAX_EVENT_SIZE];
static uint8_t g_event_sizes[SAFECORE_EVENT_PRIORITIES][SAFECORE_EVENT_QUEUE_SIZE];
static volatile uint8_t g_heads[SAFECORE_EVENT_PRIORITIES];
static volatile uint8_t g_tails[SAFECORE_EVENT_PRIORITIES];
static uint32_t g_dropped_events[SAFECORE_EVENT_PRIORITIES] = {0};

// 队列辅助函数
static inline uint8_t queue_full(uint8_t priority) {
    return (uint8_t)(g_heads[priority] + 1) % SAFECORE_EVENT_QUEUE_SIZE == g_tails[priority];
}

static inline uint8_t queue_empty(uint8_t priority) {
    return g_heads[priority] == g_tails[priority];
}

static inline int queue_push(uint8_t priority, const uint8_t *data, size_t size) {
    if (!data || size == 0 || size > SAFECORE_MAX_EVENT_SIZE) return -1;
    if (priority >= SAFECORE_EVENT_PRIORITIES) return -1;

    if (queue_full(priority)) {
#if SAFECORE_QUEUE_OVERFLOW_POLICY == SC_QUEUE_DROP_OLDEST
        g_tails[priority] = (g_tails[priority] + 1) % SAFECORE_EVENT_QUEUE_SIZE;
        g_dropped_events[priority]++;
#elif SAFECORE_QUEUE_OVERFLOW_POLICY == SC_QUEUE_PANIC
        SAFECORE_ON_ERROR("Event queue overflow - PANIC");
        while (1);
#else
        // SC_QUEUE_DROP_NEWEST
        g_dropped_events[priority]++;
        return -1;
#endif
    }

    memcpy(g_event_queues[priority][g_heads[priority]], data, size);
    g_event_sizes[priority][g_heads[priority]] = (uint8_t)size;
    g_heads[priority] = (g_heads[priority] + 1) % SAFECORE_EVENT_QUEUE_SIZE;
    return 0;
}

static inline const uint8_t* queue_pop(uint8_t priority, size_t *out_size) {
    if (queue_empty(priority)) return NULL;
    *out_size = g_event_sizes[priority][g_tails[priority]];
    const uint8_t *e = g_event_queues[priority][g_tails[priority]];
    g_tails[priority] = (g_tails[priority] + 1) % SAFECORE_EVENT_QUEUE_SIZE;
    return e;
}

// === API 实现 ===
void sc_eventbus_init(void) {
    memset(g_subscribers, 0, sizeof(g_subscribers));
    g_subscriber_count = 0;
    
    for (int i = 0; i < SAFECORE_EVENT_PRIORITIES; i++) {
        g_heads[i] = g_tails[i] = 0;
        g_dropped_events[i] = 0;
    }
    
    sc_filters_init();
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
    if (e->priority >= SAFECORE_EVENT_PRIORITIES) return -1;

    // 应用过滤器
    if (!sc_filters_check_event(e)) {
        SC_LOG("Event %d filtered out", e->id);
        return 0; // 过滤掉不算错误
    }

    return queue_push(e->priority, event_data, size);
}

void sc_eventbus_process(void) {
    uint32_t start = safecore_get_tick_ms();

    // 按优先级顺序处理：高 -> 低
    for (int prio = 0; prio < SAFECORE_EVENT_PRIORITIES; prio++) {
        size_t size;
        const uint8_t *raw;
        uint8_t processed = 0;

        // 每个优先级最多处理 N 个事件，防饿死低优先级
        while ((raw = queue_pop(prio, &size)) != NULL && processed < SAFECORE_MAX_EVENTS_PER_CYCLE) {
            const sc_event_t *e = (const sc_event_t*)raw;
            
            // 调用所有匹配的订阅者
            for (uint8_t i = 0; i < g_subscriber_count; i++) {
                if (g_subscribers[i].event_id == e->id) {
                    if (g_subscribers[i].callback) {
                        g_subscribers[i].callback(e, g_subscribers[i].ctx);
                    } else {
                        SAFECORE_ON_ERROR("Null subscriber callback!");
                    }
                }
            }
            processed++;
        }
    }

    // 超时检查
    uint32_t elapsed = safecore_get_tick_ms() - start;
    if (elapsed > SAFECORE_MAX_PROCESS_TIME_MS) {
        SAFECORE_ON_ERROR("Event processing timeout!");
    }
}

// 调试接口
uint8_t sc_eventbus_get_queue_depth(uint8_t priority) {
    if (priority >= SAFECORE_EVENT_PRIORITIES) return 0;
    int32_t depth = (int32_t)g_heads[priority] - (int32_t)g_tails[priority];
    if (depth < 0) depth += SAFECORE_EVENT_QUEUE_SIZE;
    return (uint8_t)depth;
}

void sc_eventbus_get_stats(uint8_t *depths, uint32_t *dropped) {
    if (depths) {
        for (int i = 0; i < SAFECORE_EVENT_PRIORITIES; i++) {
            depths[i] = sc_eventbus_get_queue_depth(i);
        }
    }
    if (dropped) {
        for (int i = 0; i < SAFECORE_EVENT_PRIORITIES; i++) {
            dropped[i] = g_dropped_events[i];
        }
    }
}