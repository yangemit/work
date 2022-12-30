

#ifndef _MEM_TAB_H_
#define _MEM_TAB_H_
//#include "datatypedef.h"

/* �ڴ����� */
typedef enum _MEM_ATTRS
{
	MEM_SCRATCH,                    /* �ɸ����ڴ棬���ڶ�·�л�ʱ���������� */ 
	MEM_PERSIST                     /* ���ɸ����ڴ� */ 

} MEM_ATTRS;

/* memory spaces. */
typedef enum _MEM_SPACE 
{
	MEM_EXTERNAL_PROG,              /* �ⲿ����洢��          */  
	MEM_INTERNAL_PROG,              /* �ڲ�����洢��          */

	MEM_EXTERNAL_CACHED_DATA,       /* �ⲿ��Cache�洢��       */
	MEM_EXTERNAL_UNCACHED_DATA,     /* �ⲿ����Cache�洢��     */
	MEM_INTERNAL_DATA,              /* �ڲ��洢��              */
	MEM_EXTERNAL_TILERED8 ,         /* �ⲿTilered���ݴ洢��8bit��Netra/Centaurus���� */
	MEM_EXTERNAL_TILERED16,         /* �ⲿTilered���ݴ洢��16bit��Netra/Centaurus���� */
	MEM_EXTERNAL_TILERED32 ,        /* �ⲿTilered���ݴ洢��32bit��Netra/Centaurus���� */
	MEM_EXTERNAL_TILEREDPAGE        /* �ⲿTilered���ݴ洢��page��ʽ��Netra/Centaurus���� */

} MEM_SPACE;

/* Memory table. */
typedef struct _MEM_TAB
{
	void*           base;       // Base address of allocated buf
	unsigned int    size;       // Size in MAU of allocation
	unsigned int    alignment;  // Alignment requirement (MAU)
	MEM_SPACE		space;      // Allocation space
	MEM_ATTRS		attrs;      // Memory attributes
} MEM_TAB;

#endif // _MEM_TAB_H_



