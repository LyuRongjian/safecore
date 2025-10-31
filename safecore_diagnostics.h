/*
 * safecore_diagnostics.h
 * 
 * SafeCore Diagnostics Module
 * This header file defines the diagnostics subsystem for the SafeCore framework,
 * providing error handling, fault detection, and diagnostic information
 * management capabilities.
 */

#ifndef SAFECORE_DIAGNOSTICS_H
#define SAFECORE_DIAGNOSTICS_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_DIAGNOSTICS_ENABLED == 1

/**
 * @defgroup SafeCore_DIAGNOSTICS SafeCore Diagnostics Module
 * @brief Diagnostic and error handling system for SafeCore framework
 * @{
 */

/* === Configuration === */

/**
 * @brief Maximum number of diagnostic entries
 * 
 * This macro defines the maximum number of diagnostic entries that can be
 * stored in the SafeCore diagnostics system.
 */
#ifndef SAFECORE_DIAG_MAX_ENTRIES
#define SAFECORE_DIAG_MAX_ENTRIES 128
#endif

/**
 * @brief Enable cyclic redundancy check for diagnostic data
 * 
 * This macro controls whether CRC validation is performed on diagnostic data
 * to ensure its integrity.
 */
#ifndef SAFECORE_DIAG_CRC_ENABLED
#define SAFECORE_DIAG_CRC_ENABLED 1
#endif

/* === Enumerations === */

/**
 * @brief Diagnostic error level
 * 
 * Defines the severity levels for diagnostic errors.
 */
typedef enum {
    SC_DIAG_LEVEL_INFO = 0,      /**< Informational message */
    SC_DIAG_LEVEL_WARNING,       /**< Warning message */
    SC_DIAG_LEVEL_ERROR,         /**< Error message */
    SC_DIAG_LEVEL_CRITICAL,      /**< Critical error requiring immediate attention */
    SC_DIAG_LEVEL_FATAL          /**< Fatal error causing system shutdown */
} sc_diag_level_t;

/**
 * @brief Diagnostic error source
 * 
 * Identifies the source of a diagnostic error within the system.
 */
typedef enum {
    SC_DIAG_SOURCE_CORE = 0,     /**< Error from SafeCore core */
    SC_DIAG_SOURCE_APP,          /**< Error from application code */
    SC_DIAG_SOURCE_HW,           /**< Error from hardware layer */
    SC_DIAG_SOURCE_COMM,         /**< Error from communication module */
    SC_DIAG_SOURCE_TIMING,       /**< Error from timing or scheduling */
    SC_DIAG_SOURCE_MEMORY,       /**< Error from memory management */
    SC_DIAG_SOURCE_SAFETY,       /**< Error from safety mechanisms */
    SC_DIAG_SOURCE_USER          /**< User-defined error source */
} sc_diag_source_t;

/* === Structures === */

/**
 * @brief Diagnostic entry structure
 * 
 * Represents a single diagnostic entry in the SafeCore diagnostics system.
 */
typedef struct {
    uint32_t timestamp;          /**< Timestamp when error was logged */
    sc_diag_level_t level;       /**< Severity level of the error */
    sc_diag_source_t source;     /**< Source of the error */
    uint16_t code;               /**< Error code */
    uint8_t active;              /**< Active/inactive status flag */
    uint16_t occurrence_count;   /**< Number of occurrences */
    uint8_t payload[4];          /**< Additional diagnostic data */
} sc_diag_entry_t;

/**
 * @brief Diagnostic system status
 * 
 * Contains status information about the diagnostic system itself.
 */
typedef struct {
    uint8_t active_entries;      /**< Number of active diagnostic entries */
    uint8_t max_severity_level;  /**< Highest severity level currently active */
    uint32_t total_errors;       /**< Total number of errors recorded */
    uint8_t overflow_flag;       /**< Flag indicating buffer overflow */
    uint32_t last_crc;           /**< Last CRC value calculated */
} sc_diag_system_status_t;

/* === Function Prototypes === */

/**
 * @brief Initialize the diagnostics system
 * 
 * Initializes the SafeCore diagnostics subsystem, setting up data structures
 * and resetting internal state.
 * 
 * @return int Status of initialization (0 on success, negative on error)
 */
int sc_diag_init(void);

/**
 * @brief Report a diagnostic trouble code (DTC)
 * 
 * Records a diagnostic trouble code with the specified ID and severity.
 * 
 * @param dtc_id Diagnostic trouble code identifier
 * @param severity Severity level of the DTC
 * @return int Status of reporting (0 on success, negative on error)
 */
int sc_diag_report_dtc(uint16_t dtc_id, uint8_t severity);

/**
 * @brief Clear a diagnostic trouble code (DTC)
 * 
 * Clears a specific diagnostic trouble code identified by its ID.
 * 
 * @param dtc_id Diagnostic trouble code identifier to clear
 * @return int Status of clearing operation (0 on success, negative on error)
 */
int sc_diag_clear_dtc(uint16_t dtc_id);

/**
 * @brief Perform self-test of the diagnostics system
 * 
 * Executes a self-test of the diagnostics subsystem to verify its
 * functionality and data integrity.
 * 
 * @return int Status of self-test (0 on success, negative on error)
 */
int sc_diag_self_test(void);

/** @} */

#endif /* SAFECORE_DIAGNOSTICS_ENABLED */
#endif /* SAFECORE_DIAGNOSTICS_H */