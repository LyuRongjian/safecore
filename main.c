// main.c
#include "safecore_sm.h"
#include "safecore_eventbus.h"
#include "safecore_port.h"

volatile uint32_t g_tick = 0;

uint32_t safecore_get_tick_ms(void) {
    return g_tick;
}

void safecore_error_handler(const char *msg) {
    // 实际：记录DTC、进入安全状态、复位
    (void)msg;
    while (1); // 简化停机
}

// 事件 ID
typedef enum {
    EVT_FAULT_CRITICAL = 0,   // 高优先级
    EVT_BUTTON_PRESS,         // 中优先级
    EVT_LOG_INFO,             // 低优先级
    EVT_LED_TOGGLE,
    EVT_MAX
} app_event_id_t;

// 事件定义
typedef struct {
    sc_event_t super;
    uint16_t error_code;
} fault_event_t;

typedef struct {
    sc_event_t super;
    uint8_t pin;
} button_event_t;

typedef struct {
    sc_event_t super;
    char message[16];
} log_event_t;

// LED 状态机（略，同前）
typedef struct { int led_on; } led_ctx_t;
sc_sm_result_t led_top(void *ctx, const sc_sm_event_t *e, void **next);
sc_sm_result_t led_off(void *ctx, const sc_sm_event_t *e, void **next);
sc_sm_result_t led_on(void *ctx, const sc_sm_event_t *e, void **next);

void on_led_toggle(const sc_event_t *e, void *ctx) {
    sc_sm_event_t sm_ev = { .type = SC_SM_EVENT_USER_START };
    sc_sm_dispatch((sc_state_machine_t*)ctx, &sm_ev);
}

sc_sm_result_t led_top(void *ctx, const sc_sm_event_t *e, void **next) {
    if (e->type == SC_SM_EVENT_INIT) {
        *next = led_off;
        return SC_SM_TRANSITION;
    }
    return SC_SM_SUPER;
}

sc_sm_result_t led_off(void *ctx, const sc_sm_event_t *e, void **next) {
    if (e->type == SC_SM_EVENT_ENTRY) {
        ((led_ctx_t*)ctx)->led_on = 0;
        return SC_SM_HANDLED;
    }
    if (e->type == SC_SM_EVENT_USER_START) {
        *next = led_on;
        return SC_SM_TRANSITION;
    }
    return SC_SM_SUPER;
}

sc_sm_result_t led_on(void *ctx, const sc_sm_event_t *e, void **next) {
    if (e->type == SC_SM_EVENT_ENTRY) {
        ((led_ctx_t*)ctx)->led_on = 1;
        return SC_SM_HANDLED;
    }
    if (e->type == SC_SM_EVENT_USER_START) {
        *next = led_off;
        return SC_SM_TRANSITION;
    }
    return SC_SM_SUPER;
}

// 主程序
sc_state_machine_t g_led_sm;
led_ctx_t g_led_ctx;

int main(void) {
    sc_eventbus_init();
    sc_sm_init(&g_led_sm, led_top, &g_led_ctx, "led");
    sc_eventbus_subscribe(EVT_LED_TOGGLE, on_led_toggle, &g_led_sm);

    sc_sm_event_t init_ev = { .type = SC_SM_EVENT_INIT };
    sc_sm_dispatch(&g_led_sm, &init_ev);

    while (1) {
        sc_eventbus_process();

        static uint32_t last_fault = 0, last_btn = 0, last_log = 0;

        // 模拟关键故障（高优先级）
        if (g_tick - last_fault >= 5000) {
            fault_event_t ev = {0};
            ev.super.id = EVT_FAULT_CRITICAL;
            ev.error_code = 0x1234;
            SC_PUBLISH_HIGH(&ev); // 立即处理
            last_fault = g_tick;
        }

        // 普通按钮（中优先级）
        if (g_tick - last_btn >= 1000) {
            button_event_t ev = {0};
            ev.super.id = EVT_BUTTON_PRESS;
            ev.pin = 5;
            SC_PUBLISH_MEDIUM(&ev);
            last_btn = g_tick;
        }

        // 日志（低优先级）
        if (g_tick - last_log >= 200) {
            log_event_t ev = {0};
            ev.super.id = EVT_LOG_INFO;
            (void)snprintf(ev.message, sizeof(ev.message), "tick=%lu", (unsigned long)g_tick);
            SC_PUBLISH_LOW(&ev);
        }

        g_tick++;
    }
}