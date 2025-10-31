#ifndef SAFECORE_DIAGNOSTICS_H
#define SAFECORE_DIAGNOSTICS_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_DIAGNOSTICS_ENABLED == 1

/* === 诊断接口 === */
int sc_diag_init(void);
int sc_diag_report_dtc(uint16_t dtc_id, uint8_t severity);
int sc_diag_clear_dtc(uint16_t dtc_id);
int sc_diag_self_test(void);

#endif /* SAFECORE_DIAGNOSTICS_ENABLED */
#endif /* SAFECORE_DIAGNOSTICS_H */