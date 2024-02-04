#ifndef _HF_WE100_H_
#define _HF_WE100_H_

#include "lwrb.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum _e_hf_status
{
    E_HF_REBOOT,
    E_HF_WAIT_CONNECT,
    E_HF_SET_CMDMODE,
    E_HF_WAIT_ASK_A,
    E_HF_WAIT_CMDMODE,
    E_HF_GET_DEVINFO,
	E_HF_TIMEOUT,
    E_HF_FINISH,
    E_HF_STATUS_MAX,
}E_HF_STATUS;

#define HF_TXBUFF_SIZE 64
#define HF_RXBUFF_SIZE 64

void hf_Init(void);
void hf_RunHld(void);
void hf_TimeCnt(void);
void hf_RxInt(void);

uint32_t hf_GetDeviceInfo(uint8_t cmd);
E_HF_STATUS *hf_GetDeviceStatus(void);
uint8_t *hf_GetRxBuff(void);
uint8_t *hf_GetTxBuff(void);
lwrb_t *hf_GetRbRxIns(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_HF_WE100_H_
