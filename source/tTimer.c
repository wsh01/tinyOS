#include "tinyOS.h"

#if TINYOS_ENABLE_TIMER == 1
static tList tTimerHardList;
static tList tTimerSoftList;
static tSem tTimerProtectSem;//该计数信号量保证软延时链表刷新的原子性
static tSem tTimerTickSem;//该计数信号量保证软延时链表每次刷新都要等待硬延时链表先刷新，硬延时链表在systick中断中刷新
													//硬延时刷新后才释放信号量，所以软延时时间不可预测，才叫软延时
void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
									void (*timerFunc) (void * arg), void * arg, uint32_t config)
{
	tNodeInit(&timer->linkNode);
	timer->startDelayTicks = delayTicks;
	timer->durationTicks = durationTicks;
	timer->timerFunc = timerFunc;
	timer->arg = arg;
	timer->config = config;
	
	if(delayTicks == 0)//如果初始启动延时为0，则使用周期值
	{
		timer->delayTicks = durationTicks;
	}
	else
	{
		timer->delayTicks = timer->startDelayTicks;
	}
	
	timer->state = tTimerCreated;
}

static tTask tTimeTask;
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE];

static void tTimerCallFuncList(tList * timerList)//遍历更新延时链表，如果延时到达，结构体调用回调函数
{
	tNode * node;
	for(node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)//遍历链表
	{
		tTimer * timer = tNodeParent(node, tTimer, linkNode);//得到延时结构
		if((timer->delayTicks == 0) || (--timer->delayTicks == 0))//更新计数并判断时间是否到达
		{
			timer->state = tTimerRunning;//更新状态为运行
			timer->timerFunc(timer->arg);//调用回调函数
			timer->state = tTimerStarted;//更新状态为已开始
			
			if(timer->durationTicks > 0)//如果有周期定时
			{
				timer->delayTicks = timer->durationTicks;//则周期性定时
			}
			else
			{
				tListRemove(timerList, &timer->linkNode);//否则移除结构体
				timer->state = tTimerStopped;//更新状态为已停止
			}
		}
	}
}

static void tTimerSoftTask(void * param)//延时任务入口函数
{
	for(;;)
	{
		tSemWait(&tTimerTickSem, 0);//获取计数信号量，因为其初始化为0，这里直接就进入等待了
		tSemWait(&tTimerProtectSem, 0);
		tTimerCallFuncList(&tTimerSoftList);//刷新软延时链表
		tSemNotify(&tTimerProtectSem);
	}
}

void tTimerModuleTickNotify(void)//刷新硬延时链表，该函数在systick中断函数中调用
{
	uint32_t status = tTaskEnterCritical();
	
	tTimerCallFuncList(&tTimerHardList);
	
	tTaskExitCritical(status);
	
	tSemNotify(&tTimerTickSem);//释放信号量，同步systick信号给延时任务，刷新软延时链表
}

void tTimerModuleInit(void)	//初始化延时任务
{
	tListInit(&tTimerHardList);
	tListInit(&tTimerSoftList);//初始化链表
	tSemInit(&tTimerProtectSem, 1, 1);
	tSemInit(&tTimerTickSem, 0, 0);//初始化计数信号量
}

void tTimerInitTask(void)
{
	#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRO_COUNT - 1)
	#error "The proprity of timer tasker must be greater then (TINYOS_PRO_COUNT - 1)"
	#endif
	tTaskInit(&tTimeTask, tTimerSoftTask, (void *)0, TINYOS_TIMERTASK_PRIO, tTimerTaskStack, TINYOS_TIMERTASK_STACK_SIZE);
}


void tTimerStart(tTimer * timer)//定时器启动
{
	switch(timer->state)//检查定时器状态
	{
		case tTimerCreated:
		case tTimerStopped://刚创建好或已经停止
			timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
			timer->state = tTimerStarted;
			
			if(timer->config & TIMER_CONFIG_TYPE_HARD)//根据定时器类型加入相应的定时器列表
			{
				uint32_t status = tTaskEnterCritical();//硬定时器，在时钟节拍中断中处理，所以使用critical来防护
				
				tListAddLast(&tTimerHardList, &timer->linkNode);
				tTaskExitCritical(status);
			}
			else
			{
				//软定时器，先获取信号量。以处理此时定时器任务，同时处理在访问软定时器列表导致的冲突问题
				tSemWait(&tTimerProtectSem, 0);
				tListAddLast(&tTimerSoftList, &timer->linkNode);
				tSemNotify(&tTimerProtectSem);
			}
			break;
		default:
			break;
	}
}

void tTimerStop(tTimer * timer)//定时器停止
{
	switch(timer->state)
	{
		case tTimerStarted:
		case tTimerRunning://以经开始或正在运行回调函数
			if(timer->config & TIMER_CONFIG_TYPE_HARD)//如果已经启动，判断定时器类型，然后从相应的延时列表中移除
			{
				uint32_t status = tTaskEnterCritical();
				
				tListRemove(&tTimerHardList, &timer->linkNode);//硬定时器
				
				tTaskExitCritical(status);
			}
			else
			{
				tSemWait(&tTimerProtectSem, 0);
				tListRemove(&tTimerSoftList, &timer->linkNode);//软定时器
				tSemNotify(&tTimerProtectSem);
			}
			timer->state = tTimerStopped;
			break;
		default:
			break;
	}
	
}

void tTimerDestroy(tTimer * timer)
{
	tTimerStop(timer);
}

void tTimerGetInfo (tTimer * timer, tTimerInfo * info)
{
	uint32_t status = tTaskEnterCritical();
	
	info->startDelayTicks = timer->startDelayTicks;
	info->durationTicks = timer->durationTicks;
	info->timerFunc = timer->timerFunc;
	info->config = timer->config;
	info->state = timer->state;
	
	tTaskExitCritical(status);
}

#endif
