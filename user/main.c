/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/
#include "list.h"

/*
*************************************************************************
*                              ȫ�ֱ���
*************************************************************************
*/

/* ����������ڵ� */
struct xLIST       List_Test;

/* ����ڵ� */
struct xLIST_ITEM  List_Item1;
struct xLIST_ITEM  List_Item2;
struct xLIST_ITEM  List_Item3;
struct xLIST_ITEM  List_Item4;

int remain_num;


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
	
    /* ������ڵ��ʼ�� */
    vListInitialise( &List_Test );
    
    /* �ڵ�1��ʼ�� */
    vListInitialiseItem( &List_Item1 );
    List_Item1.xItemValue = 1;
    
    /* �ڵ�2��ʼ�� */    
    vListInitialiseItem( &List_Item2 );
    List_Item2.xItemValue = 2;
    
    /* �ڵ�3��ʼ�� */
    vListInitialiseItem( &List_Item3 );
    List_Item3.xItemValue = 3;
	
	/* �ڵ�4��ʼ�� */
    vListInitialiseItem( &List_Item4 );
    List_Item3.xItemValue = 4;
    
    /* ���ڵ�������������������� */
    vListInsert( &List_Test, &List_Item1 );    
    vListInsert( &List_Test, &List_Item3 );  
	/* �����м����ʵ��  */
    vListInsert( &List_Test, &List_Item2 );	
	
	/* β������ ʵ��  */
    vListInsertEnd( &List_Test, &List_Item4 );
	
	remain_num = uxListRemove(&List_Item2);
	
	if(remain_num == 3)
	{
		/* ���Ա�ʶ���������ѭ������ɾ���ɹ� */
		while(1){}
		
	}
    
    for(;;)
	{
		/* ɶ�²��� */
	}
}


