#include "urc.h"
#include "system.h"
#include "hf_we100.h"
#include "shell.h"
#include "usart.h"
#include "stdio.h"

typedef enum _e_nurse_rx_state
{
    E_NURSE_IDLE,
    E_NURSE_FINISH,
    E_NURSE_RX_STATE_MAX,
}E_NURSE_RX_STATE;

#define NURSE_SEND_INTERVAL 10
#define NURSE_REV_TIMEOUT   100
#define NURSE_RXBUF_SIZE    64
#define NURSE_TXBUF_SIZE    64

const char NURSE_HEAD[] = "A";
const char NURSE_SET[] = "+";
const char NURSE_RESET[] = "-";
const char NURSE_SEPARATOR[] = ":";
const char NURSE_END[] = "msgend";

uint32_t g_nurseTimeCnt = 0;
uint32_t g_nurseRxTimeCnt = 0;
E_HF_STATUS *pDevStatus;
char g_NurseRxBuff[NURSE_RXBUF_SIZE] = {0};
char g_NurseTxBuff[NURSE_TXBUF_SIZE] = {0};
E_NURSE_RX_STATE g_NurseState = E_NURSE_IDLE;
S_URC_MSG g_urcIns = {0};

static inline HAL_StatusTypeDef nurse_SendByte(uint8_t *pData, uint16_t Size)
{
    return HAL_UART_Transmit(&huart3, pData, Size, 0xffff);
}

void nurse_SendMsg(void)
{
    if(g_nurseTimeCnt < NURSE_SEND_INTERVAL)
    {
        return;
    }
    g_nurseTimeCnt = 0;

//    if(g_urcIns.finFlag == 1)//FIXME
    {
        sprintf((char *)g_NurseTxBuff, "%s%s%s%s%s%s%s%s",
                NURSE_HEAD, NURSE_SET, "1", NURSE_SEPARATOR, "even massage",
                NURSE_SEPARATOR, NURSE_END, "\r\n");
        nurse_SendByte((uint8_t *)g_NurseTxBuff, strlen(g_NurseTxBuff));
//        g_urcIns.finFlag = 0;
    }
}

//一帧报文数据接收完成，如果不解析不接收下一帧报文，如果此时报文较多，生成大于消费会导致队列溢出，这样就会接收到错误的报文。
void nurse_RevMsg(void)
{
    uint32_t ret = 0;

    switch(g_NurseState)
    {
    case E_NURSE_IDLE:
        ret = urc_RevFrame(&g_urcIns);
        if(ret == E_SUCCESS)
        {
            strcat((char *)g_urcIns.frame, "\r\n");
            shellWriteEndLine(getShellIns(), (char *)g_urcIns.frame, (int)g_urcIns.frameLen);
            g_NurseState = E_NURSE_FINISH;
        }
        break;
    case E_NURSE_FINISH:
//        if(g_urcIns.finFlag == 0)//FIXME
        {
            g_urcIns.finFlag = 0;//FIXME
            memset(g_NurseRxBuff, 0, sizeof(g_NurseRxBuff));
            g_NurseState = E_NURSE_IDLE;
        }
        break;
    default:
        break;
    }
}

void nurse_Init(void)
{
    pDevStatus = hf_GetDeviceStatus();
    g_NurseState = E_NURSE_IDLE;

    g_urcIns.head = (char *)NURSE_HEAD;
    g_urcIns.headLen = strlen(NURSE_HEAD);
    g_urcIns.end = (char *)NURSE_END;
    g_urcIns.endLen = strlen(NURSE_END);
    g_urcIns.frame = (char *)g_NurseRxBuff;
    g_urcIns.frameLen = sizeof(g_NurseRxBuff);
    g_urcIns.rbIns = hf_GetRbRxIns();
    g_urcIns.timeCnt = &g_nurseRxTimeCnt;
    g_urcIns.timeout = NURSE_REV_TIMEOUT;
    urc_Init(&g_urcIns);
}

void nurse_RunHld(void)
{
    if(*pDevStatus != E_HF_FINISH)
    {
        return ;
    }
    nurse_SendMsg();
    nurse_RevMsg();
}

void nurse_TimeCnt(void)
{
    if(g_nurseTimeCnt < 4000000000)
    {
        g_nurseTimeCnt++;
    }
    if(g_nurseRxTimeCnt < 4000000000)
    {
        g_nurseRxTimeCnt++;
    }
}

S_URC_MSG *nurse_GetUrcIns(void)
{
    return &g_urcIns;
}

