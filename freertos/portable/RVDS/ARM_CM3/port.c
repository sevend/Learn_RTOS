#include "FreeRTOS.h"
#include "task.h"
#include "ARMCM3.h"
#include "portmacro.h"

static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

/*
*************************************************************************
*                              �궨��
*************************************************************************
*/
#define portINITIAL_XPSR			        ( 0x01000000 )
#define portSTART_ADDRESS_MASK				( ( StackType_t ) 0xfffffffeUL )


/* 
 * �ο����ϡ�STM32F10xxx Cortex-M3 programming manual��4.4.3���ٶ�������PM0056�������ҵ�����ĵ�
 * ��Cortex-M�У��ں�����SCB��SHPR3�Ĵ�����������SysTick��PendSV���쳣���ȼ�
 * System handler priority register 3 (SCB_SHPR3) SCB_SHPR3��0xE000 ED20
 * Bits 31:24 PRI_15[7:0]: Priority of system handler 15, SysTick exception 
 * Bits 23:16 PRI_14[7:0]: Priority of system handler 14, PendSV 
 */
#define portNVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )

#define portNVIC_PENDSV_PRI					( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI				( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )


/* ***************************
**  SysTick ���üĴ���
********************************/
#define portNVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )

#ifndef configSYSTICK_CLOCK_HZ
	#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
	/* ȷ��SysTick��ʱ�����ں�ʱ��һ�� */
	#define portNVIC_SYSTICK_CLK_BIT	( 1UL << 2UL )
#else
	#define portNVIC_SYSTICK_CLK_BIT	( 0 )
#endif

#define portNVIC_SYSTICK_INT_BIT			( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT			( 1UL << 0UL )

/*
*************************************************************************
*                              ��������
*************************************************************************
*/
void prvStartFirstTask( void );
void vPortSVCHandler( void );
void xPortPendSVHandler( void );
/* ϵͳʱ�� */
void vPortSetupTimerInterrupt( void );
/*
*************************************************************************
*                              ����ջ��ʼ������
*************************************************************************
*/

static void prvTaskExitError( void )
{
    /* ����ֹͣ������ */
    for(;;);
}

StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    /* �쳣����ʱ���Զ����ص�CPU�Ĵ��������� */
	pxTopOfStack--;
	*pxTopOfStack = portINITIAL_XPSR;	                                    /* xPSR��bit24������1 */
	pxTopOfStack--;
	*pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK;	/* PC����������ں��� */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) prvTaskExitError;	                    /* LR���������ص�ַ */
	pxTopOfStack -= 5;	/* R12, R3, R2 and R1 Ĭ�ϳ�ʼ��Ϊ0 */
	*pxTopOfStack = ( StackType_t ) pvParameters;	                        /* R0�������β� */
    
    /* �쳣����ʱ���ֶ����ص�CPU�Ĵ��������� */    
	pxTopOfStack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4Ĭ�ϳ�ʼ��Ϊ0 */

	/* ����ջ��ָ�룬��ʱpxTopOfStackָ�����ջ */
    return pxTopOfStack;
}

/*
*************************************************************************
*                              ��������������
*************************************************************************
*/


BaseType_t xPortStartScheduler( void )
{
    /* ����PendSV �� SysTick ���ж����ȼ�Ϊ��� */
	portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
	portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;
	
	/* ��ʼ��SysTick */
    vPortSetupTimerInterrupt();


	/* ������һ�����񣬲��ٷ��� */
	prvStartFirstTask();

	/* ��Ӧ�����е����� */
	return 0;
}

/*
 * �ο����ϡ�STM32F10xxx Cortex-M3 programming manual��4.4.3���ٶ�������PM0056�������ҵ�����ĵ�
 * ��Cortex-M�У��ں�����SCB�ĵ�ַ��ΧΪ��0xE000ED00-0xE000ED3F
 * 0xE000ED008ΪSCB������SCB_VTOR����Ĵ����ĵ�ַ�������ŵ������������ʼ��ַ����MSP�ĵ�ַ
 */
__asm void prvStartFirstTask( void )
{
	PRESERVE8

	/* ��Cortex-M�У�0xE000ED08��SCB_VTOR����Ĵ����ĵ�ַ��
       �����ŵ������������ʼ��ַ����MSP�ĵ�ַ */
	ldr r0, =0xE000ED08
	ldr r0, [r0]
	ldr r0, [r0]

	/* ��������ջָ��msp��ֵ */
	msr msp, r0
    
	/* ʹ��ȫ���ж� */
	cpsie i
	cpsie f
	dsb
	isb
	
    /* ����SVCȥ������һ������ */
	svc 0  
	nop
	nop
}

__asm void vPortSVCHandler( void )
{
    extern pxCurrentTCB;
    
    PRESERVE8

	ldr	r3, =pxCurrentTCB	/* ����pxCurrentTCB�ĵ�ַ��r3 */
	ldr r1, [r3]			/* ����pxCurrentTCB��r1 */
	ldr r0, [r1]			/* ����pxCurrentTCBָ���ֵ��r0��Ŀǰr0��ֵ���ڵ�һ�������ջ��ջ�� */
	ldmia r0!, {r4-r11}		/* ��r0Ϊ����ַ����ջ��������ݼ��ص�r4~r11�Ĵ�����ͬʱr0����� */
	msr psp, r0				/* ��r0��ֵ���������ջָ����µ�psp */
	isb
	mov r0, #0              /* ����r0��ֵΪ0 */
	msr	basepri, r0         /* ����basepri�Ĵ�����ֵΪ0�������е��ж϶�û�б����� */
	orr r14, #0xd           /* ����SVC�жϷ����˳�ǰ,ͨ����r14�Ĵ������4λ��λ����0x0D��
                               ʹ��Ӳ�����˳�ʱʹ�ý��̶�ջָ��PSP��ɳ�ջ���������غ�����߳�ģʽ������Thumb״̬ */
    
	bx r14                  /* �쳣���أ����ʱ��ջ�е�ʣ�����ݽ����Զ����ص�CPU�Ĵ�����
                               xPSR��PC��������ڵ�ַ����R14��R12��R3��R2��R1��R0��������βΣ�
                               ͬʱPSP��ֵҲ�����£���ָ������ջ��ջ�� */
}

__asm void xPortPendSVHandler( void )
{
	extern pxCurrentTCB;
	extern vTaskSwitchContext;

	PRESERVE8

    /* ������PendSVC Handlerʱ����һ���������еĻ�������
       xPSR��PC��������ڵ�ַ����R14��R12��R3��R2��R1��R0��������βΣ�
       ��ЩCPU�Ĵ�����ֵ���Զ����浽�����ջ�У�ʣ�µ�r4~r11��Ҫ�ֶ����� */
    /* ��ȡ����ջָ�뵽r0 */
	mrs r0, psp
	isb

	ldr	r3, =pxCurrentTCB		/* ����pxCurrentTCB�ĵ�ַ��r3 */
	ldr	r2, [r3]                /* ����pxCurrentTCB��r2 */

	stmdb r0!, {r4-r11}			/* ��CPU�Ĵ���r4~r11��ֵ�洢��r0ָ��ĵ�ַ */
	str r0, [r2]                /* ������ջ���µ�ջ��ָ��洢����ǰ����TCB�ĵ�һ����Ա����ջ��ָ�� */				
                               

	stmdb sp!, {r3, r14}        /* ��R3��R14��ʱѹ���ջ����Ϊ�������ú���vTaskSwitchContext,
                                  ���ú���ʱ,���ص�ַ�Զ����浽R14��,����һ�����÷���,R14��ֵ�ᱻ����,�����Ҫ��ջ����;
                                  R3����ĵ�ǰ���������TCBָ��(pxCurrentTCB)��ַ,�������ú���õ�,���ҲҪ��ջ���� */
	mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY    /* �����ٽ�� */
	msr basepri, r0
	dsb
	isb
	bl vTaskSwitchContext       /* ���ú���vTaskSwitchContext��Ѱ���µ���������,ͨ��ʹ����pxCurrentTCBָ���µ�������ʵ�������л� */ 
	mov r0, #0                  /* �˳��ٽ�� */
	msr basepri, r0
	ldmia sp!, {r3, r14}        /* �ָ�r3��r14 */

	ldr r1, [r3]
	ldr r0, [r1] 				/* ��ǰ���������TCB��һ����������ջ��ջ��,����ջ��ֵ����R0*/
	ldmia r0!, {r4-r11}			/* ��ջ */
	msr psp, r0
	isb
	bx r14                      /* �쳣����ʱ,R14�б����쳣���ر�־,�������غ�����߳�ģʽ���Ǵ�����ģʽ��
                                   ʹ��PSP��ջָ�뻹��MSP��ջָ�룬������ bx r14ָ���Ӳ����֪��Ҫ���쳣���أ�
                                   Ȼ���ջ�����ʱ���ջָ��PSP�Ѿ�ָ�����������ջ����ȷλ�ã�
                                   ������������е�ַ����ջ��PC�Ĵ������µ�����Ҳ�ᱻִ�С�*/
	nop
}

/*
*************************************************************************
*                             �ٽ����غ���
*************************************************************************
*/
void vPortEnterCritical( void )
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;

	/* This is not the interrupt safe version of the enter critical function so
	assert() if it is being called from an interrupt context.  Only API
	functions that end in "FromISR" can be used in an interrupt.  Only assert if
	the critical nesting count is 1 to protect against recursive calls if the
	assert function also uses a critical section. */
	if( uxCriticalNesting == 1 )
	{
		//configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
	}
}

void vPortExitCritical( void )
{
	//configASSERT( uxCriticalNesting );
	uxCriticalNesting--;
    
	if( uxCriticalNesting == 0 )
	{
		portENABLE_INTERRUPTS();
	}
}

/*
*************************************************************************
*                             ��ʼ��SysTick
*************************************************************************
*/
void vPortSetupTimerInterrupt( void )
{
     /* ������װ�ؼĴ�����ֵ */
    portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
    
    /* ����ϵͳ��ʱ����ʱ�ӵ����ں�ʱ��
       ʹ��SysTick ��ʱ���ж�
       ʹ��SysTick ��ʱ�� */
    portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT | 
                                  portNVIC_SYSTICK_INT_BIT |
                                  portNVIC_SYSTICK_ENABLE_BIT ); 
}

/*
*************************************************************************
*                             SysTick�жϷ�����
*************************************************************************
*/

void xPortSysTickHandler( void )
{
	/* ���ж� */
    vPortRaiseBASEPRI();
    
    {
        //xTaskIncrementTick();
        
        /* ����ϵͳʱ�� 
		������������Ҿ�����������ȼ��ȵ�ǰ���ȼ���ʱ����ִ��һ�������л�
		*/
		if( xTaskIncrementTick() != pdFALSE )
		{
			/* �����л���������PendSV */
            //portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
            taskYIELD();
		}
	}

	/* ���ж� */
    vPortClearBASEPRIFromISR();
}

