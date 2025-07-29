#ifndef _CAPELL_PCDN_SDK_H_
#define _CAPELL_PCDN_SDK_H_

#include <cstdint>
#include "config.h"

DCDN_NS_BEGIN

int InitSDK();

int CreateTask(uint64_t* taskId);

int StartTask(uint64_t taskId);

int CancelTask(uint64_t taskId);

DCDN_NS_END

#endif
