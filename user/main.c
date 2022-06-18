/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/
#include <stdint.h>
#include "portmacro.h"
#include "FreeRTOS.h"
#include "task.h"
#include "list.h"

/*
*************************************************************************
*                              ȫ�ֱ���
*************************************************************************
*/

uint32_t flag1;
uint32_t flag2;


/*
*************************************************************************
*                        ������ƿ� & STACK 
*************************************************************************
*/

#define TASK1_STACK_SIZE                    20
StackType_t Task1Stack[TASK1_STACK_SIZE];
TCB_t Task1TCB;

#define TASK2_STACK_SIZE                    20
StackType_t Task2Stack[TASK2_STACK_SIZE];
TCB_t Task2TCB;

TaskHandle_t Task1_Handle;
TaskHandle_t Task2_Handle;


/* ��������б� */
extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];

/*
*************************************************************************
*                               �������� 
*************************************************************************
*/
void delay (uint32_t count);
void Task1_Entry( void *p_arg );
void Task2_Entry( void *p_arg );


/*
************************************************************************
*                                main����
************************************************************************
*/
/*
* ע�����1���ù���ʹ��������棬debug��ѡ�� Ude Simulator
*           2����Targetѡ�����Ѿ���Xtal(Mhz)��ֵ��Ϊ25��Ĭ����12��
*              �ĳ�25��Ϊ�˸�system_ARMCM3.c�ж����__SYSTEM_CLOCK��ͬ��ȷ�������ʱ��ʱ��һ��
*/
int main(void)
{	
	
	
	 /* ��ʼ����������ص��б�������б� */
    prvInitialiseTaskLists();

	
	    /* �������� */
    Task1_Handle = xTaskCreateStatic( (TaskFunction_t)Task1_Entry,   /* ������� */
					                  (char *)"Task1",               /* �������ƣ��ַ�����ʽ */
					                  (uint32_t)TASK1_STACK_SIZE ,   /* ����ջ��С����λΪ�� */
					                  (void *) NULL,                 /* �����β� */
					                  (StackType_t *)Task1Stack,     /* ����ջ��ʼ��ַ */
					                  (TCB_t *)&Task1TCB );          /* ������ƿ� */
									  
	/* ��������ӵ������б� */                                 
    vListInsertEnd( &( pxReadyTasksLists[1] ), &( ((TCB_t *)(&Task1TCB))->xStateListItem ) );

									  
	Task2_Handle = xTaskCreateStatic( (TaskFunction_t)Task2_Entry,   /* ������� */
					                  (char *)"Task2",               /* �������ƣ��ַ�����ʽ */
					                  (uint32_t)TASK2_STACK_SIZE ,   /* ����ջ��С����λΪ�� */
					                  (void *) NULL,                 /* �����β� */
					                  (StackType_t *)Task2Stack,     /* ����ջ��ʼ��ַ */
					                  (TCB_t *)&Task2TCB );          /* ������ƿ� */
    /* ��������ӵ������б� */                                   
    vListInsertEnd( &( pxReadyTasksLists[2] ), &( ((TCB_t *)(&Task2TCB))->xStateListItem ) );
									  
	/* ��������������ʼ��������ȣ������ɹ��򲻷��� */								  
	vTaskStartScheduler();

    
    for(;;)
	{
	
	}
}


/*
*************************************************************************
*                               ����ʵ��
*************************************************************************
*/

/* �����ʱ */
void delay(uint32_t count)
{
	for(;count!=0;count--);
	
}

/* ����1 */
void Task1_Entry( void *p_arg )
{
	for( ;; )
	{
		flag1 = 1;
		vTaskDelay( 2 );		
		flag1 = 0;
		vTaskDelay( 2 );

	}
}

/* ����2 */
void Task2_Entry( void *p_arg )
{
	for( ;; )
	{
		flag2 = 1;
		vTaskDelay( 2 );		
		flag2 = 0;
		vTaskDelay( 2 );
		
	}
}

/* ��ȡ����������ڴ� */
StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
TCB_t IdleTaskTCB;
void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize )
{
		*ppxIdleTaskTCBBuffer=&IdleTaskTCB;
		*ppxIdleTaskStackBuffer=IdleTaskStack; 
		*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;
}

