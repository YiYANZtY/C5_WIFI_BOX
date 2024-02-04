#include "lwrb.h"
#include "shell_port.h"
#include "shell.h"
#include "usart.h"

Shell shell;
uint8_t shell_buffer[512];
uint8_t g_Uart2RxData = 0;

#define RB_SHELL_RXBUF_SIZE 64
lwrb_t rbShellRx;
uint8_t rbShellRxBuf[RB_SHELL_RXBUF_SIZE + 1];

signed short shellGetCmd(char *data, unsigned short len)
{
    return lwrb_read(&rbShellRx, data, len);
}

signed short shellSendData(char *data, unsigned short len)
{    
    return HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 0xffff);
}

void shell_Init(void)
{
    HAL_UART_Receive_IT(&huart2, &g_Uart2RxData, 1);
    shell.read = shellGetCmd;
    shell.write = shellSendData;

    lwrb_init(&rbShellRx, rbShellRxBuf, sizeof(rbShellRxBuf));

    shellInit(&shell, (char *)shell_buffer, sizeof(shell_buffer));
}

void shell_RunHld(void)
{
    shellTask(&shell);
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
