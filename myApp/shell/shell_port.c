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

#define RB_SHELL_TXBUF_SIZE 1024
lwrb_t rbShellTx;
uint8_t rbShellTxBuf[RB_SHELL_TXBUF_SIZE + 1];
uint8_t g_shellTransmited = 1; //0:doing

signed short shellGetCmd(char *data, unsigned short len)
{
    return lwrb_read(&rbShellRx, data, len);
}

signed short shellSendData(char *data, unsigned short len)
{    
    return lwrb_write(&rbShellTx, data, len);
}

void shell_Init(void)
{
    HAL_UART_Receive_IT(&huart2, &g_Uart2RxData, 1);
    shell.read = shellGetCmd;
    shell.write = shellSendData;

    lwrb_init(&rbShellRx, rbShellRxBuf, sizeof(rbShellRxBuf));
    lwrb_init(&rbShellTx, rbShellTxBuf, sizeof(rbShellTxBuf));
    g_shellTransmited = 1;

    shellInit(&shell, (char *)shell_buffer, sizeof(shell_buffer));
}

static inline void shell_TransmitData(void)
{
	if(lwrb_get_full(&rbShellTx) == 0 || g_shellTransmited == 0)
	{
		return ;
	}
	HAL_UART_Transmit_IT(&huart2, lwrb_get_linear_block_read_address(&rbShellTx), 1);
	g_shellTransmited = 0;
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

void shellRxInt(void)
{
	HAL_UART_Receive_IT(&huart2, &g_Uart2RxData, 1);
	lwrb_write(&rbShellRx, &g_Uart2RxData, sizeof(g_Uart2RxData));
}

void shellTxInt(void)
{
	lwrb_skip(&rbShellTx, 1);
	g_shellTransmited = 1;
}
