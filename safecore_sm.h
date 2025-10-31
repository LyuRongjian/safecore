// safecore_sm.h
#ifndef SAFECORE_SM_H
#define SAFECORE_SM_H

#include "safecore_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    sc_sm_handler_t handlers[SAFECORE_MAX_HSM_DEPTH];
    void *contexts[SAFECORE_MAX_HSM_DEPTH];
    uint8_t depth;
    void *user_ctx;
    const char *name;
} sc_state_machine_t;

void sc_sm_init(sc_state_machine_t *sm, sc_sm_handler_t top, void *ctx, const char *name);
void sc_sm_dispatch(sc_state_machine_t *sm, const sc_sm_event_t *e);
void sc_sm_send_entry(sc_state_machine_t *sm, void *ctx);
void sc_sm_send_exit(sc_state_machine_t *sm, void *ctx);

#ifdef __cplusplus
}
#endif
#endif