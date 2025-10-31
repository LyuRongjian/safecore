// safecore_sm.c
#include "safecore_sm.h"
#include "safecore_port.h"
#include "safecore_config.h"
#include <string.h>

void sc_sm_init(sc_state_machine_t *sm, sc_sm_handler_t top, void *ctx, const char *name) {
    if (!sm || !top) {
        SAFECORE_ON_ERROR("SM init: null pointer");
        return;
    }
    (void)memset(sm, 0, sizeof(*sm));
    sm->handlers[0] = top;
    sm->user_ctx = ctx;
    sm->name = name ? name : "sm";
}

void sc_sm_dispatch(sc_state_machine_t *sm, const sc_sm_event_t *e) {
    if (!sm || !e) {
        SAFECORE_ON_ERROR("SM dispatch: null pointer");
        return;
    }
    if (sm->depth >= SAFECORE_MAX_HSM_DEPTH) {
        SAFECORE_ON_ERROR("SM: max depth exceeded");
        return;
    }

    sc_sm_event_t ev = *e;
    ev.timestamp = safecore_get_tick_ms();

    for (int i = (int)sm->depth; i >= 0; i--) {
        if (!sm->handlers[i]) {
            SAFECORE_ON_ERROR("SM: null handler");
            return;
        }
        void *next = NULL;
        sc_sm_result_t r = sm->handlers[i](sm->contexts[i], &ev, &next);
        if (r == SC_SM_HANDLED) return;
        if (r == SC_SM_TRANSITION) {
            if (!next) {
                SAFECORE_ON_ERROR("SM: transition to null state");
                return;
            }
            // Exit current path
            for (int j = (int)sm->depth; j >= 0; j--) {
                if (sm->handlers[j]) sc_sm_send_exit(sm, sm->contexts[j]);
            }
            sm->depth = 0;
            sm->handlers[0] = (sc_sm_handler_t)next;
            sm->contexts[0] = sm->user_ctx;
            sc_sm_send_entry(sm, sm->contexts[0]);
            return;
        }
    }
}

void sc_sm_send_entry(sc_state_machine_t *sm, void *ctx) {
#if SAFECORE_ENTRY_EXIT_ENABLED
    if (!sm || !ctx) return;
    sc_sm_event_t e = { .type = SC_SM_EVENT_ENTRY, .timestamp = safecore_get_tick_ms() };
    if (sm->handlers[sm->depth]) {
        (void)sm->handlers[sm->depth](ctx, &e, NULL);
    }
#endif
}

void sc_sm_send_exit(sc_state_machine_t *sm, void *ctx) {
#if SAFECORE_ENTRY_EXIT_ENABLED
    if (!sm || !ctx) return;
    sc_sm_event_t e = { .type = SC_SM_EVENT_EXIT, .timestamp = safecore_get_tick_ms() };
    if (sm->handlers[sm->depth]) {
        (void)sm->handlers[sm->depth](ctx, &e, NULL);
    }
#endif
}