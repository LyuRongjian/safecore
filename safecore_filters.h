/*
 * safecore_filters.h
 * 
 * SafeCore Event Filtering Module
 * This header file defines the filtering mechanisms for the SafeCore framework,
 * allowing for selective processing of events based on configurable rules.
 */

#ifndef SAFECORE_FILTERS_H
#define SAFECORE_FILTERS_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_FILTERS_ENABLED == 1

/**
 * @defgroup SafeCore_FILTERS SafeCore Filtering Module
 * @brief Event filtering system for SafeCore framework
 * @{
 */

/* === Filter Interface === */

/**
 * @brief Initialize the filtering system
 * 
 * Initializes the SafeCore event filtering system, setting up data structures
 * and resetting statistics.
 */
void sc_filters_init(void);

/**
 * @brief Add a filter rule
 * 
 * Adds a single filter rule to the SafeCore filtering system.
 * 
 * @param rule Pointer to the filter rule to add
 * @return int Status code indicating success or failure
 */
int sc_filters_add_rule(const sc_filter_rule_t *rule);

/**
 * @brief Remove a filter rule
 * 
 * Removes a filter rule from the SafeCore filtering system based on index.
 * 
 * @param index Index of the rule to remove
 * @return int Status code indicating success or failure
 */
int sc_filters_remove_rule(uint8_t index);

/**
 * @brief Check if an event passes all filters
 * 
 * Evaluates an event against all active filter rules to determine if it
 * should be processed or filtered out.
 * 
 * @param e Pointer to the event to check
 * @return int 1 if event passes filters, 0 otherwise
 */
int sc_filters_check_event(const sc_event_t *e);

/**
 * @brief Load filter rules from buffer
 * 
 * Loads a set of filter rules from a memory buffer into the filtering system.
 * 
 * @param buffer Pointer to buffer containing filter rules
 * @param size Size of buffer in bytes
 * @return int Status code indicating success or failure
 */
int sc_filters_load_rules_from_buffer(const uint8_t *buffer, size_t size);

/**
 * @}
 */

#endif /* SAFECORE_FILTERS_ENABLED */
#endif /* SAFECORE_FILTERS_H */