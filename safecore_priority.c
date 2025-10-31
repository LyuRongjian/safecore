#include "safecore_priority.h"
#include "safecore_port.h"
#include "safecore_config.h"
#include "safecore_module_config.h"
#include <string.h>

#if SAFECORE_PRIORITY_ENABLED == 1

/* 多优先级队列 */
static uint8_t g_event_queues[SAFECORE_EVENT_PRIORITIES][SAFECORE_EVENT_QUEUE_SIZE][SAFECORE_MAX_EVENT_SIZE];
static uint8_t g_event_sizes[SAFECORE_EVENT_PRIORITIES][SAFECORE_EVENT_QUEUE_SIZE];
static volatile uint8_t g_heads[SAFECORE_EVENT_PRIORITIES];
static volatile uint8_t g_tails[SAFECORE_EVENT_PRIORITIES];
static uint32_t g_dropped_events[SAFECORE_EVENT_PRIORITIES] = {0};

/* 订阅表（从核心模块获取） */
extern subscriber_entry_t g_subscribers[SAFECORE_MAX_SUBSCRIBERS];
extern uint8_t g_subscriber_count;

/* 队列辅助函数 */
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
#if SAFECORE_QUEUE_OVERFLOW_POLICY == SAFECORE_QUEUE_DROP_OLDEST
        g_tails[priority] = (g_tails[priority] + 1) % SAFECORE_EVENT_QUEUE_SIZE;
        g_dropped_events[priority]++;
#elif SAFECORE_QUEUE_OVERFLOW_POLICY == SAFECORE_QUEUE_PANIC
        SAFECORE_ON_ERROR("Event queue overflow - PANIC");
        while (1);
#else
        g_dropped_events[priority]++;
        return -1;
#endif
    }

    (void)memcpy(g_event_queues[priority][g_heads[priority]], data, size);
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

void sc_priority_init(void) {
    for (int i = 0; i < SAFECORE_EVENT_PRIORITIES; i++) {
        g_heads[i] = g_tails[i] = 0;
        g_dropped_events[i] = 0;
    }
}

int sc_priority_publish_raw(const uint8_t *event_data, size_t size) {
    if (!event_data || size == 0) return -1;
    const sc_event_t *e = (const sc_event_t*)event_data;
    if (e->id >= SAFECORE_MAX_EVENT_TYPES) return -1;
    
    uint8_t priority = e->priority;
    if (priority >= SAFECORE_EVENT_PRIORITIES) priority = SAFECORE_LOW_PRIORITY;

#if SAFECORE_FILTERS_ENABLED == 1
    if (!sc_filters_check_event(e)) {
        SC_LOG("Event %d filtered out", e->id);
        return 0;
    }
#endif

    return queue_push(priority, event_data, size);
}

void sc_priority_process(void) {
    for (int prio = 0; prio < SAFECORE_EVENT_PRIORITIES; prio++) {
        size_t size;
        const uint8_t *raw;
        uint8_t processed = 0;

        while ((raw = queue_pop(prio, &size)) != NULL && 
               processed < SAFECORE_MAX_EVENTS_PER_CYCLE) {
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
            processed++;
        }
    }
}

/* 调试接口 */
uint8_t sc_priority_get_queue_depth(uint8_t priority) {
    if (priority >= SAFECORE_EVENT_PRIORITIES) return 0;
    int32_t depth = (int32_t)g_heads[priority] - (int32_t)g_tails[priority];
    if (depth < 0) depth += SAFECORE_EVENT_QUEUE_SIZE;
    return (uint8_t)depth;
}

void sc_priority_get_stats(uint8_t *depths, uint32_t *dropped) {
    if (depths) {
        for (int i = 0; i < SAFECORE_EVENT_PRIORITIES; i++) {
            depths[i] = sc_priority_get_queue_depth(i);
        }
    }
    if (dropped) {
        for (int i = 0; i < SAFECORE_EVENT_PRIORITIES; i++) {
            dropped[i] = g_dropped_events[i];
        }
    }
}

#endif /* SAFECORE_PRIORITY_ENABLED */