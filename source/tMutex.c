#include "tinyOS.h"

#if TINYOS_ENABLE_MUTEX == 1
void tMutexInit(tMutex * mutex)
{
	tEventInit(&mutex->event, tEventTypeMutex);
	
	mutex->lockedCount = 0;
	mutex->owner = (tTask *)0;
	mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;//������ȼ�
}

uint32_t tMutexWait(tMutex * mutex, uint32_t waitTicks)//��ȡ�ź���
{
	uint32_t status = tTaskEnterCritical();
	
	if (mutex->lockedCount <= 0)//û�б�����
	{
		mutex->owner = currentTask;		//ӵ����Ϊ��ǰ����
		mutex->ownerOriginalPrio = currentTask->prio;		//��ǰ�������ȼ�
		mutex->lockedCount++;		//����������һ
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else//�Ѿ�������
	{
		if(mutex->owner == currentTask)//�ٴα���ǰ��������
		{
			mutex->lockedCount++;
			tTaskExitCritical(status);
			return tErrorNoError;
		}
		else	//�Ѿ���������������
		{
			if(currentTask->prio < mutex->owner->prio)//�����ǰ�������ȼ��ϸ�
			{
				tTask * owner = mutex->owner;//��ȡ�����ź���������
				if(owner->state == TINYOS_TASK_STATE_RDY)//����������ھ���״̬
				{
					tTaskSchedUnRdy(owner);//����Ӿ���״̬�Ƴ�
					owner->prio = currentTask->prio;//���ȼ��̳�
					tTaskSchedRdy(owner);//�ָ�
				}
				else
				{
					owner->prio = currentTask->prio;//���ȼ��̳�
				}
			}
			tEventWait(&mutex->event, currentTask, (void *)0, tEventTypeMutex, waitTicks);//��ǰ��������¼��ȴ�
			tTaskExitCritical(status);
			
			tTaskSched();
			return currentTask->waitEventResult;
		}
	}
}
uint32_t tMutexNoWaitGet(tMutex * mutex)//�޵ȴ���ȡ
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

uint32_t tMutexNotify(tMutex * mutex)//�����ź����ͷ�
{
	uint32_t status = tTaskEnterCritical();
	
	if(mutex->lockedCount <= 0)				//û������
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	if(mutex->owner != currentTask)		//���ǵ�ǰ��������
	{
		tTaskExitCritical(status);
		return tErrorOwner;
	}	
	if(--mutex->lockedCount > 0)			//������Ƕ��
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	if(mutex->ownerOriginalPrio != mutex->owner->prio)   	//����������ȼ��̳�
	{
		if(mutex->owner->state == TINYOS_TASK_STATE_RDY)		//���������������������ⲻֻ���ǵ�ǰ�����𣿣���
		{
			tTaskSchedUnRdy(mutex->owner);										//����Ӿ���״̬�Ƴ�
			currentTask->prio = mutex->ownerOriginalPrio;			//�ָ�ԭ���ȼ�
			tTaskSchedRdy(mutex->owner);											//����
		}
		else
		{
			currentTask->prio = mutex->ownerOriginalPrio;			//�ָ�ԭ���ȼ�
		}
	}
	if(tEventWaitCount(&mutex->event) > 0)								//����������ڵȴ��ź����ͷ�
	{
		tTask * task = tEventWakeUp(&mutex->event, (void *)0, tErrorNoError);//����һ��
		
		mutex->owner = task;
		mutex->ownerOriginalPrio = task->prio;							//��������
		mutex->lockedCount++;
		
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
	return tErrorNoError;
}

uint32_t tMutexDestroy(tMutex * mutex)//ɾ���ȴ�����
{
	uint32_t count = 0;
	uint32_t status = tTaskEnterCritical();
	
	if(mutex->lockedCount > 0)//��������������ź���
	{
		if(mutex->ownerOriginalPrio != mutex->owner->prio)//��������ȼ��̳�
		{
			if(mutex->owner->state == TINYOS_TASK_STATE_RDY)//��������ھ���״̬
			{
				tTaskSchedUnRdy(mutex->owner);
				currentTask->prio = mutex->ownerOriginalPrio;//�ָ�ԭ���ȼ�
				tTaskSchedRdy(mutex->owner);
			}
			else
			{
				currentTask->prio = mutex->ownerOriginalPrio;//�ָ�ԭ���ȼ�
			}
		}
		count = tEventRemoveAll(&mutex->event, (void *)0, tErrorDel);//ȥ�����еȴ�����
		
		if(count > 0)
		{
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
	return count;
}

void tMutexGetInfo(tMutex * mutex, tMutexInfo * info)//�����ź�����ѯ
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
