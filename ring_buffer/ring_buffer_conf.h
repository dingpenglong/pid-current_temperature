/*
@filename	:	ring_buffer_conf.h
@brief		:	配置环形缓冲区驱动代码的行为
@time		:	2024/12/2
@author		: 	丁鹏龙

@version    :   1.0
*/

// <<< Use Configuration Wizard in Context Menu >>>
// <c1>Enable Debug Mode
//  <i>Enable print debug information by printf function.
#include <stdio.h>
#define RING_BUFFER_PRINT_DEBUG_INFO
// </c>

// <<< end of configuration section >>>

#ifndef __RING_BUFFER_CONF_H__
#define __RING_BUFFER_CONF_H__ 

#ifdef __cplusplus
extern "C" {
#endif

//必须提供动态申请空间的函数
#define RING_BUFFER_MALLOC(type, num) (type*)malloc(sizeof(type) * (num))
    
//必须提供释放动态申请空间的函数
#define RING_BUFFER_FREE(ptr)  do{\
                                if(ptr)\
                                {\
                                    free(ptr);\
                                    ptr = NULL;\
                                }\
                            }while(0)

#ifdef __cplusplus
}
#endif

#endif  /* end of __RING_BUFFER_CONF_H__ */
