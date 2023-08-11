#include "tinyOS.h"


//void*通用指针，赋给其他类型指针时需要强制类型转换，不可解引用
// void (*entry)(void *)为函数指针，函数返回值类型 (* 指针变量名) (函数参数列表);
void tTaskInit(tTask * task, void (*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, uint32_t size) 
{
	uint32_t * stackTop;
	
	task->stackBase = stack;//数据增长末端
	task->stackSize = size;
	memset(stack, 0, size);//将栈初始化为零
	
	stackTop = stack + size / sizeof(tTaskStack);//数据增长起始端
	
	*(--stackTop) = (unsigned long) (1 << 24);  	//XPSR,Thumb状态，第24位总是1，清除此位会引起错误异常
	*(--stackTop) = (unsigned long)entry;				//PC寄存器
	*(--stackTop) = (unsigned long)0x14;
	*(--stackTop) = (unsigned long)0x12;
	*(--stackTop) = (unsigned long)0x3;
	*(--stackTop) = (unsigned long)0x2;
	*(--stackTop) = (unsigned long)0x1;
	*(--stackTop) = (unsigned long)param;  			//R0
	//以上部分在退出PendSV异常时自动加载进寄存器组，需要特定顺序
	*(--stackTop) = (unsigned long)0x11;
	*(--stackTop) = (unsigned long)0x10;
	*(--stackTop) = (unsigned long)0x9;
	*(--stackTop) = (unsigned long)0x8;
	*(--stackTop) = (unsigned long)0x7;
	*(--stackTop) = (unsigned long)0x6;
	*(--stackTop) = (unsigned long)0x5;
	*(--stackTop) = (unsigned long)0x4;
	
	task->slice = TINYOS_SLICE_MAX;				//占用时间片长度
	task->stack = stackTop;
	task->delayTicks = 0;									//将延时数值初始化为零
	task->prio = prio;										//优先级
	task->state = TINYOS_TASK_STATE_RDY;	//就绪状态
	task->suspendCount = 0;
	task->clean = (void (*)(void *))0;		//函数指针
	task->cleanParam = (void *)0;					//清理参数
	task->requestDeleteFlag = 0;					//请求删除标志位
	
	
	tNodeInit(&(task->linkNode));					//初始化同优先级时间片链表节点
	tNodeInit(&(task->delayNode));				//初始化当前延时链表节点
	
	tTaskSchedRdy(task);
	
#if TINYOS_ENABLE_HOOKS == 1
	tHooksTaskInit(task);
#endif

}

void tTaskSuspend (tTask * task)			//挂起任务
{
	uint32_t status = tTaskEnterCritical();
	
	if(!(task->state & TINYOS_TASK_STATE_SUSPEND))
	{
		if(++task->suspendCount <= 1)
		{
			task->state |= TINYOS_TASK_STATE_SUSPEND;
			tTaskSchedUnRdy(task);
			if(task == currentTask)
			{
				tTaskSched();
			}
		}
	}
	
	tTaskExitCritical(status);
}

void tTaskWakeUp(tTask * task)			//唤醒任务
{
	uint32_t status = tTaskEnterCritical();
	
	if(task->state & TINYOS_TASK_STATE_SUSPEND)
	{
		if(--task->suspendCount == 0)
		{
			task->state &= ~TINYOS_TASK_STATE_SUSPEND;
			tTaskSchedRdy(task);
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}

void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param)//设置清理函数和参数
{
	task->clean = clean;
	task->cleanParam = param;
}

void tTaskForceDelete(tTask * task)//强制删除任务
{
	uint32_t status = tTaskEnterCritical();
	
	if(task->state & TINYOS_TASK_STATE_DELAYED)
	{
		tTimeTaskRemove(task);
	}
	else if(!(task->state & TINYOS_TASK_STATE_SUSPEND))
	{
		tTaskSchedRemove(task);
	}
	if(task->clean)
	{
		task->clean(task->cleanParam);//调用清理函数
	}
	if(currentTask == task)
	{
		tTaskSched();
	}
	
	tTaskExitCritical(status);
}

void tTaskRequestDelete(tTask * task)//请求删除
{
	uint32_t status = tTaskEnterCritical();
	
	task->requestDeleteFlag = 1;
	
	tTaskExitCritical(status);
}

uint8_t tTaskIsRequestedDeleted(void)//查询请求删除标志位状态
{
	uint8_t delete;
	
	uint32_t status = tTaskEnterCritical();
	
	delete = currentTask->requestDeleteFlag;
	
	tTaskExitCritical(status);
	
	return delete;
}

void tTaskDeleteSelf (void)//自我删除
{
	uint32_t status = tTaskEnterCritical();
	
	tTaskSchedRemove(currentTask);
	
	if(currentTask->clean)
	{
		currentTask->clean(currentTask->cleanParam);//调用清理函数
	}
	
	tTaskSched();
	
	tTaskExitCritical(status);
}

void tTaskGetInfo(tTask * task, tTaskInfo * info)
{
	uint32_t * stackEnd;
	uint32_t status = tTaskEnterCritical();
	
	info->delayTicks = task->delayTicks;
	info->prio = task->prio;
	info->state = task->state;
	info->slice = task->slice;
	info->suspendCount = task->suspendCount;
	info->stackSize = task->stackSize;
	
	info->stackFree = 0;
	stackEnd = task->stackBase;
	while ((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(tTaskStack)))//统计堆栈使用情况
	{
		info->stackFree++;//从数据增长末端向起始端扫描，直到第一个不为零的值，求出未使用空间
	}
	info->stackFree *= sizeof(tTaskStack);//转为位数
	tTaskExitCritical(status);
}
