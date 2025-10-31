// safecore_filters.c
#include "safecore_filters.h"
#include "safecore_config.h"
#include <string.h>

static sc_filter_rule_t g_rules[SAFECORE_MAX_FILTER_RULES];
static uint8_t g_rule_count = 0;

void sc_filters_init(void) {
    memset(g_rules, 0, sizeof(g_rules));
    g_rule_count = 0;
}

int sc_filters_add_rule(const sc_filter_rule_t *rule) {
    if (!rule || g_rule_count >= SAFECORE_MAX_FILTER_RULES) return -1;
    g_rules[g_rule_count++] = *rule;
    return 0;
}

int sc_filters_remove_rule(uint8_t index) {
    if (index >= g_rule_count) return -1;
    for (uint8_t i = index; i < g_rule_count - 1; i++) {
        g_rules[i] = g_rules[i + 1];
    }
    g_rule_count--;
    return 0;
}

int sc_filters_check_event(const sc_event_t *e) {
    if (!e) return 0; // 拒绝空事件

    for (uint8_t i = 0; i < g_rule_count; i++) {
        const sc_filter_rule_t *rule = &g_rules[i];
        if (!rule->enabled) continue;

        switch (rule->type) {
            case SC_FILTER_TYPE_ALLOW:
                if (e->id == rule->event_id) return 1; // 允许
                break;
            case SC_FILTER_TYPE_DENY:
                if (e->id == rule->event_id) return 0; // 拒绝
                break;
            case SC_FILTER_TYPE_SIZE_MIN:
                if (e->size < rule->param) return 0; // 太小
                break;
            case SC_FILTER_TYPE_SIZE_MAX:
                if (e->size > rule->param) return 0; // 太大
                break;
            case SC_FILTER_TYPE_PRIORITY:
                if (e->priority > rule->param) return 0; // 优先级太低
                break;
        }
    }
    return 1; // 默认允许（白名单模式）
}

int sc_filters_load_rules_from_buffer(const uint8_t *buffer, size_t size) {
    if (!buffer || size == 0 || size % sizeof(sc_filter_rule_t) != 0) {
        return -1;
    }
    size_t rule_count = size / sizeof(sc_filter_rule_t);
    if (rule_count > SAFECORE_MAX_FILTER_RULES) return -1;

    sc_filters_init();
    memcpy(g_rules, buffer, size);
    g_rule_count = (uint8_t)rule_count;

    // 验证规则有效性
    for (size_t i = 0; i < g_rule_count; i++) {
        if (g_rules[i].type >= SC_FILTER_TYPE_PRIORITY + 1) {
            g_rules[i].enabled = 0;
        }
    }
    return 0;
}