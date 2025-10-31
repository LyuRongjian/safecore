#include "safecore_diagnostics.h"
#include "safecore_port.h"
#include <string.h>

#if SAFECORE_DIAGNOSTICS_ENABLED == 1

/* === 静态数据 === */
static sc_dtc_type_t g_dtc_table[SAFECORE_MAX_DTCS];
static uint8_t g_dtc_count = 0;
static uint8_t g_dtc_initialized = 0;

/* === 函数实现 === */
int sc_diag_init(void) {
    if (g_dtc_initialized) {
        return -1;
    }
    
    (void)memset(g_dtc_table, 0, sizeof(g_dtc_table));
    g_dtc_count = 0;
    g_dtc_initialized = 1;
    
    return 0;
}

int sc_diag_report_dtc(uint16_t dtc_id, uint8_t severity) {
    if (!g_dtc_initialized) {
        return -1;
    }
    
    /* 查找或创建 DTC */
    uint8_t i;
    for (i = 0; i < g_dtc_count; i++) {
        if (g_dtc_table[i].dtc_id == dtc_id) {
            break;
        }
    }
    
    if (i >= SAFECORE_MAX_DTCS) {
        return -1; /* DTC 表满 */
    }
    
    if (i == g_dtc_count) {
        /* 新 DTC */
        g_dtc_table[i].dtc_id = dtc_id;
        g_dtc_table[i].timestamp = safecore_get_tick_ms();
        g_dtc_count++;
    }
    
    g_dtc_table[i].severity = severity;
    g_dtc_table[i].status |= 0x01; /* 测试失败位 */
    
    return 0;
}

int sc_diag_clear_dtc(uint16_t dtc_id) {
    if (!g_dtc_initialized) {
        return -1;
    }
    
    uint8_t i;
    for (i = 0; i < g_dtc_count; i++) {
        if (g_dtc_table[i].dtc_id == dtc_id) {
            (void)memset(&g_dtc_table[i], 0, sizeof(sc_dtc_type_t));
            
            /* 移动后续 DTC */
            for (uint8_t j = i; j < g_dtc_count - 1; j++) {
                g_dtc_table[j] = g_dtc_table[j + 1];
            }
            g_dtc_count--;
            return 0;
        }
    }
    
    return -1; /* 未找到 DTC */
}

int sc_diag_self_test(void) {
    if (!g_dtc_initialized) {
        return -1;
    }
    
    /* 执行自检逻辑 */
    uint32_t current_tick = safecore_get_tick_ms();
    
    /* 检查时间戳单调性 */
    if (current_tick < g_dtc_table[0].timestamp) {
        (void)sc_diag_report_dtc(0x1001, SC_DTC_FATAL);
        return -1;
    }
    
    /* 检查内存完整性 */
    uint32_t crc = 0; /* 计算 DTC 表 CRC */
    for (uint8_t i = 0; i < g_dtc_count; i++) {
        crc += g_dtc_table[i].dtc_id;
    }
    
    if (crc == 0 && g_dtc_count > 0) {
        (void)sc_diag_report_dtc(0x1002, SC_DTC_ERROR);
        return -1;
    }
    
    return 0;
}

#endif /* SAFECORE_DIAGNOSTICS_ENABLED */