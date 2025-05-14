#ifndef __TPS546D24A_CONF__H__
#define __TPS546D24A_CONF__H__
/*
@filename   tps546d24a_conf.h

@brief		基于PMBUS控制的恒流源

@time		2024/10/10

@author		丁鹏龙

@version    1.0

--------------------------------------------------------

@attention


*/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "delay.h"


//打印调试信息日志开关
//#define TPS546D24A_PRINT_DEBUG_INFO 1


//必须提供动态申请空间的函数，以满足初始化TPS546D24A的需求
#define TPS546D24A_MALLOC(type) (type*)malloc(sizeof(type))
    
//必须提供释放动态申请空间的函数，以满足反初始化TPS546D24A的需求
#define TPS546D24A_FREE(ptr)  do{\
                                if(ptr)\
                                {\
                                    free(ptr);\
                                    ptr = NULL;\
                                }\
                            }while(0)

#ifdef __cplusplus
}
#endif

#endif /* __TPS546D24A_CONF__H__ */