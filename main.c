/*
 * main.c
 * 
 * SafeCore Framework Examples
 * This file demonstrates multiple example implementations of the SafeCore
 * framework, showcasing different versions and features.
 */
#include "safecore_types.h"
#include "safecore_port.h"
#include "safecore_config.h"
#include "safecore_core.h"
#include "safecore_safety.h"

/* === Global Variables === */
volatile uint32_t g_tick = 0U; /* System tick counter */

/* === Platform Interface Implementation === */

/**
 * @brief Get current system tick in milliseconds
 * 
 * This function returns the current system tick count, used for timing
 * operations and periodic tasks.
 * 
 * @return uint32_t Current tick count in milliseconds
 */
uint32_t safecore_get_tick_ms(void) {
    return g_tick;
}

/**
 * @brief Error handler for SafeCore operations
 * 
 * This function handles errors detected by the SafeCore framework,
 * logging the error and entering a safe shutdown state.
 * 
 * @param msg Error message to log
 */
void safecore_error_handler(const char *msg) {
    SC_LOG("ERROR: %s", msg);
    for (;;) { /* Safety shutdown */ }
}

/* === Example Applications === */

/**
 * @brief SafeCore Basic Example
 * 
 * This example demonstrates the basic functionality of SafeCore,
 * including state machine implementation and event handling.
 */
void basic_example(void) {
    // Redefine configuration for basic example
    #undef SAFECORE_PRIORITY_ENABLED
    #define SAFECORE_PRIORITY_ENABLED           0
    #undef SAFECORE_FILTERS_ENABLED
    #define SAFECORE_FILTERS_ENABLED            0
    #undef SAFECORE_DIAGNOSTICS_ENABLED
    #define SAFECORE_DIAGNOSTICS_ENABLED        0
    #undef SAFECORE_COMM_ENABLED
    #define SAFECORE_COMM_ENABLED               0
    #undef SAFECORE_SAFETY_ENABLED
    #define SAFECORE_SAFETY_ENABLED             0

    /**
     * @brief Application event identifiers
     */
    typedef enum {
        EVT_BUTTON_PRESS = 0,
        EVT_LED_TOGGLE,
        EVT_MAX
    } app_event_id_t;

    /**
     * @brief Button press event structure
     */
    typedef struct {
        sc_event_t super; /* Base event structure */
        uint8_t pin;      /* Pin number that triggered the event */
    } button_event_t;

    /* LED State Machine */
    typedef struct { int32_t led_on; } led_ctx_t;

    /**
     * @brief LED state machine top-level handler
     * 
     * @param ctx Context pointer for the state machine
     * @param e Event being processed
     * @param next Pointer to store next state handler
     * @return sc_sm_result_t Result of state processing
     */
    sc_sm_result_t led_top(void *ctx, const sc_sm_event_t *e, void **next) {
        if (e->type == SC_EVENT_INIT) {
            *next = NULL; /* Simplified example */
            return SC_SM_TRANSITION;
        }
        return SC_SM_SUPER;
    }

    /**
     * @brief Handler for LED toggle events
     * 
     * @param e Event being processed
     * @param ctx Context pointer (LED state machine)
     */
    void on_led_toggle(const sc_event_t *e, void *ctx) {
        sc_sm_event_t sm_ev = { .type = SC_EVENT_USER_START };
        // sc_sm_dispatch((sc_state_machine_t*)ctx, &sm_ev);
    }

    /* Main program variables */
    sc_state_machine_t g_led_sm;
    led_ctx_t g_led_ctx;

    /* Initialize components */
    sc_eventbus_init();
    sc_sm_init(&g_led_sm, led_top, &g_led_ctx, "led");
    sc_eventbus_subscribe(EVT_LED_TOGGLE, on_led_toggle, &g_led_sm);

    /* Send initialization event */
    sc_sm_event_t init_ev = { .type = SC_EVENT_INIT };
    sc_sm_dispatch(&g_led_sm, &init_ev);

    /* Run test loop */
    uint8_t i;
    for (i = 0U; i < 10U; i++) {
        sc_eventbus_process();
        
        /* Generate periodic events */
        static uint32_t last = 0U;
        if (safecore_get_tick_ms() - last >= 2000U) {
            button_event_t ev = {0};
            ev.super.id = EVT_BUTTON_PRESS;
            ev.pin = 5;
            SC_PUBLISH_EVENT(&ev);
            
            sc_event_t toggle = { .id = EVT_LED_TOGGLE };
            SC_PUBLISH_EVENT(&toggle);
            
            last = safecore_get_tick_ms();
        }
        
        /* Increment tick counter */
        g_tick++;
    }
}

/**
 * @brief SafeCore Priority Example
 * 
 * This example demonstrates the priority-based event handling
 * and filtering capabilities of SafeCore.
 */
void priority_example(void) {
    // Redefine configuration for priority example
    #undef SAFECORE_PRIORITY_ENABLED
    #define SAFECORE_PRIORITY_ENABLED           1
    #undef SAFECORE_FILTERS_ENABLED
    #define SAFECORE_FILTERS_ENABLED            1
    #undef SAFECORE_DIAGNOSTICS_ENABLED
    #define SAFECORE_DIAGNOSTICS_ENABLED        0
    #undef SAFECORE_COMM_ENABLED
    #define SAFECORE_COMM_ENABLED               0
    #undef SAFECORE_SAFETY_ENABLED
    #define SAFECORE_SAFETY_ENABLED             0

    /**
     * @brief Application event identifiers
     */
    typedef enum {
        EVT_EMERGENCY_STOP = 0,
        EVT_BUTTON_PRESS,
        EVT_LOG_INFO,
        EVT_MAX
    } app_event_id_t;

    /**
     * @brief Emergency stop event structure
     */
    typedef struct {
        sc_event_t super;      /* Base event structure */
        uint8_t source_id;     /* Source of the emergency stop */
        uint16_t error_code;   /* Error code associated with stop */
    } emergency_event_t;

    /* Initialize components */
    sc_eventbus_init();
    
    /* Load filter rules */
    sc_filter_rule_t rules[] = {
        {1, SC_FILTER_TYPE_ALLOW, EVT_EMERGENCY_STOP, 0},
        {1, SC_FILTER_TYPE_ALLOW, EVT_BUTTON_PRESS, 0},
        {1, SC_FILTER_TYPE_SIZE_MIN, 0, 4},
    };
    sc_filters_load_rules_from_buffer((uint8_t*)rules, sizeof(rules));

    /* Run test loop */
    uint8_t i;
    for (i = 0U; i < 10U; i++) {
        sc_eventbus_process();
        
        /* Generate periodic events with different priorities */
        static uint32_t last = 0U;
        if (safecore_get_tick_ms() - last >= 1000U) {
            /* Emergency event (high priority) */
            emergency_event_t emg;
            (void)memset(&emg, 0, sizeof(emg));
            emg.super.id = EVT_EMERGENCY_STOP;
            SC_PUBLISH_EMERGENCY(&emg);
            
            /* Button event (standard priority) */
            sc_event_t btn = {0U};
            btn.id = EVT_BUTTON_PRESS;
            SC_PUBLISH_STANDARD(&btn);
            
            /* Log event (low priority) */
            sc_event_t log = {0U};
            log.id = EVT_LOG_INFO;
            SC_PUBLISH_LOW(&log);
            
            last = safecore_get_tick_ms();
        }
        
        /* Display queue statistics */
        uint8_t depths[3];
        uint32_t dropped[3];
        sc_priority_get_stats(depths, dropped);
        SC_LOG("Queue: E=%d S=%d L=%d", depths[0], depths[1], depths[2]);
        
        /* Increment tick counter */
        g_tick++;
    }
}

/**
 * @brief SafeCore Automotive Example
 * 
 * This example demonstrates the automotive features of SafeCore,
 * including safety mechanisms, diagnostics, and communication capabilities.
 */
void automotive_example(void) {
    // Redefine configuration for automotive example
    #undef SAFECORE_AUTOSAR_ENABLED
    #define SAFECORE_AUTOSAR_ENABLED            1
    #undef SAFECORE_DIAGNOSTICS_ENABLED
    #define SAFECORE_DIAGNOSTICS_ENABLED        1
    #undef SAFECORE_COMM_ENABLED
    #define SAFECORE_COMM_ENABLED               1
    #undef SAFECORE_CAN_ENABLED
    #define SAFECORE_CAN_ENABLED                1
    #undef SAFECORE_UDS_ENABLED
    #define SAFECORE_UDS_ENABLED                1
    #undef SAFECORE_SAFETY_ENABLED
    #define SAFECORE_SAFETY_ENABLED             1
    #undef SAFECORE_WATCHDOG_INTEGRATION
    #define SAFECORE_WATCHDOG_INTEGRATION       1
    #undef SAFECORE_SELF_TEST_ENABLED
    #define SAFECORE_SELF_TEST_ENABLED          1

    /* Safety initialization */
    #if SAFECORE_SAFETY_ENABLED == 1
    if (sc_safety_init() != SAFECORE_SAFETY_OK) {
        for (;;) { /* Safety shutdown */ }
    }
    #endif
    
    /* System initialization */
    sc_eventbus_init();
    #if SAFECORE_DIAGNOSTICS_ENABLED == 1
    sc_diag_init();
    #endif
    #if SAFECORE_COMM_ENABLED == 1
    sc_com_init();
    #endif
    
    /* Self-test */
    #if SAFECORE_DIAGNOSTICS_ENABLED == 1
    if (sc_diag_self_test() != 0) {
        #if SAFECORE_SAFETY_ENABLED == 1
        sc_safety_enter_safe_state();
        #endif
    }
    #endif
    
    /* Run test loop */
    for (int i = 0; i < 10; i++) {
        sc_eventbus_process();
        
        /* Safety checks */
        #if SAFECORE_SAFETY_ENABLED == 1
        if (sc_safety_perform_self_test() != SAFECORE_SAFETY_OK) {
            sc_safety_enter_safe_state();
        }
        #endif
        
        /* Watchdog refresh */
        #if SAFECORE_WATCHDOG_INTEGRATION == 1
        sc_watchdog_refresh();
        #endif
        
        /* Increment tick counter */
        g_tick++;
    }
}

/**
 * @brief Main function
 * 
 * Entry point for the SafeCore examples program. Runs all three example
 * implementations in sequence.
 * 
 * @return int 0 on successful completion
 */
int main(void) {
    int result = 0;
    
    SC_LOG("SafeCore Modular - Starting Examples");
    
    SC_LOG("=== Running Basic Example ===");
    basic_example();
    
    SC_LOG("=== Running Priority Example ===");
    priority_example();
    
    SC_LOG("=== Running Automotive Example ===");
    automotive_example();
    
    SC_LOG("=== All Examples Completed ===");
    
    return result;
}