// main.c
#include "safecore_sm.h"
#include "safecore_eventbus.h"
#include "safecore_filters.h"
#include "safecore_port.h"

// 模拟 tick
volatile uint32_t g_tick = 0;
uint32_t safecore_get_tick_ms(void) { return g_tick; }

void safecore_error_handler(const char *msg) {
    SC_LOG("ERROR: %s", msg);
    while (1); // 安全停机
}

// 事件定义
typedef enum {
    EVT_BUTTON_PRESS = 0,
    EVT_EMERGENCY_STOP,
    EVT_SENSOR_UPDATE,
    EVT_LOG_INFO,
    EVT_MAX
} app_event_id_t;

// 紧急停止事件
typedef struct {
    sc_event_t super;
    uint8_t source_id;
    uint16_t error_code;
} emergency_event_t;

// 按钮事件
typedef struct {
    sc_event_t super;
    uint8_t pin;
    uint8_t state;
} button_event_t;

// LED 状态机
typedef struct { int led_on; } led_ctx_t;

sc_sm_result_t led_top(void *ctx, const sc_sm_event_t *e, void **next);
sc_sm_result_t led_off(void *ctx, const sc_sm_event_t *e, void **next);
sc_sm_result_t led_on(void *ctx, const sc_sm_event_t *e, void **next);

// 订阅者回调
void on_emergency(const sc_event_t *e, void *ctx) {
    const emergency_event_t *ee = (const emergency_event_t*)e;
    SC_LOG("EMERGENCY from %d, code %d", ee->source_id, ee->error_code);
    // 立即安全停机逻辑
}

void on_button(const sc_event_t *e, void *ctx) {
    const button_event_t *be = (const button_event_t*)e;
    SC_LOG("Button %d: %s", be->pin, be->state ? "PRESSED" : "RELEASED");
}

// 动态规则加载示例
void load_runtime_rules(void) {
    // 定义运行时过滤规则
    sc_filter_rule_t rules[] = {
        {1, SC_FILTER_TYPE_ALLOW, EVT_EMERGENCY_STOP, 0},  // 允许紧急停止
        {1, SC_FILTER_TYPE_ALLOW, EVT_BUTTON_PRESS, 0},   // 允许按钮
        {1, SC_FILTER_TYPE_SIZE_MIN, 0, 4},               // 最小4字节
        {1, SC_FILTER_TYPE_PRIORITY, 0, 1},               // 最低标准优先级
    };
    
    if (sc_filters_load_rules_from_buffer((uint8_t*)rules, sizeof(rules)) == 0) {
        SC_LOG("Dynamic rules loaded successfully");
    } else {
        SC_LOG("Failed to load dynamic rules");
    }
}

// 主程序
int main(void) {
    sc_eventbus_init();
    
    // 订阅事件
    sc_eventbus_subscribe(EVT_EMERGENCY_STOP, on_emergency, NULL);
    sc_eventbus_subscribe(EVT_BUTTON_PRESS, on_button, NULL);

    // 加载动态过滤规则
    load_runtime_rules();

    while (1) {
        sc_eventbus_process();

        // 模拟事件生成
        static uint32_t last_event = 0;
        if (g_tick - last_event >= 1000) {
            // 紧急事件（高优先级）
            emergency_event_t emg = {0};
            emg.super.id = EVT_EMERGENCY_STOP;
            emg.source_id = 1;
            emg.error_code = 0x1001;
            SC_PUBLISH_EMERGENCY(&emg); // 立即处理

            // 按钮事件（标准优先级）
            button_event_t btn = {0};
            btn.super.id = EVT_BUTTON_PRESS;
            btn.pin = 5;
            btn.state = 1;
            SC_PUBLISH_STANDARD(&btn);

            last_event = g_tick;
        }

        // 调试信息
        static uint32_t last_debug = 0;
        if (g_tick - last_debug >= 5000) {
            uint8_t depths[3];
            uint32_t dropped[3];
            sc_eventbus_get_stats(depths, dropped);
            SC_LOG("Queue depths: H=%d M=%d L=%d, Dropped: H=%d M=%d L=%d", 
                   depths[0], depths[1], depths[2], dropped[0], dropped[1], dropped[2]);
            last_debug = g_tick;
        }

        g_tick++; // 模拟 SysTick
    }
}