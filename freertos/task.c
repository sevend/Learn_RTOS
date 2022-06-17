#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"



/*
*************************************************************************
*                               ������ƿ�
*************************************************************************
*/

/* ��ǰ�������е������������ƿ�ָ�룬Ĭ�ϳ�ʼ��ΪNULL */
TCB_t * volatile pxCurrentTCB = NULL;

/* ��������б� */
List_t pxReadyTasksLists[ configMAX_PRIORITIES ];


static volatile UBaseType_t uxCurrentNumberOfTasks 	= ( UBaseType_t ) 0U;


/*
*************************************************************************
*                               ��������
*************************************************************************
*/

static void prvInitialiseNewTask( 	TaskFunction_t pxTaskCode,              /* ������� */
									const char * const pcName,              /* �������ƣ��ַ�����ʽ */
									const uint32_t ulStackDepth,            /* ����ջ��С����λΪ�� */
									void * const pvParameters,              /* �����β� */
									TaskHandle_t * const pxCreatedTask,     /* ������ */
									TCB_t *pxNewTCB );


/*
*************************************************************************
*                               �궨��
*************************************************************************
*/
                                    

                                   

/*
*************************************************************************
*                               ��̬���񴴽�����
*************************************************************************
*/
#if( configSUPPORT_STATIC_ALLOCATION == 1 )

TaskHandle_t xTaskCreateStatic(	TaskFunction_t pxTaskCode,           /* ������� */
					            const char * const pcName,           /* �������ƣ��ַ�����ʽ */
					            const uint32_t ulStackDepth,         /* ����ջ��С����λΪ�� */
					            void * const pvParameters,           /* �����β� */
					            StackType_t * const puxStackBuffer,  /* ����ջ��ʼ��ַ */
					            TCB_t * const pxTaskBuffer )         /* ������ƿ�ָ�� */
{
	TCB_t *pxNewTCB;
	TaskHandle_t xReturn;

	if( ( pxTaskBuffer != NULL ) && ( puxStackBuffer != NULL ) )
	{		
		pxNewTCB = ( TCB_t * ) pxTaskBuffer; 
		pxNewTCB->pxStack = ( StackType_t * ) puxStackBuffer;

		/* �����µ����� */
		prvInitialiseNewTask( pxTaskCode,        /* ������� */
                              pcName,            /* �������ƣ��ַ�����ʽ */
                              ulStackDepth,      /* ����ջ��С����λΪ�� */ 
                              pvParameters,      /* �����β� */
                              &xReturn,          /* ������ */ 
                              pxNewTCB);         /* ����ջ��ʼ��ַ */      

	}
	else
	{
		xReturn = NULL;
	}

	/* ������������������񴴽��ɹ�����ʱxReturnӦ��ָ��������ƿ� */
    return xReturn;
}

#endif /* configSUPPORT_STATIC_ALLOCATION */

/* ��ʼ�����񣬷���������ƿ�ָ��TCB
*/
static void prvInitialiseNewTask( 	TaskFunction_t pxTaskCode,              /* ������� */
									const char * const pcName,              /* �������ƣ��ַ�����ʽ */
									const uint32_t ulStackDepth,            /* ����ջ��С����λΪ�� */
									void * const pvParameters,              /* �����β� */
									TaskHandle_t * const pxCreatedTask,     /* ������ */
									TCB_t *pxNewTCB )                       /* ������ƿ�ָ�� */

{
	StackType_t *pxTopOfStack;
	UBaseType_t x;	
	
	/* ��ȡջ����ַ */
	pxTopOfStack = pxNewTCB->pxStack + ( ulStackDepth - ( uint32_t ) 1 );
	//pxTopOfStack = ( StackType_t * ) ( ( ( portPOINTER_SIZE_TYPE ) pxTopOfStack ) & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) );
	/* ������8�ֽڶ��� */
	pxTopOfStack = ( StackType_t * ) ( ( ( uint32_t ) pxTopOfStack ) & ( ~( ( uint32_t ) 0x0007 ) ) );	

	/* ����������ִ洢��TCB�� */
	for( x = ( UBaseType_t ) 0; x < ( UBaseType_t ) configMAX_TASK_NAME_LEN; x++ )
	{
		pxNewTCB->pcTaskName[ x ] = pcName[ x ];

		if( pcName[ x ] == 0x00 )
		{
			break;
		}
	}
	/* �������ֵĳ��Ȳ��ܳ���configMAX_TASK_NAME_LEN */
	pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';

    /* ��ʼ��TCB�е�xStateListItem�ڵ� */
    vListInitialiseItem( &( pxNewTCB->xStateListItem ) );
    /* ����xStateListItem�ڵ��ӵ���� */
	listSET_LIST_ITEM_OWNER( &( pxNewTCB->xStateListItem ), pxNewTCB );
    
    
    /* ��ʼ������ջ */
	pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack, pxTaskCode, pvParameters );   


	/* ��������ָ��������ƿ� */
    if( ( void * ) pxCreatedTask != NULL )
	{		
		*pxCreatedTask = ( TaskHandle_t ) pxNewTCB;
	}
}

/* ��ʼ��������ص��б� */
/* �����б��ʼ������һ�����飬�����Ա��˫������
** ��ʼ����ɣ������Ա��ֻ�ǵ������ڵ�˫������

** ִ��˫���������
*/
void prvInitialiseTaskLists( void )
{
    UBaseType_t uxPriority;
    
    
    for( uxPriority = ( UBaseType_t ) 0U; uxPriority < ( UBaseType_t ) configMAX_PRIORITIES; uxPriority++ )
	{
		vListInitialise( &( pxReadyTasksLists[ uxPriority ] ) );
	}
}

