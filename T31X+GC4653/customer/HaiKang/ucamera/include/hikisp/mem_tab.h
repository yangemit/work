

#ifndef _MEM_TAB_H_
#define _MEM_TAB_H_
//#include "datatypedef.h"

/* 内存属性 */
typedef enum _MEM_ATTRS
{
	MEM_SCRATCH,                    /* 可复用内存，能在多路切换时有条件复用 */ 
	MEM_PERSIST                     /* 不可复用内存 */ 

} MEM_ATTRS;

/* memory spaces. */
typedef enum _MEM_SPACE 
{
	MEM_EXTERNAL_PROG,              /* 外部程序存储区          */  
	MEM_INTERNAL_PROG,              /* 内部程序存储区          */

	MEM_EXTERNAL_CACHED_DATA,       /* 外部可Cache存储区       */
	MEM_EXTERNAL_UNCACHED_DATA,     /* 外部不可Cache存储区     */
	MEM_INTERNAL_DATA,              /* 内部存储区              */
	MEM_EXTERNAL_TILERED8 ,         /* 外部Tilered数据存储区8bit，Netra/Centaurus特有 */
	MEM_EXTERNAL_TILERED16,         /* 外部Tilered数据存储区16bit，Netra/Centaurus特有 */
	MEM_EXTERNAL_TILERED32 ,        /* 外部Tilered数据存储区32bit，Netra/Centaurus特有 */
	MEM_EXTERNAL_TILEREDPAGE        /* 外部Tilered数据存储区page形式，Netra/Centaurus特有 */

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



