/*
@filename	:	ring_buffer_conf.h
@brief		:	���û��λ����������������Ϊ
@time		:	2024/12/2
@author		: 	������

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

//�����ṩ��̬����ռ�ĺ���
#define RING_BUFFER_MALLOC(type, num) (type*)malloc(sizeof(type) * (num))
    
//�����ṩ�ͷŶ�̬����ռ�ĺ���
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
