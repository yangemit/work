/** 
 * @file        hik_golbal_def.h
 * @brief     data type define.    
 * @details  
 * @author      Yao Jinbo
 * @date     2014.10.26
 * @version  1.0.0 
 * @par Copyright (c):  HangZhou Hikvision Digital Technology Co., Ltd.
 * @note HangZhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 * @par History:          
 *   version: author, date, desc\n 
 */ 

#ifndef _HIK_GOLBAL_DEF_
#define _HIK_GOLBAL_DEF_

/**==================== 定义数据结构相关宏===================*/
typedef unsigned long long	U64;
typedef long long			S64;

typedef unsigned long long	u64;
typedef long long			s64;

typedef unsigned int		U32;
typedef int					S32;

typedef unsigned int		u32;
typedef int					s32;

typedef unsigned short		U16;
typedef short				S16;

typedef unsigned short		u16;
typedef short				s16;

typedef unsigned char		U08;
typedef char				S08;

typedef unsigned char		u08;
typedef char				s08;

typedef unsigned char		u8;
typedef unsigned char		U8;

typedef char				s8;
typedef char				S8;

typedef float				FLT32;
typedef float				FLT;

typedef unsigned char *		PU8;

typedef unsigned int *		PU32;

typedef int	*				PS32;

typedef void				VOID;

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef OK
#define OK      0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef BOOL
#define BOOL	int
#endif

/** 定义BOOL枚举*/
typedef enum {
    HIK_FALSE = 0,
    HIK_TRUE  = 1
} HIK_BOOL;


/**================= 定义基本数学相关运算宏==============*/
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef ROUND_UP
#define ROUND_UP(size, align)     (((size) + ((align) - 1)) & ~((align) - 1))
#endif

#ifndef DIV_ROUND
#define DIV_ROUND(divident, divider)    ( ( (divident)+((divider)>>1)) / (divider) )
#endif

#ifndef CLIP
#define	CLIP(a, min, max) ((a) > (max)) ? (max) : (((a) < (min)) ? (min) : (a))
#endif

#ifndef ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))
#endif

#ifndef SQUARE
#define SQUARE(x)          ((x) * (x))
#endif

#ifndef DELAY_MS
#define DELAY_MS(val)		usleep((val)*1000)
#endif


#ifndef RC_S16
#define RC_S16(a)	((a)&0x8000)?((~(a)+1)|0x8000):((a))
#endif

#ifndef U16toU13
#define U16toU13(a)  ((a)&0x8000)?((((a)<<1)&0x1FFF)|0x1000):((a)<<1)
#endif

/**=================== 定义调试打印相关宏===================*/
/**定义打印信息命令*/
#define PRINT_INFO(print_level,en, x...)  \
		if((en)&(print_level))\
		{                         			   \
	        printf("<ISP> ");printf(x);        \
		}
		
#define PRINT_INFO1(print_level,en, x...)  \
		if((en)&(print_level))\
		{									   \
			printf(x);		  \
		}

#define PRINT_DEBUG(...)  \
		do{\
			{\
				printf( "%s %d: ", __func__,  __LINE__);\
				printf(__VA_ARGS__);\
			}\
		}while(0)\


/**判断是否出错，返回错误码*/
#define CHECK_ERROR(state, error_code)  \
		if (state)                              \
		{                                       \
			return error_code;                  \
		}

/**错误打印立刻打印*/
#define CHECK_RET(state, error_code)						\
		if (state)													\
		{\
			printf("[%s]:%d line [%s] return %d ERROR\n",__FILE__,__LINE__ , __func__, error_code);	\
			return error_code;\
		}

/**判断是否为开关量，返回错误码*/
#define CHECK_SWITCH(param, error_code)  \
		if (param > 0x1f && param < 0x0)           \
		{                                       \
			return error_code;                  \
		}


/**判断是否在给定范围内，返回错误码*/
#define CHECK_RANGE(param, rangeMin, rangeMax, error_code)  \
		if (param < rangeMin || param > rangeMax) \
		{                                       \
			return error_code;                  \
		}

/** 安全释放动态分配的内存*/
#ifndef SAFE_FREE
#define SAFE_FREE(x) do { if ((x) != NULL) { free((x)); (x) = NULL;} } while(0)
#endif

/**  安全关闭文件描述符*/
#ifndef SAFE_CLOSE
#define SAFE_CLOSE(fd) do { if (-1 != fd) { close(fd); fd = -1; } } while(0)
#endif

#endif /* _HIK_GOLBAL_DEF_ */

