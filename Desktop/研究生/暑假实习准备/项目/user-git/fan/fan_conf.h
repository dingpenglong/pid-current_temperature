#ifndef __FAN_CONF__H__
#define __FAN_CONF__H__
/*
@filename   fan_conf.h

@brief		
@time		2024/11/26

@author		������

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

//��ӡ������Ϣ��־����
//#define FAN_PRINT_DEBUE_INFO 1


//�����ṩ��̬����ռ�ĺ������������ʼ��TPS546D24A������
#define FAN_MALLOC(type) (type*)malloc(sizeof(type))
    
//�����ṩ�ͷŶ�̬����ռ�ĺ����������㷴��ʼ��TPS546D24A������
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
                            
                            