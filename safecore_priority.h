/*
 * safecore_priority.h
 * 
 * SafeCore Priority Queue Interface
 * This module provides priority-based event queue functionality for the SafeCore framework,
 * allowing events to be processed based on their priority levels.
 */
#ifndef SAFECORE_PRIORITY_H
#define SAFECORE_PRIORITY_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_PRIORITY_ENABLED == 1

/* === Priority Queue Interface === */

/**
 * @brief Initialize the priority queue system
 * 
 * This function initializes all necessary data structures for the priority queue system,
 * including setting up queues for each priority level.
 */
void sc_priority_init(void);

/**
 * @brief Publish raw event data to the priority queue
 * 
 * This function publishes raw event data to the appropriate priority queue
 * based on the priority field within the event data.
 * 
 * @param event_data Pointer to the event data to be published
 * @param size Size of the event data in bytes
 * @return int Returns 0 on success, negative value on failure
 */
int sc_priority_publish_raw(const uint8_t *event_data, size_t size);

/**
 * @brief Process events in the priority queue
 * 
 * This function processes events from the priority queues in order of priority,
 * handling the highest priority events first.
 */
void sc_priority_process(void);

/* === Priority Publishing Macros === */

/**
 * @brief Publish an emergency priority event
 * 
 * This macro asserts that the event size is within limits, sets the event priority
 * to emergency, and publishes the event data to the priority queue system.
 * 
 * @param evt_ptr Pointer to the event structure to be published
 */
#define SC_PUBLISH_EMERGENCY(evt_ptr) \
    (SC_STATIC_ASSERT(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event_too_large"), \
     ((void)((sc_event_t*)(evt_ptr))->priority = SC_PRIORITY_EMERGENCY), \
     sc_priority_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))))

/**
 * @brief Publish a standard priority event
 * 
 * This macro asserts that the event size is within limits, sets the event priority
 * to standard, and publishes the event data to the priority queue system.
 * 
 * @param evt_ptr Pointer to the event structure to be published
 */
#define SC_PUBLISH_STANDARD(evt_ptr) \
    (SC_STATIC_ASSERT(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event_too_large"), \
     ((void)((sc_event_t*)(evt_ptr))->priority = SC_PRIORITY_STANDARD), \
     sc_priority_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))))

/**
 * @brief Publish a low priority event
 * 
 * This macro asserts that the event size is within limits, sets the event priority
 * to low, and publishes the event data to the priority queue system.
 * 
 * @param evt_ptr Pointer to the event structure to be published
 */
#define SC_PUBLISH_LOW(evt_ptr) \
    (SC_STATIC_ASSERT(sizeof(*(evt_ptr)) <= SAFECORE_MAX_EVENT_SIZE, "Event_too_large"), \
     ((void)((sc_event_t*)(evt_ptr))->priority = SC_PRIORITY_LOW), \
     sc_priority_publish_raw((const uint8_t*)(evt_ptr), sizeof(*(evt_ptr))))

/* === Debug Interface === */

/**
 * @brief Get the current depth of a specific priority queue
 * 
 * This function returns the number of events currently in the specified priority queue.
 * 
 * @param priority Priority level of the queue to query
 * @return uint8_t Number of events in the specified priority queue
 */
uint8_t sc_priority_get_queue_depth(uint8_t priority);

/**
 * @brief Get statistics about the priority queue system
 * 
 * This function retrieves statistics about all priority queues, including the current
 * depth of each queue and the total number of dropped events.
 * 
 * @param depths Array to store the depth of each priority queue
 * @param dropped Pointer to store the total number of dropped events
 */
void sc_priority_get_stats(uint8_t *depths, uint32_t *dropped);

#endif /* SAFECORE_PRIORITY_ENABLED */
#endif /* SAFECORE_PRIORITY_H */