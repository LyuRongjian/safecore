#ifndef SAFECORE_SAFETY_H
#define SAFECORE_SAFETY_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_SAFETY_ENABLED == 1

/* === 安全机制接口 === */
typedef enum {
    SAFECORE_SAFETY_OK,
    SAFECORE_SAFETY_WARNING,
    SAFECORE_SAFETY_ERROR,
    SAFECORE_SAFETY_FATAL
} sc_safety_status_t;

sc_safety_status_t sc_safety_init(void);
sc_safety_status_t sc_safety_perform_self_test(void);
void sc_safety_enter_safe_state(void);
int sc_watchdog_refresh(void);

#endif /* SAFECORE_SAFETY_ENABLED */
#endif /* SAFECORE_SAFETY_H */