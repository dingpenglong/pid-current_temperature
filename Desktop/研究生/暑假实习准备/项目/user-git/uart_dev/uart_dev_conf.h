/*
@filename	:	uart_dev_conf.h
@brief		:	用于配置uart_dev驱动的某些行为,比如提供动态内存管理函数。
@time		:	2024/12/02
@author		: 	黎健

@version    :   1.0
*/

#ifndef __UART_DEV_CONF_H__
#define __UART_DEV_CONF_H__ 

#ifdef __cplusplus
extern "C" {
#endif

//必须提供动态申请空间的函数，以满足初始化串口驱动的需求
#define UART_DEV_MALLOC(type, num) (type*)malloc(sizeof(type) * (num))
    
//必须提供释放动态申请空间的函数，以满足反初始化M601驱动的需求
#define UART_DEV_FREE(ptr)  do{\
                                if(ptr)\
                                {\
                                    free(ptr);\
                                    ptr = NULL;\
                                }\
                            }while(0)

#ifdef __cplusplus
}
#endif

#endif  /* end of __UART_DEV_CONF_H__ */
