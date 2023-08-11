#include "tinyOS.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL					0xE000ED04	 //终端控制及状态寄存器ICSR,32位
#define NVIC_PENDSVSET				0x10000000   //悬起PendSV。读取它返回PendSV的状态
#define NVIC_SYSPRI2 					0xE000ED22   //PendSV优先级寄存器PRI_14，8位
#define NVIC_FENDSV_PRI				0x000000FF   //最低优先级

/*
volatile提醒编译器它后面所定义的变量随时都有可能改变，
因此编译后的程序每次需要存储或读取这个变量的时候，告诉编译器对该变量不做优化，
都会直接从变量内存地址中读取数据，从而可以提供对特殊地址的稳定访问。
*/
#define MEM32(addr)						*(volatile unsigned long *)(addr)
#define MEM8(addr)						*(volatile unsigned char *)(addr) //位带操作

uint32_t tTaskEnterCritical(void)//进入临界区关中断
{
	uint32_t primask = __get_PRIMASK(); //保存中断使能配置，防止嵌套关中断时外层临界区保护失败
	__disable_irq();
	return primask;
}

void tTaskExitCritical(uint32_t status)//退出临界区开中断
{
	__set_PRIMASK(status);
}

__asm void PendSV_Handler (void) //内联汇编，PendSV异常处理函数，类似与中断服务函数，函数名称是固定的
{
	IMPORT currentTask
	IMPORT nextTask
																	//MRS状态寄存器到通用寄存器的传送指令,得到currentTask
	MRS R0, PSP											//在任务中，所有栈空间的使用都是通过R13的PSP指针进行指向的。
	CBZ R0, PendSVHandler_nosave		//判断跳转
	
	STMDB R0!, {R4-R11}							//将R4到R11保存到PSP，其他寄存器会自动保存
	
	LDR R1, =currentTask						//取currentTask地址
	LDR R1, [R1]										//得到当前任务结构体的栈地址
	STR R0, [R1]										//将PSP栈的内容保存到任务栈中
			
PendSVHandler_nosave
	LDR R0, =currentTask						
	LDR R1, = nextTask
	LDR R2, [R1]										//得到下个任务结构体的栈地址
	STR R2, [R0]										//currentTask=nextTask
	
	LDR R0, [R2]										//取nextTask的内容
	LDMIA R0!, {R4-R11}							//从nextTask中恢复R4到R11
	
	MSR PSP, R0											//MSR通用寄存器到状态寄存器的传送指令，将PSP指向nextTask，切换到nextTask
	ORR LR, LR, #0x04								//将LR（R１４）寄存器的２位段设置为１，表示从进程堆栈中做出栈操作，返回后使用PSP。为０则使用MSP
	
	BX LR
/*在进入PendSV和退出PendSV时自动保存和恢复一部分寄存器，其过程操作的是PSP所指向的用户堆栈并且遵循一定的顺序，
	所以有22行和40行的切换操作*/	
}

void tTaskRunFirst()
{
	__set_PSP(0);//将PSP设置为零，当作一个初始任务的标志，在切换任务函数中进行判断
	
	MEM8(NVIC_SYSPRI2) = NVIC_FENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; //位带操作，写入寄存器，触发PendSV异常，进入处理函数，书123页
}

void tTaskSwitch()
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}


