// safecore_filters.h
#ifndef SAFECORE_FILTERS_H
#define SAFECORE_FILTERS_H

#include "safecore_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// 过滤器类型
typedef enum {
    SC_FILTER_TYPE_ALLOW,      // 允许指定事件
    SC_FILTER_TYPE_DENY,       // 拒绝指定事件
    SC_FILTER_TYPE_SIZE_MIN,   // 最小大小限制
    SC_FILTER_TYPE_SIZE_MAX,   // 最大大小限制
    SC_FILTER_TYPE_PRIORITY,   // 优先级限制
} sc_filter_type_t;

// 过滤规则
typedef struct {
    uint8_t enabled;           // 规则是否启用
    sc_filter_type_t type;     // 规则类型
    uint8_t event_id;          // 事件ID（或优先级值）
    uint8_t param;             // 参数（如最小优先级、最小大小等）
} sc_filter_rule_t;

// 过滤器接口
void sc_filters_init(void);
int sc_filters_add_rule(const sc_filter_rule_t *rule);
int sc_filters_remove_rule(uint8_t index);
int sc_filters_check_event(const sc_event_t *e);

// 动态规则加载接口
int sc_filters_load_rules_from_buffer(const uint8_t *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif // SAFECORE_FILTERS_H