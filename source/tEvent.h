#ifndef TEVENT_H
#define TEVENT_H

#include "tTask.h"
#include "tLib.h"

typedef enum _tEventType{
	tEventTypeUnknow,
	tEventTypeSem,
	tEventTypeMbox,
	tEventTypeMemBlock,
	tEventTypeFlagGroup,
	tEventTypeMutex,
}tEventType;

typedef struct _tEvent{
	tEventType type;
	tList waitList;
}tEvent;

void tEventInit (tEvent * event, tEventType type);
void tEventWait (tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout);
tTask * tEventWakeUp (tEvent * event, void * msg, uint32_t result);
void tEventRemoveTask (tTask * task, void * msg, uint32_t result);
uint32_t tEventRemoveAll(tEvent * event, void * msg, uint32_t result);
uint32_t tEventWaitCount(tEvent * event);
tTask * tEventWakeUpTask (tEvent * event, tTask * task, void * msg, uint32_t result);
#endif

