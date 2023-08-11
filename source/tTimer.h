#ifndef TTIMER_H
#define TTIMER_H

#include "tEvent.h"
typedef enum _tTimerState
{
	tTimerCreated,
	tTimerStarted,
	tTimerRunning,
	tTimerStopped,
	tTimerDestroyed,
}tTimerState;

typedef struct _tTimer
{
	tNode linkNode;
	uint32_t startDelayTicks;		//���������Ӻ��ticks��
	uint32_t durationTicks;			//���ڶ�ʱ������tick��
	uint32_t delayTicks;				//��ǰ��ʱ�ݼ�����ֵ
	void (*timerFunc) (void * arg);		//��ʱ�ص�����
	void * arg;									//���ݸ��ص������Ĳ���
	uint32_t config;						//��ʱ�����ò���
	tTimerState state;					//��ʱ��״̬
}tTimer;

typedef struct _tTImerInfo
{
	uint32_t startDelayTicks;
	uint32_t durationTicks;
	
	void (*timerFunc) (void * arg);
	
	void * arg;
	uint32_t config;
	
	tTimerState state;
}tTimerInfo;

#define TIMER_CONFIG_TYPE_HARD 		(1 << 0)
#define TIMER_CONFIG_TYPE_SOFT		(0 << 0)

void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
									void (*timerFunc) (void * arg), void * arg, uint32_t config);
void tTimerModuleInit(void);
void tTimerModuleTickNotify(void);
void tTimerStart(tTimer * timer);
void tTimerStop(tTimer * timer);
void tTimerDestroy(tTimer * timer);
void tTimerGetInfo (tTimer * timer, tTimerInfo * info);
void tTimerInitTask(void);
#endif


