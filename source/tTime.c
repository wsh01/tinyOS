#include "tinyOS.h"


void tTaskDelay(uint32_t delay)			//������ʱ
{
	uint32_t status = tTaskEnterCritical();//�����ٽ�������
	
	tTimeTaskWait(currentTask,delay);
	
	tTaskSchedUnRdy(currentTask);
	tTaskExitCritical(status);	//�˳��ٽ�������
	
	tTaskSched();
}

