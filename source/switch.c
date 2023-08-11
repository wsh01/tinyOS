#include "tinyOS.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL					0xE000ED04	 //�ն˿��Ƽ�״̬�Ĵ���ICSR,32λ
#define NVIC_PENDSVSET				0x10000000   //����PendSV����ȡ������PendSV��״̬
#define NVIC_SYSPRI2 					0xE000ED22   //PendSV���ȼ��Ĵ���PRI_14��8λ
#define NVIC_FENDSV_PRI				0x000000FF   //������ȼ�

/*
volatile���ѱ�����������������ı�����ʱ���п��ܸı䣬
��˱����ĳ���ÿ����Ҫ�洢���ȡ���������ʱ�򣬸��߱������Ըñ��������Ż���
����ֱ�Ӵӱ����ڴ��ַ�ж�ȡ���ݣ��Ӷ������ṩ�������ַ���ȶ����ʡ�
*/
#define MEM32(addr)						*(volatile unsigned long *)(addr)
#define MEM8(addr)						*(volatile unsigned char *)(addr) //λ������

uint32_t tTaskEnterCritical(void)//�����ٽ������ж�
{
	uint32_t primask = __get_PRIMASK(); //�����ж�ʹ�����ã���ֹǶ�׹��ж�ʱ����ٽ�������ʧ��
	__disable_irq();
	return primask;
}

void tTaskExitCritical(uint32_t status)//�˳��ٽ������ж�
{
	__set_PRIMASK(status);
}

__asm void PendSV_Handler (void) //������࣬PendSV�쳣���������������жϷ����������������ǹ̶���
{
	IMPORT currentTask
	IMPORT nextTask
																	//MRS״̬�Ĵ�����ͨ�üĴ����Ĵ���ָ��,�õ�currentTask
	MRS R0, PSP											//�������У�����ջ�ռ��ʹ�ö���ͨ��R13��PSPָ�����ָ��ġ�
	CBZ R0, PendSVHandler_nosave		//�ж���ת
	
	STMDB R0!, {R4-R11}							//��R4��R11���浽PSP�������Ĵ������Զ�����
	
	LDR R1, =currentTask						//ȡcurrentTask��ַ
	LDR R1, [R1]										//�õ���ǰ����ṹ���ջ��ַ
	STR R0, [R1]										//��PSPջ�����ݱ��浽����ջ��
			
PendSVHandler_nosave
	LDR R0, =currentTask						
	LDR R1, = nextTask
	LDR R2, [R1]										//�õ��¸�����ṹ���ջ��ַ
	STR R2, [R0]										//currentTask=nextTask
	
	LDR R0, [R2]										//ȡnextTask������
	LDMIA R0!, {R4-R11}							//��nextTask�лָ�R4��R11
	
	MSR PSP, R0											//MSRͨ�üĴ�����״̬�Ĵ����Ĵ���ָ���PSPָ��nextTask���л���nextTask
	ORR LR, LR, #0x04								//��LR��R�������Ĵ����ģ�λ������Ϊ������ʾ�ӽ��̶�ջ������ջ���������غ�ʹ��PSP��Ϊ����ʹ��MSP
	
	BX LR
/*�ڽ���PendSV���˳�PendSVʱ�Զ�����ͻָ�һ���ּĴ���������̲�������PSP��ָ����û���ջ������ѭһ����˳��
	������22�к�40�е��л�����*/	
}

void tTaskRunFirst()
{
	__set_PSP(0);//��PSP����Ϊ�㣬����һ����ʼ����ı�־�����л��������н����ж�
	
	MEM8(NVIC_SYSPRI2) = NVIC_FENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; //λ��������д��Ĵ���������PendSV�쳣�����봦��������123ҳ
}

void tTaskSwitch()
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}


