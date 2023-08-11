#include "tinyOS.h"

#if TINYOS_ENABLE_MUTEX == 1
void tMutexInit(tMutex * mutex)
{
	tEventInit(&mutex->event, tEventTypeMutex);
	
	mutex->lockedCount = 0;
	mutex->owner = (tTask *)0;
	mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;//最低优先级
}

uint32_t tMutexWait(tMutex * mutex, uint32_t waitTicks)//获取信号量
{
	uint32_t status = tTaskEnterCritical();
	
	if (mutex->lockedCount <= 0)//没有被锁定
	{
		mutex->owner = currentTask;		//拥有者为当前任务
		mutex->ownerOriginalPrio = currentTask->prio;		//当前任务优先级
		mutex->lockedCount++;		//锁定次数加一
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else//已经被锁定
	{
		if(mutex->owner == currentTask)//再次被当前任务锁定
		{
			mutex->lockedCount++;
			tTaskExitCritical(status);
			return tErrorNoError;
		}
		else	//已经被其他任务锁定
		{
			if(currentTask->prio < mutex->owner->prio)//如果当前任务优先级较高
			{
				tTask * owner = mutex->owner;//获取锁定信号量的任务
				if(owner->state == TINYOS_TASK_STATE_RDY)//如果该任务处于就绪状态
				{
					tTaskSchedUnRdy(owner);//将其从就绪状态移除
					owner->prio = currentTask->prio;//优先级继承
					tTaskSchedRdy(owner);//恢复
				}
				else
				{
					owner->prio = currentTask->prio;//优先级继承
				}
			}
			tEventWait(&mutex->event, currentTask, (void *)0, tEventTypeMutex, waitTicks);//当前任务加入事件等待
			tTaskExitCritical(status);
			
			tTaskSched();
			return currentTask->waitEventResult;
		}
	}
}
uint32_t tMutexNoWaitGet(tMutex * mutex)//无等待获取
{
	uint32_t status = tTaskEnterCritical();
	
	if(mutex->lockedCount <= 0)
	{
		mutex->owner = currentTask;
		mutex->ownerOriginalPrio = currentTask->prio;
		mutex->lockedCount++;
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		if(mutex->owner == currentTask)
		{
			mutex->lockedCount++;
			tTaskExitCritical(status);
			return tErrorNoError;
		}
		tTaskExitCritical(status);
		return tErrorResourceUnavaliable;
	}
}

uint32_t tMutexNotify(tMutex * mutex)//互斥信号量释放
{
	uint32_t status = tTaskEnterCritical();
	
	if(mutex->lockedCount <= 0)				//没有锁定
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	if(mutex->owner != currentTask)		//不是当前任务锁定
	{
		tTaskExitCritical(status);
		return tErrorOwner;
	}	
	if(--mutex->lockedCount > 0)			//锁定已嵌套
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	if(mutex->ownerOriginalPrio != mutex->owner->prio)   	//如果发生优先级继承
	{
		if(mutex->owner->state == TINYOS_TASK_STATE_RDY)		//如果任务就绪，？？？，这不只能是当前任务吗？？？
		{
			tTaskSchedUnRdy(mutex->owner);										//将其从就绪状态移除
			currentTask->prio = mutex->ownerOriginalPrio;			//恢复原优先级
			tTaskSchedRdy(mutex->owner);											//就绪
		}
		else
		{
			currentTask->prio = mutex->ownerOriginalPrio;			//恢复原优先级
		}
	}
	if(tEventWaitCount(&mutex->event) > 0)								//如果有任务在等待信号量释放
	{
		tTask * task = tEventWakeUp(&mutex->event, (void *)0, tErrorNoError);//唤醒一个
		
		mutex->owner = task;
		mutex->ownerOriginalPrio = task->prio;							//更新数据
		mutex->lockedCount++;
		
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
	return tErrorNoError;
}

uint32_t tMutexDestroy(tMutex * mutex)//删除等待任务
{
	uint32_t count = 0;
	uint32_t status = tTaskEnterCritical();
	
	if(mutex->lockedCount > 0)//如果有任务锁定信号量
	{
		if(mutex->ownerOriginalPrio != mutex->owner->prio)//如果有优先级继承
		{
			if(mutex->owner->state == TINYOS_TASK_STATE_RDY)//如果任务在就绪状态
			{
				tTaskSchedUnRdy(mutex->owner);
				currentTask->prio = mutex->ownerOriginalPrio;//恢复原优先级
				tTaskSchedRdy(mutex->owner);
			}
			else
			{
				currentTask->prio = mutex->ownerOriginalPrio;//恢复原优先级
			}
		}
		count = tEventRemoveAll(&mutex->event, (void *)0, tErrorDel);//去除所有等待任务
		
		if(count > 0)
		{
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
	return count;
}

void tMutexGetInfo(tMutex * mutex, tMutexInfo * info)//互斥信号量查询
{
	uint32_t status = tTaskEnterCritical();
	
	info->taskCount = tEventWaitCount(&mutex->event);
	info->ownerPrio = mutex->ownerOriginalPrio;
	info->inHeritedPrio = mutex->owner->prio;
	info->owner = mutex->owner;
	info->lockedCount = mutex->lockedCount;
	
	tTaskExitCritical(status);
}

#endif
