#include "tinyOs.h"

#if TINYOS_ENABLE_MEMBLOCK == 1
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt)//存储块初始化
{
	uint8_t * memBlockStart = (uint8_t *)memStart;							  		//存储块开头（为啥要转换为8位？）
	uint8_t * memBlockEnd = memBlockStart + blockSize * blockCnt;			//存储块结尾
	
	if(blockSize < sizeof(tNode))		//如果存储块小于链表节点大小
	{
		return;
	}
	
	tEventInit(&memBlock->event, tEventTypeMemBlock);//初始化等待存储块的任务列表
	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockCnt;
	
	tListInit(&memBlock->blockList);							//初始化空闲存储块链表
	while(memBlockStart < memBlockEnd)						//将存储块入链
	{
		/*
		？？？why为什么可以将int指针强制转换为一个结构体的指针
		好吧，其实是可以的，不同的数据类型占有的空间大小不一，但是他们都必须有个地址，
		而这个地址就是硬件访问的依据，而名字只是提供给程序员的一种记住这个地址的方便一点的方法。
		所以转成什么样都可以，这里转换完，就相当于定义了一个tNode
		*/
		tNodeInit((tNode *)memBlockStart);					
		tListAddLast(&memBlock->blockList, (tNode *)memBlockStart);
		
		memBlockStart += blockSize;
	}
}

uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	if(tListCount(&memBlock->blockList) > 0)//如果有存储块在等待
	{
		*mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);//直接取第一个
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else//没有，则进入事件等待，并切换任务，
	{
		tEventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);
		tTaskExitCritical(status);
		
		tTaskSched();
		
		*mem = currentTask->eventMsg;//唤醒后，获取保存到事件中的存储块
		
		return currentTask->waitEventResult;
	}
}

uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, void ** mem)//不等待获取
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

void tMemBlockNotify(tMemBlock * memBlock, uint8_t * mem)//存储块到达
{
	uint32_t status = tTaskEnterCritical();
	
	if(tEventWaitCount(&memBlock->event) > 0)//如果有事件在等待
	{
		tTask * task = tEventWakeUp(&memBlock->event,(void *)mem, tErrorNoError);//将内存块保存到事件中
		
		
		if(task->prio < currentTask->prio)
		{
			tTaskSched();
		}
		
	}
	else
	{
		tListAddLast(&memBlock->blockList, (tNode *)mem);//将存储块插入链表
	}
	tTaskExitCritical(status);
}

uint32_t tMemBlockDestroy(tMemBlock * memBlock)//存储块的删除
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

void tMemBlockGetInfo(tMemBlock * memBlock, tMemBlockInfo * info)//获取信息
{
	uint32_t status = tTaskEnterCritical();
	
	info->count = tListCount(&memBlock->blockList);
	info->maxCount = memBlock->maxCount;
	info->blockSize = memBlock->blockSize;
	info->taskCount = tEventWaitCount(&memBlock->event);
	
	tTaskExitCritical(status);
}

#endif
