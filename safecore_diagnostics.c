/*
 * safecore_diagnostics.c
 * 
 * SafeCore Diagnostics Implementation
 * This file implements diagnostic capabilities for SafeCore, including
 * DTC (Diagnostic Trouble Code) management and self-test functionality.
 */
#include "safecore_diagnostics.h"
#include "safecore_port.h"
#include <string.h>

#if SAFECORE_DIAGNOSTICS_ENABLED == 1

/** 
 * @brief Static Data
 * 
 * These variables maintain the state of the diagnostic system.
 */
static sc_dtc_type_t g_dtc_table[SAFECORE_MAX_DTCS];    /* Table of active DTCs */
static uint8_t g_dtc_count = 0;                         /* Number of active DTCs */
static uint8_t g_dtc_initialized = 0;                   /* Initialization flag */

/**
 * @brief Initialize the diagnostic system
 * 
 * This function initializes the diagnostic module by clearing the DTC table
 * and resetting all counters and flags.
 * 
 * @return int 0 on success, -1 if already initialized
 */
int sc_diag_init(void) {
    if (g_dtc_initialized) {
        return -1; /* Already initialized */
    }
    
    (void)memset(g_dtc_table, 0U, sizeof(g_dtc_table));
    g_dtc_count = 0U;
    g_dtc_initialized = 1U;
    
    return 0;
}

/**
 * @brief Report a Diagnostic Trouble Code (DTC)
 * 
 * This function reports a new or updates an existing DTC in the diagnostic system.
 * If the DTC already exists, it updates the severity and status. If it's new,
 * it adds it to the table with a timestamp.
 * 
 * @param dtc_id The unique identifier of the DTC
 * @param severity The severity level of the DTC
 * @return int 0 on success, -1 if not initialized or DTC table is full
 */
int sc_diag_report_dtc(uint16_t dtc_id, uint8_t severity) {
    if (!g_dtc_initialized) {
        return -1; /* Not initialized */
    }
    
    /* Find existing DTC or position for new one */
    uint8_t i;
    for (i = 0U; i < g_dtc_count; i++) {
        if (g_dtc_table[i].dtc_id == dtc_id) {
            break; /* DTC already exists */
        }
    }
    
    if (i >= SAFECORE_MAX_DTCS) {
        return -1; /* DTC table is full */
    }
    
    if (i == g_dtc_count) {
        /* New DTC */
        g_dtc_table[i].dtc_id = dtc_id;
        g_dtc_table[i].timestamp = safecore_get_tick_ms();
        g_dtc_count++;
    }
    
    /* Update DTC information */
    g_dtc_table[i].severity = severity;
    g_dtc_table[i].status |= 0x01; /* Set test failed bit */
    
    return 0;
}

/**
 * @brief Clear a Diagnostic Trouble Code (DTC)
 * 
 * This function removes a specific DTC from the diagnostic system by ID.
 * It also reorganizes the DTC table to fill the gap left by the removed DTC.
 * 
 * @param dtc_id The unique identifier of the DTC to clear
 * @return int 0 on success, -1 if not initialized or DTC not found
 */
int sc_diag_clear_dtc(uint16_t dtc_id) {
    if (!g_dtc_initialized) {
        return -1; /* Not initialized */
    }
    
    uint8_t i;
    for (i = 0U; i < g_dtc_count; i++) {
        if (g_dtc_table[i].dtc_id == dtc_id) {
            /* Clear the DTC entry */
            (void)memset(&g_dtc_table[i], 0, sizeof(sc_dtc_type_t));
            
            /* Shift remaining DTCs to fill the gap */
            uint8_t j;
            for (j = i; j < (g_dtc_count - 1U); j++) {
                g_dtc_table[j] = g_dtc_table[j + 1];
            }
            
            /* Decrement the DTC count */
            g_dtc_count--;
            return 0;
        }
    }
    
    return -1; /* DTC not found */
}

/**
 * @brief Perform diagnostic system self-test
 * 
 * This function performs self-tests on the diagnostic system itself, checking
 * for timestamp consistency and basic memory integrity.
 * 
 * @return int 0 if self-test passed, -1 if not initialized or self-test failed
 */
int sc_diag_self_test(void) {
    if (!g_dtc_initialized) {
        return -1; /* Not initialized */
    }
    
    /* Execute self-test logic */
    uint32_t current_tick = safecore_get_tick_ms();
    
    /* Check timestamp monotonicity */
    for (uint8_t i = 1U; i < g_dtc_count; i++) {
        if (g_dtc_table[i].timestamp < g_dtc_table[i-1].timestamp) {
            (void)sc_diag_report_dtc(0x1001, SC_DTC_FATAL); /* Report timestamp error */
            return -1;
        }
    }
    
    /* Check memory integrity */
    uint32_t crc = 0U; /* Simple checksum of DTC IDs */
    uint8_t i;
    
    for (i = 0U; i < g_dtc_count; i++) {
        crc += g_dtc_table[i].dtc_id;
    }
    
    /* Verify integrity of non-empty table */
    if ((crc == 0U) && (g_dtc_count > 0U)) {
        (void)sc_diag_report_dtc(0x1002U, SC_DTC_ERROR); /* Report memory integrity error */
        return -1;
    }
    
    return 0; /* Self-test passed */
}

#endif /* SAFECORE_DIAGNOSTICS_ENABLED */