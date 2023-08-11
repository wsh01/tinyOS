#include "tinyOS.h"


void tTaskDelay(uint32_t delay)			//任务软定时
{
	uint32_t status = tTaskEnterCritical();//进入临界区保护
	
	tTimeTaskWait(currentTask,delay);
	
	tTaskSchedUnRdy(currentTask);
	tTaskExitCritical(status);	//退出临界区保护
	
	tTaskSched();
}

