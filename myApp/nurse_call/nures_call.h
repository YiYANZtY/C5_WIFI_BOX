#ifndef _NURES_CALL_H_
#define _NURES_CALL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "urc.h"

void nurse_Init(void);
void nurse_RunHld(void);
void nurse_TimeCnt(void);
S_URC_MSG *nurse_GetUrcIns(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_NURES_CALL_H_
