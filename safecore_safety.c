#include "safecore_safety.h"
#include "safecore_port.h"

#if SAFECORE_SAFETY_ENABLED == 1

sc_safety_status_t sc_safety_init(void) {
    return SAFECORE_SAFETY_OK;
}

sc_safety_status_t sc_safety_perform_self_test(void) {
    return SAFECORE_SAFETY_OK;
}

void sc_safety_enter_safe_state(void) {
    /* 进入安全状态逻辑 */
    while (1) {
        /* 安全停机 */
    }
}

int sc_watchdog_refresh(void) {
    return 0;
}

#endif /* SAFECORE_SAFETY_ENABLED */