#ifndef SAFECORE_COM_BRIDGE_H
#define SAFECORE_COM_BRIDGE_H

#include "safecore_types.h"
#include "safecore_config.h"

#if SAFECORE_COMM_ENABLED == 1

/* === 通信桥接接口 === */
int sc_com_init(void);
int sc_com_can_send_frame(const sc_can_frame_type_t *frame);
int sc_com_can_receive_frame(sc_can_frame_type_t *frame);

#endif /* SAFECORE_COMM_ENABLED */
#endif /* SAFECORE_COM_BRIDGE_H */