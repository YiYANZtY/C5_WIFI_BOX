#include "urc.h"
#include "system.h"

typedef enum _e_urc_rx_state
{
    E_IDLE,
    E_WAIT_END,
    E_FINISH,
    E_URC_RX_STATE_MAX,
}E_URC_RX_STATE;

S_URC_MSG *pUrcIns = NULL;
E_URC_RX_STATE g_RxState = E_IDLE;

uint32_t urc_CheckHead(lwrb_t *pRbIns, const char *str, uint8_t len)
{
    uint8_t g_RxTmpBuf[8] = {0};

    if(lwrb_get_full(pRbIns) < len)
    {//数据不够
        return E_FAIL;
    }
    lwrb_peek(pRbIns, 0, g_RxTmpBuf, len);
    if(strcmp((char *)g_RxTmpBuf, str) != 0)
    {//数据不正确
        lwrb_skip(pRbIns, 1);
        return E_FAIL;
    }

    return E_SUCCESS;
}

uint32_t urc_RevData(lwrb_t *pRbIns, const char *str, uint8_t len, char *rxbuf)
{
    uint8_t g_RxTmpBuf[8] = {0};

    if(lwrb_get_full(pRbIns) < len)
    {//数据不够
        return E_FAIL;
    }
    lwrb_peek(pRbIns, 0, g_RxTmpBuf, len);
    if(strcmp((char *)g_RxTmpBuf, str) != 0)
    {
        lwrb_read(pRbIns, g_RxTmpBuf, 1);
        strncat((char *)rxbuf, (char *)g_RxTmpBuf, 1);
        return E_FAIL;
    }
    lwrb_read(pRbIns, g_RxTmpBuf, len);
    strncat((char *)rxbuf, (char *)g_RxTmpBuf, len);

    return E_SUCCESS;
}

uint32_t urc_RevFrame(void)
{
    uint32_t ret = E_FAIL;

    switch(g_RxState)
    {
    case E_IDLE:
        ret = urc_CheckHead(pUrcIns->rbIns, pUrcIns->head, pUrcIns->headLen);
        if(ret != E_SUCCESS)
        {
            break;
        }
        memset(pUrcIns->frame, 0, pUrcIns->frameLen);
        *pUrcIns->timeCnt = 0;
        g_RxState = E_WAIT_END;
        break;
    case E_WAIT_END:
        if(*pUrcIns->timeCnt > pUrcIns->timeout)
        {//超时
            memset(pUrcIns->frame, 0, pUrcIns->frameLen);
            g_RxState = E_IDLE;
            break;
        }
        ret = urc_RevData(pUrcIns->rbIns, pUrcIns->end, pUrcIns->endLen, (char *)pUrcIns->frame);
        if(ret != E_SUCCESS)
        {
            break;
        }
        pUrcIns->finFlag = 1;
        g_RxState = E_FINISH;
        break;
    case E_FINISH:
        if(pUrcIns->finFlag == 0)
        {
            g_RxState = E_IDLE;
            break;
        }
        return E_SUCCESS;
        break;
    default:
        break;
    }

    return E_FAIL;
}

void urc_Init(S_URC_MSG *urcIns)
{
    pUrcIns = urcIns;
    g_RxState = E_IDLE;
}
