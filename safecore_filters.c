/*
 * safecore_filters.c
 * 
 * SafeCore Event Filtering Implementation
 * This file implements the event filtering mechanism for SafeCore, allowing
 * events to be processed based on configurable rules.
 */
#include "safecore_filters.h"
#include "safecore_config.h"
#include <string.h>

#if SAFECORE_FILTERS_ENABLED == 1

/**
 * @brief Global filter rules array
 * 
 * Stores all configured filter rules for event processing
 */
static sc_filter_rule_t g_rules[SAFECORE_MAX_FILTER_RULES];

/**
 * @brief Current number of active filter rules
 */
static uint8_t g_rule_count = 0;

/**
 * @brief Initialize the event filtering system
 * 
 * This function initializes the filter system by clearing all rules
 * and resetting the rule counter to zero.
 */
void sc_filters_init(void) {
    /* Clear all filter rules */
    (void)memset(g_rules, 0, sizeof(g_rules));
    /* Reset rule counter */
    g_rule_count = 0U;
}

/**
 * @brief Add a new filter rule
 * 
 * This function adds a new filter rule to the system if there is space
 * available in the rules array.
 * 
 * @param rule Pointer to the rule structure to add
 * @return 0 on success, -1 on failure (invalid rule or no space)
 */
int sc_filters_add_rule(const sc_filter_rule_t *rule) {
    int result = -1;
    
    if ((rule != NULL) && (g_rule_count < SAFECORE_MAX_FILTER_RULES)) {
        /* Copy the rule into the rules array */
        g_rules[g_rule_count] = *rule;
        g_rule_count++;
        result = 0;
    }
    
    return result;
}

/**
 * @brief Remove a filter rule by index
 * 
 * This function removes a filter rule at the specified index, shifting
 * subsequent rules to fill the gap.
 * 
 * @param index Index of the rule to remove
 * @return 0 on success, -1 on failure (invalid index)
 */
int sc_filters_remove_rule(uint8_t index) {
    int result = -1;
    
    if (index < g_rule_count) {
        uint8_t i;
        /* Shift rules after the removed one to fill the gap */
        for (i = index; i < (g_rule_count - 1U); i++) {
            g_rules[i] = g_rules[i + 1U];
        }
        /* Decrement rule count */
        g_rule_count--;
        result = 0;
    }
    
    return result;
}

/**
 * @brief Check if an event should be processed based on filter rules
 * 
 * This function evaluates an event against all active filter rules
 * to determine if it should be processed or filtered out.
 * The system operates in whitelist mode by default.
 * 
 * @param e Pointer to the event to check
 * @return 1 if the event should be processed, 0 if it should be filtered out
 */
int sc_filters_check_event(const sc_event_t *e) {
    int result = 0; /* Default to rejecting null events */
    
    if (e != NULL) {
        result = 1; /* Default to allowing events (whitelist mode) */
        uint8_t i;
        
        /* Check event against all active rules */
        for (i = 0U; i < g_rule_count; i++) {
            const sc_filter_rule_t *rule = &g_rules[i];
            
            if (rule->enabled != 0U) {
                switch (rule->type) {
                    case SC_FILTER_TYPE_ALLOW:
                        if (e->id == rule->event_id) {
                            result = 1; /* Allow the event */
                        }
                        break;
                    case SC_FILTER_TYPE_DENY:
                        if (e->id == rule->event_id) {
                            result = 0; /* Deny the event */
                        }
                        break;
                    case SC_FILTER_TYPE_SIZE_MIN:
                        if (e->size < rule->param) {
                            result = 0; /* Size too small */
                        }
                        break;
                    case SC_FILTER_TYPE_SIZE_MAX:
                        if (e->size > rule->param) {
                            result = 0; /* Size too large */
                        }
                        break;
                    case SC_FILTER_TYPE_PRIORITY:
                        if (e->priority > rule->param) {
                            result = 0; /* Priority too low */
                        }
                        break;
                    default:
                        /* Unknown rule type, maintain current behavior */
                        break;
                }
            }
        }
    }
    
    return result;
}

/**
 * @brief Load filter rules from a buffer
 * 
 * This function loads multiple filter rules from a memory buffer,
 * validates them, and initializes the filter system with these rules.
 * 
 * @param buffer Pointer to buffer containing rule data
 * @param size Size of the buffer in bytes
 * @return 0 on success, -1 on failure
 */
int sc_filters_load_rules_from_buffer(const uint8_t *buffer, size_t size) {
    int result = -1;
    
    /* Validate buffer and size */
    if ((buffer != NULL) && (size > 0U) && ((size % sizeof(sc_filter_rule_t)) == 0U)) {
        size_t rule_count = size / sizeof(sc_filter_rule_t);
        
        if (rule_count <= SAFECORE_MAX_FILTER_RULES) {
            /* Initialize system and copy rules */
            sc_filters_init();
            (void)memcpy(g_rules, buffer, size);
            g_rule_count = (uint8_t)rule_count;

            /* Validate all loaded rules */
            uint8_t i;
            for (i = 0U; i < g_rule_count; i++) {
                /* Disable rules with invalid types */
            if (g_rules[i].type >= (SC_FILTER_TYPE_PRIORITY + 1U)) {
                    g_rules[i].enabled = 0U;
                }
            }
            result = 0;
        }
    }
    
    return result;
}

#endif /* SAFECORE_FILTERS_ENABLED */