/**
 * @file safecore_port.h
 * 
 * SafeCore Platform Port Interface
 * This header file provides platform abstraction functions
 * that allow SafeCore to be ported to different hardware platforms.
 */

#ifndef SAFECORE_PORT_H
#define SAFECORE_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get current system tick in milliseconds
 * 
 * This function returns the current system tick count in milliseconds.
 * It must be implemented by the platform to provide timing information
 * to the SafeCore framework.
 * 
 * @return uint32_t Current system tick in milliseconds
 */
extern uint32_t safecore_get_tick_ms(void);

/**
 * @brief Handle critical errors
 * 
 * This function is called when a critical error is detected by the
 * SafeCore framework. The platform must implement appropriate error
 * handling, which typically includes logging the error and entering
 * a safe shutdown state.
 * 
 * @param msg Error message describing the critical condition
 */
extern void safecore_error_handler(const char *msg);

#ifdef __cplusplus
}
#endif

#endif // SAFECORE_PORT_H