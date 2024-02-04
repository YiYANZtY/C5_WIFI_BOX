#ifndef _URC_H_
#define _URC_H_

#include "lwrb.h"
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _s_urc_msg
{
    lwrb_t *rbIns;
    char *head;
    uint8_t headLen;
    char *end;
    uint8_t endLen;
    char *frame;
    uint8_t frameLen;
    uint32_t timeout;
    uint32_t *timeCnt;
    uint8_t finFlag;
}S_URC_MSG;

uint32_t urc_CheckHead(lwrb_t *pRbIns, const char *str, uint8_t len);
uint32_t urc_RevData(lwrb_t *pRbIns, const char *str, uint8_t len, char *rxbuf);
uint32_t urc_RevFrame(void);
void urc_Init(S_URC_MSG *urcIns);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_URC_H_
