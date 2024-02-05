#include "urc.h"
#include "nures_call.h"
#include "system.h"
#include "hf_we100.h"
#include "usart.h"
#include "shell_port.h"
#include "shell.h"
#include "stdlib.h"
#include "stdio.h"

int func(int argc, char *argv[])
{
    shellPrint(shellGetCurrent(), "%dparameter(s)\r\n", argc);
    for (int i = 1; i < argc; i++)
    {
    	shellPrint(shellGetCurrent(), "%s\r\n", argv[i]);
    }

    return argc;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), func, func, test);

int hfGetDevInfo(int argc, char *argv[])
{
	char str[16];
	S_HF_DEVICE * pHfDevIns = hf_GetDevIns();

	shellWriteEndLine(shellGetCurrent(), pHfDevIns->ssid, strlen(pHfDevIns->ssid));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	shellWriteEndLine(shellGetCurrent(), pHfDevIns->password, strlen(pHfDevIns->password));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	shellWriteEndLine(shellGetCurrent(), pHfDevIns->mode, strlen(pHfDevIns->mode));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	shellWriteEndLine(shellGetCurrent(), pHfDevIns->addr, strlen(pHfDevIns->addr));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	shellWriteEndLine(shellGetCurrent(), pHfDevIns->mask, strlen(pHfDevIns->mask));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	shellWriteEndLine(shellGetCurrent(), pHfDevIns->gateway, strlen(pHfDevIns->gateway));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	shellWriteEndLine(shellGetCurrent(), pHfDevIns->protocol, strlen(pHfDevIns->protocol));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	shellWriteEndLine(shellGetCurrent(), pHfDevIns->cs, strlen(pHfDevIns->cs));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	shellWriteEndLine(shellGetCurrent(), pHfDevIns->ip, strlen(pHfDevIns->ip));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));
	itoa((int)pHfDevIns->port, str, 10);
	shellWriteEndLine(shellGetCurrent(), str, strlen(str));
	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));

    return argc;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), hfGetDevInfo, hfGetDevInfo, hfGetDevInfo);

int hfSetDevCmd(int argc, char *argv[])
{
	if(argc != 2)
	{
		return 0;
	}
	HAL_UART_Transmit(&huart3, argv[1], strlen(argv[1]), 0xffff);
	HAL_UART_Transmit(&huart3, "\r\n", strlen("\r\n"), 0xffff);

//	while(urc_RevFrame() == E_SUCCESS);
//	shellWriteEndLine(shellGetCurrent(), pUrcIns->frame, strlen(pUrcIns->frame));
//	shellWriteEndLine(shellGetCurrent(), "\r\n", strlen("\r\n"));

    return argc;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), hfSetDevCmd, hfSetDevCmd, hfSetDevCmd);

