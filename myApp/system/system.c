#include "nures_call.h"
#include "hf_we100.h"
#include "shell_port.h"
#include "tim.h"
#include "usart.h"
#include "stm32g0xx_hal.h"

void sys_Init(void)
{
    HAL_TIM_Base_Start_IT(&htim6);
    shell_Init();
    hf_Init();
    nurse_Init();
}

void sys_Run(void)
{
    shell_RunHld();
    hf_RunHld();
    nurse_RunHld();
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart1)
    {
    }
    if(huart == &huart2)
    {
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart1)
    {

    }
    if(huart == &huart2)
    {
        shellRxInt();
    }
    if(huart == &huart3)
    {
        hf_RxInt();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim6)
    {
        hf_TimeCnt();
        nurse_TimeCnt();
    }
}
