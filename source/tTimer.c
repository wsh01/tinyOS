#include "tinyOS.h"

#if TINYOS_ENABLE_TIMER == 1
static tList tTimerHardList;
static tList tTimerSoftList;
static tSem tTimerProtectSem;//�ü����ź�����֤����ʱ����ˢ�µ�ԭ����
static tSem tTimerTickSem;//�ü����ź�����֤����ʱ����ÿ��ˢ�¶�Ҫ�ȴ�Ӳ��ʱ������ˢ�£�Ӳ��ʱ������systick�ж���ˢ��
													//Ӳ��ʱˢ�º���ͷ��ź�������������ʱʱ�䲻��Ԥ�⣬�Ž�����ʱ
void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
									void (*timerFunc) (void * arg), void * arg, uint32_t config)
{
	tNodeInit(&timer->linkNode);
	timer->startDelayTicks = delayTicks;
	timer->durationTicks = durationTicks;
	timer->timerFunc = timerFunc;
	timer->arg = arg;
	timer->config = config;
	
	if(delayTicks == 0)//�����ʼ������ʱΪ0����ʹ������ֵ
	{
		timer->delayTicks = durationTicks;
	}
	else
	{
		timer->delayTicks = timer->startDelayTicks;
	}
	
	timer->state = tTimerCreated;
}

static tTask tTimeTask;
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE];

static void tTimerCallFuncList(tList * timerList)//����������ʱ���������ʱ����ṹ����ûص�����
{
	tNode * node;
	for(node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)//��������
	{
		tTimer * timer = tNodeParent(node, tTimer, linkNode);//�õ���ʱ�ṹ
		if((timer->delayTicks == 0) || (--timer->delayTicks == 0))//���¼������ж�ʱ���Ƿ񵽴�
		{
			timer->state = tTimerRunning;//����״̬Ϊ����
			timer->timerFunc(timer->arg);//���ûص�����
			timer->state = tTimerStarted;//����״̬Ϊ�ѿ�ʼ
			
			if(timer->durationTicks > 0)//��������ڶ�ʱ
			{
				timer->delayTicks = timer->durationTicks;//�������Զ�ʱ
			}
			else
			{
				tListRemove(timerList, &timer->linkNode);//�����Ƴ��ṹ��
				timer->state = tTimerStopped;//����״̬Ϊ��ֹͣ
			}
		}
	}
}

static void tTimerSoftTask(void * param)//��ʱ������ں���
{
	for(;;)
	{
		tSemWait(&tTimerTickSem, 0);//��ȡ�����ź�������Ϊ���ʼ��Ϊ0������ֱ�Ӿͽ���ȴ���
		tSemWait(&tTimerProtectSem, 0);
		tTimerCallFuncList(&tTimerSoftList);//ˢ������ʱ����
		tSemNotify(&tTimerProtectSem);
	}
}

void tTimerModuleTickNotify(void)//ˢ��Ӳ��ʱ�����ú�����systick�жϺ����е���
{
	uint32_t status = tTaskEnterCritical();
	
	tTimerCallFuncList(&tTimerHardList);
	
	tTaskExitCritical(status);
	
	tSemNotify(&tTimerTickSem);//�ͷ��ź�����ͬ��systick�źŸ���ʱ����ˢ������ʱ����
}

void tTimerModuleInit(void)	//��ʼ����ʱ����
{
	tListInit(&tTimerHardList);
	tListInit(&tTimerSoftList);//��ʼ������
	tSemInit(&tTimerProtectSem, 1, 1);
	tSemInit(&tTimerTickSem, 0, 0);//��ʼ�������ź���
}

void tTimerInitTask(void)
{
	#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRO_COUNT - 1)
	#error "The proprity of timer tasker must be greater then (TINYOS_PRO_COUNT - 1)"
	#endif
	tTaskInit(&tTimeTask, tTimerSoftTask, (void *)0, TINYOS_TIMERTASK_PRIO, tTimerTaskStack, TINYOS_TIMERTASK_STACK_SIZE);
}


void tTimerStart(tTimer * timer)//��ʱ������
{
	switch(timer->state)//��鶨ʱ��״̬
	{
		case tTimerCreated:
		case tTimerStopped://�մ����û��Ѿ�ֹͣ
			timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
			timer->state = tTimerStarted;
			
			if(timer->config & TIMER_CONFIG_TYPE_HARD)//���ݶ�ʱ�����ͼ�����Ӧ�Ķ�ʱ���б�
			{
				uint32_t status = tTaskEnterCritical();//Ӳ��ʱ������ʱ�ӽ����ж��д�������ʹ��critical������
				
				tListAddLast(&tTimerHardList, &timer->linkNode);
				tTaskExitCritical(status);
			}
			else
			{
				//��ʱ�����Ȼ�ȡ�ź������Դ����ʱ��ʱ������ͬʱ�����ڷ�����ʱ���б��µĳ�ͻ����
				tSemWait(&tTimerProtectSem, 0);
				tListAddLast(&tTimerSoftList, &timer->linkNode);
				tSemNotify(&tTimerProtectSem);
			}
			break;
		default:
			break;
	}
}

void tTimerStop(tTimer * timer)//��ʱ��ֹͣ
{
	switch(timer->state)
	{
		case tTimerStarted:
		case tTimerRunning://�Ծ���ʼ���������лص�����
			if(timer->config & TIMER_CONFIG_TYPE_HARD)//����Ѿ��������ж϶�ʱ�����ͣ�Ȼ�����Ӧ����ʱ�б����Ƴ�
			{
				uint32_t status = tTaskEnterCritical();
				
				tListRemove(&tTimerHardList, &timer->linkNode);//Ӳ��ʱ��
				
				tTaskExitCritical(status);
			}
			else
			{
				tSemWait(&tTimerProtectSem, 0);
				tListRemove(&tTimerSoftList, &timer->linkNode);//��ʱ��
				tSemNotify(&tTimerProtectSem);
			}
			timer->state = tTimerStopped;
			break;
		default:
			break;
	}
	
}

void tTimerDestroy(tTimer * timer)
{
	tTimerStop(timer);
}

void tTimerGetInfo (tTimer * timer, tTimerInfo * info)
{
	uint32_t status = tTaskEnterCritical();
	
	info->startDelayTicks = timer->startDelayTicks;
	info->durationTicks = timer->durationTicks;
	info->timerFunc = timer->timerFunc;
	info->config = timer->config;
	info->state = timer->state;
	
	tTaskExitCritical(status);
}

#endif
