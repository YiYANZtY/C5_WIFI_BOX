#include "hf_we100.h"
#include "urc.h"
#include "system.h"
#include "usart.h"
#include "shell.h"
#include "stm32g0xx_hal.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct _s_hf_device S_HF_DEVICE;

//命令
const char CMD_SET_CMDMODE[] = "+++";
const char ASK_SET_CMDMODE[] = "a";
const char REBOOT_DEVICE[] = "HFAT+Z";
const char INTO_THROUGH_PUT[] = "HFAT+ENTM";
const char SOCKA_NET_INFO[] = "HFAT+NETP";
const char WS_SSID[] = "HFAT+WSSSID";
const char WS_KEY[] = "HFAT+WSKEY";
const char GET_STA_INFO[] = "HFAT+WANN";

//应答
const char ASK_CMDMODE[] = "+ok";
const char ASK_OK[] = "+ok=";
const char ASK_EVENT_CON_ON[] = "+EVENT=CON_ON";
const char ASK_EVENT_DHCP_OK[] = "+EVENT=DHCP_OK";
const char ASK_HEAD[] = "+";
const char ASK_END[] = "\r\n";

#define RB_HF_RXBUF_SIZE 64
#define COMMUNICAT_TIMEOUT 2000
#define CONNECT_TIMEOUT 15000

//串口接受数据
uint8_t g_Uart3RxData = 0;
uint8_t rbHfRxBuf[RB_HF_RXBUF_SIZE + 1];

//汉枫接受发送数据
uint8_t hfTxBuff[HF_TXBUFF_SIZE] = {0};
uint8_t hfRxBuff[HF_RXBUFF_SIZE] = {0};
uint8_t tmpRxBuff[HF_RXBUFF_SIZE] = {0};

//系统参数
E_HF_STATUS g_hfStatus;
uint32_t g_hfTimeCnt = 0;
lwrb_t rbHfRx;
S_HF_DEVICE g_hfDevInfo = {0};
S_URC_MSG g_hfUrcIns = {0};

static void hf_ResTimeCnt(void)
{
    g_hfTimeCnt = 0;
}

static inline HAL_StatusTypeDef hf_SendByte(uint8_t *pData, uint16_t Size)
{
    return HAL_UART_Transmit(&huart3, pData, Size, 0xffff);
}

static inline void hf_StartReceiveIT(uint8_t *pData, uint16_t Size)
{
	HAL_StatusTypeDef ret;
	ret = HAL_UART_Receive_IT(&huart3, pData, Size);
	if(ret != HAL_OK)
	{
		if(ret == HAL_BUSY)
		{//如果数据流较大，溢出会导致停止接收数据，无视溢出情况。
			__HAL_UART_CLEAR_OREFLAG(&huart3);
			huart3.RxState = HAL_UART_STATE_READY;
			huart3.Lock = HAL_UNLOCKED;
			ret = HAL_UART_Receive_IT(&huart3, pData, Size);
		}
	}
}

uint32_t hf_Reboot(void)
{
    memset(hfTxBuff, 0, sizeof(hfTxBuff));

    sprintf((char *)hfTxBuff, "%s%s", REBOOT_DEVICE, "\r\n");
    hf_SendByte((uint8_t *)hfTxBuff, sizeof(hfTxBuff));
    hf_ResTimeCnt();

    return E_SUCCESS;
}

uint32_t hf_WaitConnect(void)
{
    memset(hfRxBuff, 0, sizeof(hfRxBuff));

    if(g_hfTimeCnt >= CONNECT_TIMEOUT)
    {//Timeout
        return E_TIMEOUT;
    }

    return urc_CheckHead(&rbHfRx, ASK_EVENT_DHCP_OK, strlen(ASK_EVENT_DHCP_OK));
}

static uint32_t hf_GetDeviceInfo(uint8_t cmd)
{
	static uint8_t step = 0;
    uint8_t len = 0, cnt = 0;

    memset(hfTxBuff, 0, sizeof(hfTxBuff));
    memset(hfRxBuff, 0, sizeof(hfRxBuff));
    len = len;

    if(cmd == E_START)
    {
        step = 0;
    }
    switch(step)
    {
    case 0://protocol
        sprintf((char *)hfTxBuff, "%s%s", SOCKA_NET_INFO, "\r\n");
        hf_SendByte((uint8_t *)hfTxBuff, sizeof(hfTxBuff));
        hf_ResTimeCnt();
        step++;
        break;
    case 1://head
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {//Timeout
            step = 0xaa;
            return E_TIMEOUT;
        }
        if(lwrb_peek(&rbHfRx, 0, hfRxBuff, strlen(ASK_OK)) != strlen(ASK_OK))
        {//接受数据不足
            return E_FAIL;
        }
        if(strcmp((char *)hfRxBuff, ASK_OK) != 0)
        {//接收数据错误
            lwrb_skip(&rbHfRx, 1);
            return E_FAIL;
        }
        lwrb_skip(&rbHfRx, strlen(ASK_OK));
        step++;
        break;
    case 2://protocol
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, ",") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.protocol, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 3://cs
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, ",") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.cs, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 4://port
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, ",") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        g_hfDevInfo.port = atoi((char *)hfRxBuff);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 5://ip
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, "\r") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.ip, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 6://ssid
        sprintf((char *)hfTxBuff, "%s%s", WS_SSID, "\r\n");
        hf_SendByte((uint8_t *)hfTxBuff, sizeof(hfTxBuff));
        hf_ResTimeCnt();
        step++;
        break;
    case 7:
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {//Timeout
            step = 0xaa;
            return E_TIMEOUT;
        }
        if(lwrb_peek(&rbHfRx, 0, hfRxBuff, strlen(ASK_OK)) != strlen(ASK_OK))
        {//接受数据不足
            return E_FAIL;
        }
        if(strcmp((char *)hfRxBuff, ASK_OK) != 0)
        {//接收数据错误
            lwrb_skip(&rbHfRx, 1);
            return E_FAIL;
        }
        lwrb_skip(&rbHfRx, strlen(ASK_OK));
        step++;
        break;
    case 8:
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, "\r") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.ssid, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 9://password
        sprintf((char *)hfTxBuff, "%s%s", WS_KEY, "\r\n");
        hf_SendByte((uint8_t *)hfTxBuff, sizeof(hfTxBuff));
        hf_ResTimeCnt();
        step++;
        break;
    case 10:
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {//Timeout
            step = 0xaa;
            return E_TIMEOUT;
        }
        if(lwrb_peek(&rbHfRx, 0, hfRxBuff, strlen(ASK_OK)) != strlen(ASK_OK))
        {//接受数据不足
            return E_FAIL;
        }
        if(strcmp((char *)hfRxBuff, ASK_OK) != 0)
        {//接收数据错误
            lwrb_skip(&rbHfRx, 1);
            return E_FAIL;
        }
        lwrb_skip(&rbHfRx, strlen(ASK_OK));
        step++;
        break;
    case 11:
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, "\r") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.password, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 12://ssid
        sprintf((char *)hfTxBuff, "%s%s", GET_STA_INFO, "\r\n");
        hf_SendByte((uint8_t *)hfTxBuff, sizeof(hfTxBuff));
        hf_ResTimeCnt();
        step++;
        break;
    case 13:
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {//Timeout
            step = 0xaa;
            return E_TIMEOUT;
        }
        if(lwrb_peek(&rbHfRx, 0, hfRxBuff, strlen(ASK_OK)) != strlen(ASK_OK))
        {//接受数据不足
            return E_FAIL;
        }
        if(strcmp((char *)hfRxBuff, ASK_OK) != 0)
        {//接收数据错误
            lwrb_skip(&rbHfRx, 1);
            return E_FAIL;
        }
        lwrb_skip(&rbHfRx, strlen(ASK_OK));
        step++;
        break;
    case 14://mode
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, ",") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.mode, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 15://addr
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, ",") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.addr, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 16://mask
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, ",") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.mask, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 17://gateway
        len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        while(strcmp((char *)hfRxBuff, "\r") != 0 && g_hfTimeCnt < COMMUNICAT_TIMEOUT)
        {
            cnt++;
            len = lwrb_peek(&rbHfRx, cnt, hfRxBuff, 1);
        }
        if(g_hfTimeCnt >= COMMUNICAT_TIMEOUT)
        {
            return E_TIMEOUT;
        }
        lwrb_read(&rbHfRx, hfRxBuff, cnt);
        strncpy(g_hfDevInfo.gateway, (char *)hfRxBuff, cnt);
        lwrb_skip(&rbHfRx, 1);//skip ","
        step++;
        break;
    case 18:
        return E_SUCCESS;
        break;
    default:
        break;
    }

    return E_FAIL;
}

static uint32_t hf_SetCmdMode(void)
{
    hf_SendByte((uint8_t *)CMD_SET_CMDMODE, strlen(CMD_SET_CMDMODE));
    lwrb_reset(&rbHfRx);

    return E_SUCCESS;
}

static uint32_t hf_WaitAskA(void)
{
    if(lwrb_peek(&rbHfRx, 0, hfRxBuff, strlen(ASK_SET_CMDMODE)) != strlen(ASK_SET_CMDMODE))
    {//接受数据不足
        return E_FAIL;
    }
    if(strcmp((char *)hfRxBuff, ASK_SET_CMDMODE) != 0)
    {//接收数据错误
        lwrb_skip(&rbHfRx, 1);
        return E_FAIL;
    }
    lwrb_skip(&rbHfRx, strlen(ASK_SET_CMDMODE));
    hf_SendByte((uint8_t *)ASK_SET_CMDMODE, strlen(ASK_SET_CMDMODE));

    return E_SUCCESS;
}

static uint32_t hf_WaitCmdMode(void)
{
    if(lwrb_peek(&rbHfRx, 0, hfRxBuff, strlen(ASK_CMDMODE)) != strlen(ASK_CMDMODE))
    {//接受数据不足
        return E_FAIL;
    }
    if(strcmp((char *)hfRxBuff, ASK_CMDMODE) != 0)
    {//接收数据错误
        lwrb_skip(&rbHfRx, 1);
        return E_FAIL;
    }
    lwrb_skip(&rbHfRx, strlen(ASK_CMDMODE));

    return E_SUCCESS;
}

void hf_Init(void)
{
    g_hfStatus = E_HF_REBOOT;
    lwrb_init(&rbHfRx, rbHfRxBuf, sizeof(rbHfRxBuf));
	hf_StartReceiveIT(&g_Uart3RxData, 1);

    g_hfUrcIns.head = (char *)ASK_HEAD;
    g_hfUrcIns.headLen = strlen(ASK_HEAD);
    g_hfUrcIns.end = (char *)ASK_END;
    g_hfUrcIns.endLen = strlen(ASK_END);
    g_hfUrcIns.frame = (char *)tmpRxBuff;
    g_hfUrcIns.frameLen = sizeof(tmpRxBuff);
    g_hfUrcIns.rbIns = &rbHfRx;
    g_hfUrcIns.timeCnt = &g_hfTimeCnt;
    g_hfUrcIns.timeout = COMMUNICAT_TIMEOUT;
    urc_Init(&g_hfUrcIns);
}

void hf_RunHld(void)
{
    uint32_t ret = E_FAIL;
    memset(hfRxBuff, 0, sizeof(hfRxBuff));

    switch(g_hfStatus)
    {
    case E_HF_REBOOT:
        ret = hf_Reboot();
        if(ret == E_SUCCESS)
        {
            g_hfStatus = E_HF_WAIT_CONNECT;
        }
        break;
    case E_HF_WAIT_CONNECT:
        ret = hf_WaitConnect();
        if(ret == E_SUCCESS)
        {
            hf_GetDeviceInfo(E_START);
            g_hfStatus = E_HF_GET_DEVINFO;
        }
        else if(ret == E_TIMEOUT)
        {
        	g_hfStatus = E_HF_TIMEOUT;
        }
        break;
    case E_HF_SET_CMDMODE:
        ret = hf_SetCmdMode();
        if(ret == E_SUCCESS)
        {
            g_hfStatus = E_HF_WAIT_ASK_A;
        }
        break;
    case E_HF_WAIT_ASK_A:
        ret = hf_WaitAskA();
        if(ret == E_SUCCESS)
        {
            g_hfStatus = E_HF_WAIT_CMDMODE;
        }
        break;
    case E_HF_WAIT_CMDMODE:
        ret = hf_WaitCmdMode();
        if(ret == E_SUCCESS)
        {
            g_hfStatus = E_HF_FINISH;
        }
        break;
    case E_HF_GET_DEVINFO:
        ret = hf_GetDeviceInfo(E_KEEP);
        if(ret == E_SUCCESS)
        {
            g_hfStatus = E_HF_FINISH;
        }
        break;
    case E_HF_TIMEOUT:
    	break;
    case E_HF_SHELL:
		if(urc_RevFrame(&g_hfUrcIns) == E_SUCCESS)
		{
			shellWriteEndLine(getShellIns(), (char *)tmpRxBuff, sizeof(tmpRxBuff));
			g_hfUrcIns.finFlag = 0;
			g_hfStatus = E_HF_FINISH;
		}
		break;
    case E_HF_FINISH:
        break;
    default:
        break;
    }
}

void hf_TimeCnt(void)
{
    if(g_hfTimeCnt < 4000000000)
    {
        g_hfTimeCnt++;
    }
}

void hf_RxInt(void)
{
	hf_StartReceiveIT(&g_Uart3RxData, 1);
    lwrb_write(&rbHfRx, &g_Uart3RxData, sizeof(g_Uart3RxData));
}

void hf_RxErrorCb(void)
{
	//如果数据流较大，溢出会导致停止接收数据，无视溢出情况。
	if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_ORE) != RESET)
	{
		__HAL_UART_CLEAR_OREFLAG(&huart3);
		HAL_UART_Receive_IT(&huart3, &g_Uart3RxData, 1);
	}
}

E_HF_STATUS *hf_GetDeviceStatus(void)
{
    return &g_hfStatus;
}

uint8_t *hf_GetRxBuff(void)
{
    return hfRxBuff;
}

uint8_t *hf_GetTxBuff(void)
{
    return hfTxBuff;
}

lwrb_t *hf_GetRbRxIns(void)
{
    return &rbHfRx;
}

S_HF_DEVICE *hf_GetDevIns(void)
{
	return &g_hfDevInfo;
}
