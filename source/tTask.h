#ifndef TTASK_H
#define TTASK_H

#define TINYOS_TASK_STATE_RDY					0
#define TINYOS_TASK_STATE_DESTORYED   (1 << 1)
#define TINYOS_TASK_STATE_DELAYED			(1 << 2)//用这种方式，只需要一个位来保存当前状态，可以留其他位扩展其他状态
#define TINYOS_TASK_STATE_SUSPEND			(1 << 3)
#define TINYOS_TASK_WAIT_MASK         (0xFF << 16)//事件等待状态

#include "tLib.h"

struct _tEvent;

typedef uint32_t tTaskStack;

typedef struct _tTask{
	tTaskStack * stack; 
	uint32_t * stackBase;
	uint32_t stackSize;
	tNode linkNode;			 	//同优先级任务链表节点
	uint32_t delayTicks; 	//任务软定时计数
	tNode delayNode;		 	//延时链表节点
	uint32_t prio;				//优先级
	uint32_t state;			 	//就绪状态
	uint32_t slice;				//占用时间片长度
	uint32_t suspendCount;//挂起计数 
	
	void (*clean) (void * param); //清理函数指针
	void * cleanParam;
	uint8_t requestDeleteFlag;		//请求删除标志
	
	struct _tEvent * waitEvent;		//事件
	void * eventMsg;							//事件消息
	uint32_t waitEventResult;
	
	uint32_t waitFlagsType;				//等待标志
	uint32_t eventFlags;					//事件标志
}tTask;


typedef struct _tTaskInfo{
	uint32_t delayTicks;
	uint32_t prio;
	uint32_t state;
	uint32_t slice;
	uint32_t suspendCount;
	
	uint32_t stackSize;
	uint32_t stackFree;
}tTaskInfo;
void tTaskGetInfo(tTask * task, tTaskInfo * info);
void tTaskInit(tTask * task, void (*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, uint32_t size);
void tTaskSuspend (tTask * task);
void tTaskWakeUp(tTask * task);
void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param);
void tTaskForceDelete(tTask * task);
void tTaskRequestDelete(tTask * task);
uint8_t tTaskIsRequestedDeleted(void);
void tInitApp(void);
void tTaskDeleteSelf (void);
#endif

