#ifndef _SHELL_PORT_H_
#define _SHELL_PORT_H_

#include "stm32g0xx_hal.h"
#include "stddef.h"

typedef struct shell_def Shell;
/**
 * @brief 是否使用默认shell任务while循环
 *        使能此宏，则`shellTask()`函数会一直循环读取输入，一般使用操作系统建立shell
 *        任务时使能此宏，关闭此宏的情况下，一般适用于无操作系统，在主循环中调用`shellTask()`
 */
#define     SHELL_TASK_WHILE            0
/**
 * @brief 支持shell尾行模式
 */
#define     SHELL_SUPPORT_END_LINE      1
/**
 * @brief 获取系统时间(ms)
 *        定义此宏为获取系统Tick，如`HAL_GetTick()`
 * @note 此宏不定义时无法使用双击tab补全命令help，无法使用shell超时锁定
 */
#define     SHELL_GET_TICK()            HAL_GetTick()

void shell_Init(void);
Shell *getShellIns(void);
void shell_RunHld(void);
void shell_RxInt(void);
void shell_RxErrorCb(void);
void shell_TxInt(void);

#endif /*_SHELL_PORT_H_*/
