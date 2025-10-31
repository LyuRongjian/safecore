/*
 * safecore_safety.h
 * 
 * SafeCore Safety Mechanisms
 * This header file defines the safety mechanisms for the SafeCore framework,
 * providing functionality for watchdog management, self-testing, and
 * safe state management in embedded systems.
 */

#ifndef SAFECORE_SAFETY_H
#define SAFECORE_SAFETY_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_SAFETY_ENABLED == 1

/**
 * @defgroup SafeCore_SAFETY SafeCore Safety Module
 * @brief Safety mechanisms for SafeCore framework
 * @{
 */

/* === Safety Interface === */

/**
 * @brief Safety status enumeration
 * 
 * Defines the possible safety status values returned by the safety module.
 */
typedef enum {
    SAFECORE_SAFETY_OK,      /**< Safety status is normal */
    SAFECORE_SAFETY_WARNING, /**< Safety warning detected */
    SAFECORE_SAFETY_ERROR,   /**< Safety error detected */
    SAFECORE_SAFETY_FATAL    /**< Fatal safety error detected */
} sc_safety_status_t;

/**
 * @brief Initialize the safety module
 * 
 * Initializes the SafeCore safety mechanisms, including watchdog integration
 * if enabled, and sets up the initial safety state.
 * 
 * @return sc_safety_status_t Initial safety status
 */
sc_safety_status_t sc_safety_init(void);

/**
 * @brief Perform a self-test of safety mechanisms
 * 
 * Executes a comprehensive self-test of the safety module and related
 * safety-critical components.
 * 
 * @return sc_safety_status_t Result of the self-test
 */
sc_safety_status_t sc_safety_perform_self_test(void);

/**
 * @brief Enter safe state
 * 
 * Transitions the system into a predefined safe state, ensuring that
 * all safety-critical operations are properly handled.
 */
void sc_safety_enter_safe_state(void);

/**
 * @brief Refresh the watchdog
 * 
 * Updates the watchdog timer to prevent system reset. This function
 * should be called periodically from the main loop or by critical tasks.
 * 
 * @return int Status code indicating success or failure
 */
int sc_watchdog_refresh(void);

/**
 * @}
 */

#endif /* SAFECORE_SAFETY_ENABLED */
#endif /* SAFECORE_SAFETY_H */