#include "tinyOs.h"

#if TINYOS_ENABLE_MEMBLOCK == 1
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt)//�洢���ʼ��
{
	uint8_t * memBlockStart = (uint8_t *)memStart;							  		//�洢�鿪ͷ��ΪɶҪת��Ϊ8λ����
	uint8_t * memBlockEnd = memBlockStart + blockSize * blockCnt;			//�洢���β
	
	if(blockSize < sizeof(tNode))		//����洢��С������ڵ��С
	{
		return;
	}
	
	tEventInit(&memBlock->event, tEventTypeMemBlock);//��ʼ���ȴ��洢��������б�
	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockCnt;
	
	tListInit(&memBlock->blockList);							//��ʼ�����д洢������
	while(memBlockStart < memBlockEnd)						//���洢������
	{
		/*
		������whyΪʲô���Խ�intָ��ǿ��ת��Ϊһ���ṹ���ָ��
		�ðɣ���ʵ�ǿ��Եģ���ͬ����������ռ�еĿռ��С��һ���������Ƕ������и���ַ��
		�������ַ����Ӳ�����ʵ����ݣ�������ֻ���ṩ������Ա��һ�ּ�ס�����ַ�ķ���һ��ķ�����
		����ת��ʲô�������ԣ�����ת���꣬���൱�ڶ�����һ��tNode
		*/
		tNodeInit((tNode *)memBlockStart);					
		tListAddLast(&memBlock->blockList, (tNode *)memBlockStart);
		
		memBlockStart += blockSize;
	}
}

uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	if(tListCount(&memBlock->blockList) > 0)//����д洢���ڵȴ�
	{
		*mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);//ֱ��ȡ��һ��
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else//û�У�������¼��ȴ������л�����
	{
		tEventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);
		tTaskExitCritical(status);
		
		tTaskSched();
		
		*mem = currentTask->eventMsg;//���Ѻ󣬻�ȡ���浽�¼��еĴ洢��
		
		return currentTask->waitEventResult;
	}
}

uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, void ** mem)//���ȴ���ȡ
{
	uint32_t status = tTaskEnterCritical();
	
	if(tListCount(&memBlock->blockList) > 0)
	{
		*mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		tTaskExitCritical(status);
		return tErrorResourceUnavaliable;
	}
}

void tMemBlockNotify(tMemBlock * memBlock, uint8_t * mem)//�洢�鵽��
{
	uint32_t status = tTaskEnterCritical();
	
	if(tEventWaitCount(&memBlock->event) > 0)//������¼��ڵȴ�
	{
		tTask * task = tEventWakeUp(&memBlock->event,(void *)mem, tErrorNoError);//���ڴ�鱣�浽�¼���
		
		
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
		
	}
	else
	{
		tListAddLast(&memBlock->blockList, (tNode *)mem);//���洢���������
	}
	tTaskExitCritical(status);
}

uint32_t tMemBlockDestroy(tMemBlock * memBlock)//�洢���ɾ��
{
	uint32_t status = tTaskEnterCritical();
	
	uint32_t count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);
	tTaskExitCritical(status);
	
	if(count > 0)
	{
		tTaskSched();
	}
	return count;
}

void tMemBlockGetInfo(tMemBlock * memBlock, tMemBlockInfo * info)//��ȡ��Ϣ
{
	uint32_t status = tTaskEnterCritical();
	
	info->count = tListCount(&memBlock->blockList);
	info->maxCount = memBlock->maxCount;
	info->blockSize = memBlock->blockSize;
	info->taskCount = tEventWaitCount(&memBlock->event);
	
	tTaskExitCritical(status);
}

#endif
