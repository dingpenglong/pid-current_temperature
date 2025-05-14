#ifndef __TPS546D24A_CONF__H__
#define __TPS546D24A_CONF__H__
/*
@filename   tps546d24a_conf.h

@brief		����PMBUS���Ƶĺ���Դ

@time		2024/10/10

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


//��ӡ������Ϣ��־����
//#define TPS546D24A_PRINT_DEBUG_INFO 1


//�����ṩ��̬����ռ�ĺ������������ʼ��TPS546D24A������
#define TPS546D24A_MALLOC(type) (type*)malloc(sizeof(type))
    
//�����ṩ�ͷŶ�̬����ռ�ĺ����������㷴��ʼ��TPS546D24A������
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