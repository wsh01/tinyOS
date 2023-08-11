#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tEvent.h"

typedef struct _tMemBlock//空闲存储块链表
{
	tEvent event;					//等待存储块的任务列表
	void * memStart;			//存储块开始地址
	uint32_t blockSize;		//存储块大小
	uint32_t maxCount;		//存储块数量
	tList blockList;			//空闲存储块链表
}tMemBlock;
typedef struct _tMemBlockInfo
{
	uint32_t count;
	uint32_t maxCount;
	uint32_t blockSize;
	uint32_t taskCount;
}tMemBlockInfo;


void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);
uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, void ** mem);
void tMemBlockNotify(tMemBlock * memBlock, uint8_t * mem);
uint32_t tMemBlockDestroy(tMemBlock * memBlock);
void tMemBlockGetInfo(tMemBlock * memBlock, tMemBlockInfo * info);

#endif


