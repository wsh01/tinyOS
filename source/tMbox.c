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

uint32_t tMboxWait(tMbox * mbox, void **msg, uint32_t waitTicks)//�ȴ���ȡ��Ϣ
{
	uint32_t status = tTaskEnterCritical();
	
	if(mbox->count > 0)//�������Ϣ�����ݶ�������ȡ��Ȼ����¶�����
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
	else//û��Ϣ�������¼��ȴ����У��л�����
	{
		tEventWait(&mbox->event, currentTask, (void *)0, tEventTypeSem, waitTicks);
		tTaskExitCritical(status);
		
		tTaskSched();
		
		*msg = currentTask->eventMsg;//���»ص������񣬶�ȡnotify���浽�����е���Ϣ
		
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
	else//û����Ϣ��ֱ���˳���������ȴ�
	{
		tTaskExitCritical(status);
		return tErrorResourceUnavaliable;
	}
}


uint32_t tMboxNotify(tMbox * mbox, void * msg, uint32_t notifyOption)//д����Ϣ
{
	uint32_t status = tTaskEnterCritical();
	
	if(tEventWaitCount(&mbox->event) > 0)//�������ڵȴ���Ϣ��
	{
		tTask * task = tEventWakeUp(&mbox->event, (void *)msg, tErrorNoError);//���ѵȴ����񣬲�����Ϣ���浽������
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	else//û�������ڵȴ���Ϣ
	{
		if(mbox->count >= mbox->maxCount)//���û���㹻�ռ�
		{
			tTaskExitCritical(status);
			return tErrorResourceFull;
		}
		if(notifyOption & TMBOXSendFront)//���д�뷽ʽΪǰ��д��
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
		else//����д��
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

void tMboxFlush (tMbox * mbox)//�����Ϣ
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

uint32_t tMboxDestroy(tMbox * mbox)//��յȴ��¼�
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
