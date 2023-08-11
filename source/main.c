#include "tinyOS.h"
#include "ARMCM3.h"
#include "tConfig.h"
tTask * currentTask;
tTask * nextTask;
tTask * idleTask;
tList taskTable[TINYOS_PRO_COUNT];
tBitmap taskPrioBitmap;

uint8_t schedLockCount;

uint32_t tickCount;
tList tTaskDelayedList;

uint32_t idleCount;
uint32_t idleMaxCount;

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1	
static void initCpuUsageState(void);
static void checkCpuUsage(void);
static void cpuUsageSyncWithSysTick(void);
#endif

tTask * tTaskHighestReady(void)//查找就绪最高优先级任务
{
	uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
	tNode * node = tListFirst(&taskTable[highestPrio]);
	return tNodeParent(node, tTask, linkNode);
	
}
void tTaskSchedInit(void)//调度锁初始化
{
	int i;
	schedLockCount = 0;
	tBitmapInit(&taskPrioBitmap);
	for(i = 0; i < TINYOS_PRO_COUNT; i++)
	{
		tListInit(&taskTable[i]);
	}
}

void tTaskSchedDisable(void)//上调度锁
{
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount < 255)
	{
		schedLockCount++;//每调用一次自增一，相当于多重锁
	}
	tTaskExitCritical(status);
}

void tTaskSchedEnable(void)//解锁
{
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount > 0)
	{
		if(--schedLockCount == 0)//每解一次锁就进调度函数判断一次
		{
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}

void tTaskSchedRdy(tTask * task)					//恢复当前任务到状态
{
	tListAddFirst(&(taskTable[task->prio]),&(task->linkNode));
	tBitmapSet(&taskPrioBitmap, task->prio);
}

void tTaskSchedUnRdy(tTask * task)				//将任务从就绪状态中删除
{
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

void tTaskSchedRemove(tTask * task)				//任务移除
{
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

void tTaskSched(void)//调度算法
{
	tTask * tempTask;
	uint32_t status = tTaskEnterCritical();//进入临界区保护
	if(schedLockCount > 0)//调度锁判断
	{
		tTaskExitCritical(status);
		return;
	}
	
	
	tempTask = tTaskHighestReady();
	if(tempTask != currentTask)
	{
		nextTask = tempTask;
		
#if TINYOS_ENABLE_HOOKS == 1
		tHooksTaskSwitch(currentTask, nextTask);
#endif
		
		tTaskSwitch();
	}
	
	tTaskExitCritical(status);	//退出临界区保护
}


void tTaskDelayedInit(void)
{
	tListInit(&tTaskDelayedList);
}

void tTimeTaskWait(tTask * task, uint32_t ticks)		//任务进入延时链表
{
	task->delayTicks = ticks;
	tListAddLast(&tTaskDelayedList, &(task->delayNode));
	task->state |= TINYOS_TASK_STATE_DELAYED;					//用这种方式，只需要一个位来保存当前状态，可以留其他位扩展其他状态
}

void tTimeTaskWakeUp(tTask * task)									//将任务从延时链表去除
{
	tListRemove(&tTaskDelayedList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAYED;				//用这种方式，只需要一个位来保存当前状态，可以留其他位扩展其他状态
}

void tTimeTaskRemove (tTask * task)									//将任务从延时链表删除
{
	tListRemove(&tTaskDelayedList, &(task->delayNode));
}

void tTimeTicksInit(void)
{
	tickCount = 0;
}

void tTaskSystemHandler(void)													//systick中断处理
{
	tNode * node;
	uint32_t status = tTaskEnterCritical();//进入临界区保护

	for(node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode);node = node->nextNode)
	{
		tTask * task = tNodeParent(node, tTask, delayNode);
		if(--task->delayTicks == 0)
		{
			if(task->waitEvent)				//延时事件
			{
				tEventRemoveTask(task, (void *)0, tErrorTimeout);
			}
			tTimeTaskWakeUp(task);
			
			tTaskSchedRdy(task);
		}
	}
	
	if(--currentTask->slice == 0)											//同优先级时间片切换
	{
		if(tListCount(&taskTable[currentTask->prio]) > 0)
		{
			tListRemoveFirst(&taskTable[currentTask->prio]);
			tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));
			
			currentTask->slice = TINYOS_SLICE_MAX;
		}
	}
	tickCount++;
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1	
	checkCpuUsage();
#endif
	
	tTaskExitCritical(status);	//退出临界区保护
	
#if TINYOS_ENABLE_TIMER == 1	
	tTimerModuleTickNotify();//刷新软定时器延时列表
#endif	

#if TINYOS_ENABLE_HOOKS == 1
	tHooksSysTicks();
#endif	
	tTaskSched();
}

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1	

static float cpuUsage;
static uint32_t enableCpuUsageState;

static void initCpuUsageState(void)
{
	idleCount = 0;
	idleMaxCount = 0;
	cpuUsage = 0.0f;
	enableCpuUsageState = 0;
}

static void checkCpuUsage(void)
{
	if(enableCpuUsageState == 0)
	{
		enableCpuUsageState = 1;
		tickCount = 0;
		return;
	}
	if(tickCount == TICKS_PER_SEC)
	{
		idleMaxCount = idleCount;
		idleCount = 0;
		
		tTaskSchedEnable();
	}
	else if(tickCount % TICKS_PER_SEC == 0)
	{
		cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
		idleCount = 0;
	}
}

static void cpuUsageSyncWithSysTick(void)
{
	while(enableCpuUsageState == 0)
	{
		;;
	}
}

float tCpuUsageGet(void)
{
	float usage = 0;
	uint32_t status = tTaskEnterCritical();
	usage = cpuUsage;
	tTaskExitCritical(status);
	
	return usage;
}
#endif

tTask tTaskIdle;
tTaskStack idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE];

void idleTaskEntry(void * param)		//空闲任务
{
	tTaskSchedDisable();
	
	tInitApp();
	
#if TINYOS_ENABLE_TIMER == 1	
	tTimerInitTask();
#endif
	
	tSetSysTickPeriod(TINYOS_SYSTICK_MS);
	
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1	
	cpuUsageSyncWithSysTick();
#endif
	
 	for(;;)
	{
		uint32_t status = tTaskEnterCritical();
		idleCount++;
		tTaskExitCritical(status);
#if TINYOS_ENABLE_HOOKS == 1
		tHooksCpuIdle();
#endif
	}
}

int main ()
{
	tTaskSchedInit();//调度锁初始化
	
	tTaskDelayedInit();
	
#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleInit();
#endif
	
	tTimeTicksInit();	

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1		
	initCpuUsageState();
#endif
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0,TINYOS_PRO_COUNT-1, idleTaskEnv, TINYOS_IDLETASK_STACK_SIZE);
		 
	nextTask = tTaskHighestReady();
	
	tTaskRunFirst();
	
	return 0;
}
	
