#ifndef __FAN_CONF__H__
#define __FAN_CONF__H__
/*
@filename   fan_conf.h

@brief		
@time		2024/11/26

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
#include "stdlib.h"

//打印调试信息日志开关
//#define FAN_PRINT_DEBUE_INFO 1


//必须提供动态申请空间的函数，以满足初始化TPS546D24A的需求
#define FAN_MALLOC(type) (type*)malloc(sizeof(type))
    
//必须提供释放动态申请空间的函数，以满足反初始化TPS546D24A的需求
#define FAN_FREE(ptr)  do{\
                                if(ptr)\
                                {\
                                    free(ptr);\
                                    ptr = NULL;\
                                }\
                            }while(0)

#ifdef __cplusplus
}
#endif

#endif /* __FAN_CONF__H__ */
                            
                            