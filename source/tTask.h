#ifndef TTASK_H
#define TTASK_H

#define TINYOS_TASK_STATE_RDY					0
#define TINYOS_TASK_STATE_DESTORYED   (1 << 1)
#define TINYOS_TASK_STATE_DELAYED			(1 << 2)//�����ַ�ʽ��ֻ��Ҫһ��λ�����浱ǰ״̬������������λ��չ����״̬
#define TINYOS_TASK_STATE_SUSPEND			(1 << 3)
#define TINYOS_TASK_WAIT_MASK         (0xFF << 16)//�¼��ȴ�״̬

#include "tLib.h"

struct _tEvent;

typedef uint32_t tTaskStack;

typedef struct _tTask{
	tTaskStack * stack; 
	uint32_t * stackBase;
	uint32_t stackSize;
	tNode linkNode;			 	//ͬ���ȼ���������ڵ�
	uint32_t delayTicks; 	//������ʱ����
	tNode delayNode;		 	//��ʱ����ڵ�
	uint32_t prio;				//���ȼ�
	uint32_t state;			 	//����״̬
	uint32_t slice;				//ռ��ʱ��Ƭ����
	uint32_t suspendCount;//������� 
	
	void (*clean) (void * param); //������ָ��
	void * cleanParam;
	uint8_t requestDeleteFlag;		//����ɾ����־
	
	struct _tEvent * waitEvent;		//�¼�
	void * eventMsg;							//�¼���Ϣ
	uint32_t waitEventResult;
	
	uint32_t waitFlagsType;				//�ȴ���־
	uint32_t eventFlags;					//�¼���־
}tTask;


typedef struct _tTaskInfo{
	uint32_t delayTicks;
	uint32_t prio;
	uint32_t state;
	uint32_t slice;
	uint32_t suspendCount;
	
	uint32_t stackSize;
	uint32_t stackFree;
}tTaskInfo;
void tTaskGetInfo(tTask * task, tTaskInfo * info);
void tTaskInit(tTask * task, void (*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, uint32_t size);
void tTaskSuspend (tTask * task);
void tTaskWakeUp(tTask * task);
void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param);
void tTaskForceDelete(tTask * task);
void tTaskRequestDelete(tTask * task);
uint8_t tTaskIsRequestedDeleted(void);
void tInitApp(void);
void tTaskDeleteSelf (void);
#endif

