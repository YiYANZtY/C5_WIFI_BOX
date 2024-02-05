#include "urc.h"
#include "system.h"

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

uint32_t urc_RevFrame(S_URC_MSG *urcIns)
{
    uint32_t ret = E_FAIL;

    switch(urcIns->state)
    {
    case E_IDLE:
        ret = urc_CheckHead(urcIns->rbIns, urcIns->head, urcIns->headLen);
        if(ret != E_SUCCESS)
        {
            break;
        }
        memset(urcIns->frame, 0, urcIns->frameLen);
        *urcIns->timeCnt = 0;
        urcIns->state = E_WAIT_END;
        break;
    case E_WAIT_END:
        if(*urcIns->timeCnt > urcIns->timeout)
        {//超时
            memset(urcIns->frame, 0, urcIns->frameLen);
            urcIns->state = E_IDLE;
            break;
        }
        ret = urc_RevData(urcIns->rbIns, urcIns->end, urcIns->endLen, (char *)urcIns->frame);
        if(ret != E_SUCCESS)
        {
            break;
        }
        urcIns->finFlag = 1;
        urcIns->state = E_FINISH;
        break;
    case E_FINISH:
        if(urcIns->finFlag == 0)
        {
            urcIns->state = E_IDLE;
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
	urcIns->state = E_IDLE;
}
