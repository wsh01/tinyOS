#ifndef TFLAGGROUP_H
#define TFLAGGROUP_H

#include "tEvent.h"

typedef struct _tFlagGroup
{
	tEvent event;
	uint32_t flag;
}tFlagGroup;

typedef struct _tFlagGroupInfo
{
	uint32_t flags;
	uint32_t taskCount;
}tFlagGroupInfo;

#define TFLAGGROUP_CLEAR  (0X0 << 0)
#define TFLAGGROUP_SET    (0X1 << 0)
#define TELAGGROUP_ANY 		(0X0 << 1)
#define TFLAGGROUP_ALL		(0X1 << 1)

#define TFLAGGROUP_SET_ALL			(TFLAGGROUP_SET | TFLAGGROUP_ALL)
#define TFLAGGROUP_SET_ANY			(TFLAGGROUP_SET | TFLAGGROUP_ANY)
#define TFLAGGROUP_CLEAR_ALL		(TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)
#define TFLAGGROUP_CLEAR_ANY		(TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)

#define TFLAGGROUP_CONSUME			(1 << 7)
void tFlagGroupInit(tFlagGroup * flagGroup, uint32_t flags);
uint32_t tFlagGroupWait(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag, uint32_t waitTicks);
uint32_t tFlagGroupNoWaitGet(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag);
void tFlagGroupNotify(tFlagGroup * tFlagGroup, uint8_t isSet, uint32_t flags);
uint32_t tFlagGroupDestroy(tFlagGroup * flagGroup);
void tFlagGroupGetInfo(tFlagGroup * flagGroup, tFlagGroupInfo * info);

#endif


