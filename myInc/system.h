#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define E_FAIL      (0)
#define E_SUCCESS   (1)
#define E_TIMEOUT   (2)

#define E_START     (0)
#define E_KEEP      (1)
#define E_STOP      (2)

void sys_Init(void);
void sys_Run(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_SYSTEM_H_
