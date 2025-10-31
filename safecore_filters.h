#ifndef SAFECORE_FILTERS_H
#define SAFECORE_FILTERS_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_FILTERS_ENABLED == 1

/* === 过滤器接口 === */
void sc_filters_init(void);
int sc_filters_add_rule(const sc_filter_rule_t *rule);
int sc_filters_remove_rule(uint8_t index);
int sc_filters_check_event(const sc_event_t *e);
int sc_filters_load_rules_from_buffer(const uint8_t *buffer, size_t size);

#endif /* SAFECORE_FILTERS_ENABLED */
#endif /* SAFECORE_FILTERS_H */