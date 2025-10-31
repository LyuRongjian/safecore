#include "safecore_types.h"
#include "safecore_port.h"
#include "safecore_config.h"
#include "safecore_core.h"
#include "safecore_safety.h"

// 平台接口实现
volatile uint32_t g_tick = 0;

uint32_t safecore_get_tick_ms(void) {
    return g_tick;
}

void safecore_error_handler(const char *msg) {
    SC_LOG("ERROR: %s", msg);
    while (1); /* 安全停机 */
}

/* === 示例应用 === */

// examples/basic_example.c - SafeCore v2.0 基础版
void basic_example(void) {
    // 重新定义配置为 v2.0
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

    typedef enum {
        EVT_BUTTON_PRESS = 0,
        EVT_LED_TOGGLE,
        EVT_MAX
    } app_event_id_t;

    typedef struct {
        sc_event_t super;
        uint8_t pin;
    } button_event_t;

    /* LED 状态机 */
    typedef struct { int led_on; } led_ctx_t;

    sc_sm_result_t led_top(void *ctx, const sc_sm_event_t *e, void **next) {
        if (e->type == SC_EVENT_INIT) {
            *next = NULL; /* 简化示例 */
            return SC_SM_TRANSITION;
        }
        return SC_SM_SUPER;
    }

    void on_led_toggle(const sc_event_t *e, void *ctx) {
        sc_sm_event_t sm_ev = { .type = SC_EVENT_USER_START };
        // sc_sm_dispatch((sc_state_machine_t*)ctx, &sm_ev);
    }

    /* 主程序 */
    sc_state_machine_t g_led_sm;
    led_ctx_t g_led_ctx;

    sc_eventbus_init();
    sc_sm_init(&g_led_sm, led_top, &g_led_ctx, "led");

    sc_eventbus_subscribe(EVT_LED_TOGGLE, on_led_toggle, &g_led_sm);

    sc_sm_event_t init_ev = { .type = SC_EVENT_INIT };
    sc_sm_dispatch(&g_led_sm, &init_ev);

    for (int i = 0; i < 10; i++) {
        sc_eventbus_process();
        
        static uint32_t last = 0;
        if (safecore_get_tick_ms() - last >= 2000) {
            button_event_t ev = {0};
            ev.super.id = EVT_BUTTON_PRESS;
            ev.pin = 5;
            SC_PUBLISH_EVENT(&ev);
            
            sc_event_t toggle = { .id = EVT_LED_TOGGLE };
            SC_PUBLISH_EVENT(&toggle);
            
            last = safecore_get_tick_ms();
        }
        
        g_tick++;
    }
}

// examples/priority_example.c - SafeCore v3.0 优先级版
void priority_example(void) {
    // 重新定义配置为 v3.0
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

    typedef enum {
        EVT_EMERGENCY_STOP = 0,
        EVT_BUTTON_PRESS,
        EVT_LOG_INFO,
        EVT_MAX
    } app_event_id_t;

    /* 紧急停止事件 */
    typedef struct {
        sc_event_t super;
        uint8_t source_id;
        uint16_t error_code;
    } emergency_event_t;

    sc_eventbus_init();
    
    /* 加载过滤规则 */
    sc_filter_rule_t rules[] = {
        {1, SC_FILTER_TYPE_ALLOW, EVT_EMERGENCY_STOP, 0},
        {1, SC_FILTER_TYPE_ALLOW, EVT_BUTTON_PRESS, 0},
        {1, SC_FILTER_TYPE_SIZE_MIN, 0, 4},
    };
    sc_filters_load_rules_from_buffer((uint8_t*)rules, sizeof(rules));

    for (int i = 0; i < 10; i++) {
        sc_eventbus_process();
        
        static uint32_t last = 0;
        if (safecore_get_tick_ms() - last >= 1000) {
            /* 紧急事件（高优先级） */
            emergency_event_t emg = {0};
            emg.super.id = EVT_EMERGENCY_STOP;
            SC_PUBLISH_EMERGENCY(&emg);
            
            /* 按钮事件（标准优先级） */
            sc_event_t btn = {0};
            btn.id = EVT_BUTTON_PRESS;
            SC_PUBLISH_STANDARD(&btn);
            
            /* 日志事件（低优先级） */
            sc_event_t log = {0};
            log.id = EVT_LOG_INFO;
            SC_PUBLISH_LOW(&log);
            
            last = safecore_get_tick_ms();
        }
        
        /* 调试信息 */
        uint8_t depths[3];
        uint32_t dropped[3];
        sc_priority_get_stats(depths, dropped);
        SC_LOG("Queue: E=%d S=%d L=%d", depths[0], depths[1], depths[2]);
        
        g_tick++;
    }
}

// examples/automotive_example.c - SafeCore v4.0 汽车版
void automotive_example(void) {
    // 重新定义配置为 v4.0
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

    /* 安全初始化 */
    #if SAFECORE_SAFETY_ENABLED == 1
    if (sc_safety_init() != SAFECORE_SAFETY_OK) {
        while (1); /* 安全停机 */
    }
    #endif
    
    /* 系统初始化 */
    sc_eventbus_init();
    #if SAFECORE_DIAGNOSTICS_ENABLED == 1
    sc_diag_init();
    #endif
    #if SAFECORE_COMM_ENABLED == 1
    sc_com_init();
    #endif
    
    /* 自检 */
    #if SAFECORE_DIAGNOSTICS_ENABLED == 1
    if (sc_diag_self_test() != 0) {
        #if SAFECORE_SAFETY_ENABLED == 1
        sc_safety_enter_safe_state();
        #endif
    }
    #endif
    
    for (int i = 0; i < 10; i++) {
        sc_eventbus_process();
        
        /* 安全检查 */
        #if SAFECORE_SAFETY_ENABLED == 1
        if (sc_safety_perform_self_test() != SAFECORE_SAFETY_OK) {
            sc_safety_enter_safe_state();
        }
        #endif
        
        /* 看门狗刷新 */
        #if SAFECORE_WATCHDOG_INTEGRATION == 1
        sc_watchdog_refresh();
        #endif
        
        g_tick++;
    }
}

/* === 主函数 === */
int main(void) {
    SC_LOG("SafeCore Modular v4.0 - Starting Examples");
    
    SC_LOG("=== Running v2.0 Basic Example ===");
    basic_example();
    
    SC_LOG("=== Running v3.0 Priority Example ===");
    priority_example();
    
    SC_LOG("=== Running v4.0 Automotive Example ===");
    automotive_example();
    
    SC_LOG("=== All Examples Completed ===");
    
    return 0;
}