#include "FreeRTOS.h"
#include "task.h"
#include "ARMCM3.h"
#include "portmacro.h"

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



/*
*************************************************************************
*                              ��������
*************************************************************************
*/
void prvStartFirstTask( void );
void vPortSVCHandler( void );
void xPortPendSVHandler( void );


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

