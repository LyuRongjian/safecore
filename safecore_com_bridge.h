/*
 * safecore_com_bridge.h
 * 
 * SafeCore Communication Bridge Interface
 * This header file defines the communication interface for SafeCore,
 * providing abstraction layers for various communication protocols
 * such as CAN used in embedded systems.
 */

#ifndef SAFECORE_COM_BRIDGE_H
#define SAFECORE_COM_BRIDGE_H

#include "safecore_types.h"
#include "safecore_config.h"

/**
 * @defgroup SafeCore_COM SafeCore Communication Module
 * @brief Communication interface abstraction for SafeCore framework
 * @{
 */

#if SAFECORE_COMM_ENABLED == 1

/**
 * @brief Initialize SafeCore communication module
 * 
 * This function initializes the SafeCore communication bridge,
 * setting up the necessary infrastructure for managing communication channels.
 * 
 * @return int Status of initialization (0 for success, negative for error)
 */
int sc_com_init(void);

/**
 * @brief Send a CAN frame
 * 
 * Transmits a CAN frame through the communication system.
 * 
 * @param frame Pointer to the CAN frame to be transmitted
 * @return int Status of transmission (0 for success, negative for error)
 */
int sc_com_can_send_frame(const sc_can_frame_type_t *frame);

/**
 * @brief Receive a CAN frame
 * 
 * Receives a CAN frame from the communication system.
 * 
 * @param frame Pointer to store the received CAN frame
 * @return int Status of reception (0 for success, negative for error)
 */
int sc_com_can_receive_frame(sc_can_frame_type_t *frame);

#endif /* SAFECORE_COMM_ENABLED */

/**
 * @}
 */

#endif /* SAFECORE_COM_BRIDGE_H */