#include "safecore_com_bridge.h"
#include "safecore_config.h"
#include <string.h>

#if SAFECORE_COMM_ENABLED == 1

int sc_com_init(void) {
    return 0;
}

int sc_com_can_send_frame(const sc_can_frame_type_t *frame) {
    if (!frame) return -1;
    /* 实际 CAN 发送逻辑 */
    return 0;
}

int sc_com_can_receive_frame(sc_can_frame_type_t *frame) {
    if (!frame) return -1;
    /* 实际 CAN 接收逻辑 */
    return 0;
}

#endif /* SAFECORE_COMM_ENABLED */