#include "tinyOS.h"
#if TINYOS_ENABLE_MBOX == 1
void tMboxInit(tMbox * mbox, void **msgBuffer, uint32_t maxCount)
{
	tEventInit(&mbox->event, tEventTypeMbox);
	
	mbox->msgBuffer = msgBuffer;
	mbox->maxCount = maxCount;
	mbox->read = 0;
	mbox->write = 0;
	mbox->count = 0;
	
}

uint32_t tMboxWait(tMbox * mbox, void **msg, uint32_t waitTicks)//等待获取消息
{
	uint32_t status = tTaskEnterCritical();
	
	if(mbox->count > 0)//如果有消息，根据读索引读取，然后更新读索引
	{
		--mbox->count;
		*msg = mbox->msgBuffer[mbox->read++];
		
		if(mbox->read >= mbox->maxCount)
		{
			mbox->read = 0;
		}
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else//没消息，进入事件等待队列，切换任务
	{
		tEventWait(&mbox->event, currentTask, (void *)0, tEventTypeSem, waitTicks);
		tTaskExitCritical(status);
		
		tTaskSched();
		
		*msg = currentTask->eventMsg;//重新回到该任务，读取notify保存到任务中的消息
		
		return currentTask->waitEventResult;
	}
}

uint32_t tMboxNoWaitGet(tMbox * mbox, void **msg)
{
	uint32_t status = tTaskEnterCritical();
	
	if(mbox->count > 0)
	{
		--mbox->count;
		*msg = mbox->msgBuffer[mbox->read++];
		if(mbox->read >= mbox->maxCount)
		{
			mbox->read = 0;
		}
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else//没有消息就直接退出，不进入等待
	{
		tTaskExitCritical(status);
		return tErrorResourceUnavaliable;
	}
}


uint32_t tMboxNotify(tMbox * mbox, void * msg, uint32_t notifyOption)//写入消息
{
	uint32_t status = tTaskEnterCritical();
	
	if(tEventWaitCount(&mbox->event) > 0)//有任务在等待消息，
	{
		tTask * task = tEventWakeUp(&mbox->event, (void *)msg, tErrorNoError);//唤醒等待任务，并将消息保存到任务里
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	else//没有任务在等待消息
	{
		if(mbox->count >= mbox->maxCount)//如果没有足够空间
		{
			tTaskExitCritical(status);
			return tErrorResourceFull;
		}
		if(notifyOption & TMBOXSendFront)//如果写入方式为前向写入
		{
			if(mbox->read <= 0)
			{
				mbox->read = mbox->maxCount - 1;
			}
			else
			{
				--mbox->read;
			}
			mbox->msgBuffer[mbox->read] = msg;
		}
		else//正常写入
		{
			mbox->msgBuffer[mbox->write++] = msg;
			if(mbox->write >= mbox->maxCount)
			{
				mbox->write = 0;
			}
		}
		mbox->count++;
	}
	
	tTaskExitCritical(status);
	return tErrorNoError;
}

void tMboxFlush (tMbox * mbox)//清空消息
{
	uint32_t status = tTaskEnterCritical();
	
	if(tEventWaitCount(&mbox->event) == 0)
	{
		mbox->read = 0;
		mbox->write = 0;
		mbox->count = 0;
	}
	tTaskExitCritical(status);
}

uint32_t tMboxDestroy(tMbox * mbox)//清空等待事件
{
	uint32_t status = tTaskEnterCritical ();
	
	uint32_t count = tEventRemoveAll(&mbox->event , (void *)0, tErrorDel);
	
	tTaskExitCritical (status);
	
	if(count > 0)
	{
		tTaskSched();
	}
	return count; 
}
void tMboxGetInfo(tMbox * mbox, tMboxInfo * info)
{
	uint32_t status = tTaskEnterCritical();
	
	info->count = mbox->count;
	info->maxCount = mbox->maxCount;
	info->taskCount = tEventWaitCount(&mbox->event);
	
	tTaskExitCritical(status);
	
}

#endif
