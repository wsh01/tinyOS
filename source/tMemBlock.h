#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "tEvent.h"

typedef struct _tMemBlock//���д洢������
{
	tEvent event;					//�ȴ��洢��������б�
	void * memStart;			//�洢�鿪ʼ��ַ
	uint32_t blockSize;		//�洢���С
	uint32_t maxCount;		//�洢������
	tList blockList;			//���д洢������
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


