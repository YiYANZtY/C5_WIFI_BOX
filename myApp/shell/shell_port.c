#include "lwrb.h"
#include "shell_port.h"
#include "shell.h"
#include "usart.h"

Shell shell;
uint8_t shell_buffer[512];

#define RB_SHELL_RXBUF_SIZE 64
lwrb_t rbShellRx;
uint8_t rbShellRxBuf[RB_SHELL_RXBUF_SIZE + 1];
uint8_t g_Uart2RxData = 0;

#define RB_SHELL_TXBUF_SIZE 512
lwrb_t rbShellTx;
uint8_t rbShellTxBuf[RB_SHELL_TXBUF_SIZE + 1];
uint8_t g_shellTransmited = 1; //0:doing

static inline void shell_TransmitData(void)
{
	if(lwrb_get_full(&rbShellTx) == 0 || g_shellTransmited == 0)
	{
		return ;
	}
	HAL_UART_Transmit_IT(&huart2, lwrb_get_linear_block_read_address(&rbShellTx), 1);
	g_shellTransmited = 0;
}

static inline void shell_StartReceiveIT(uint8_t *pData, uint16_t Size)
{
	HAL_StatusTypeDef ret;
	ret = HAL_UART_Receive_IT(&huart2, pData, Size);
	if(ret != HAL_OK)
	{
		if(ret == HAL_BUSY)
		{//如果数据流较大，溢出会导致停止接收数据，无视溢出情况。
			__HAL_UART_CLEAR_OREFLAG(&huart2);
			huart2.RxState = HAL_UART_STATE_READY;
			huart2.Lock = HAL_UNLOCKED;
			ret = HAL_UART_Receive_IT(&huart2, pData, Size);
		}
	}
}

signed short shellGetCmd(char *data, unsigned short len)
{
    return lwrb_read(&rbShellRx, data, len);
}

signed short shellSendData(char *data, unsigned short len)
{
	int tmp = 0;
	tmp = lwrb_get_free(&rbShellTx);

	while(tmp < len)
	{
		tmp = lwrb_get_free(&rbShellTx);
		shell_TransmitData();
	}
    return lwrb_write(&rbShellTx, data, len);
}

void shell_Init(void)
{
    shell_StartReceiveIT(&g_Uart2RxData, 1);
    shell.read = shellGetCmd;
    shell.write = shellSendData;

    lwrb_init(&rbShellRx, rbShellRxBuf, sizeof(rbShellRxBuf));
    lwrb_init(&rbShellTx, rbShellTxBuf, sizeof(rbShellTxBuf));
    g_shellTransmited = 1;

    shellInit(&shell, (char *)shell_buffer, sizeof(shell_buffer));
}

void shell_RunHld(void)
{
    shellTask(&shell);
    shell_TransmitData();
}

Shell *getShellIns(void)
{
    return &shell;
}

void shell_RxInt(void)
{
    shell_StartReceiveIT(&g_Uart2RxData, 1);
	lwrb_write(&rbShellRx, &g_Uart2RxData, sizeof(g_Uart2RxData));
}

void shell_RxErrorCb(void)
{
	//如果数据流较大，溢出会导致停止接收数据，无视溢出情况。
	if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE) != RESET)
	{
		__HAL_UART_CLEAR_OREFLAG(&huart2);
		HAL_UART_Receive_IT(&huart2, &g_Uart2RxData, 1);
	}
}

void shell_TxInt(void)
{
	lwrb_skip(&rbShellTx, 1);
	g_shellTransmited = 1;
    shell_TransmitData();
}
