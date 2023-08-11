#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tConfig.h"
#include "tEvent.h"
#include "tTask.h"
#include "tSem.h"
#include "tMbox.h"
#include "tMemBlock.h"
#include "tFlagGroup.h"
#include "tMutex.h"
#include "tTimer.h"
#include "tHooks.h"

#define TICKS_PER_SEC 	(1000 / TINYOS_SYSTICK_MS)

typedef enum _tError{
	tErrorNoError = 0,
	tErrorTimeout,
	tErrorResourceUnavaliable,
	tErrorDel,
	tErrorResourceFull,
	tErrorOwner,
}tError;


extern tTask * currentTask;
extern tTask * nextTask;


uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

void tTaskRunFirst (void);
void tTaskSwitch (void);

void tTaskSchedInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);
void tTaskSched(void);

void tTaskSchedRdy(tTask * task);
void tTaskSchedUnRdy(tTask * task);

void tTimeTaskWait(tTask * task, uint32_t ticks);
void tTimeTaskWakeUp(tTask * task);
void tTimeTaskRemove (tTask * task);
void tTaskSchedRemove(tTask * task);
void tTaskSystemHandler(void);
void tTaskDelay(uint32_t delay);
void tSetSysTickPeriod(uint32_t ms);

float tCpuUsageGet(void);
#endif


