// safecore_port.h
#ifndef SAFECORE_PORT_H
#define SAFECORE_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t safecore_get_tick_ms(void);
extern void safecore_error_handler(const char *msg);

#ifdef __cplusplus
}
#endif

#endif // SAFECORE_PORT_H