#ifndef __DRV8412_CONF__H__
#define __DRV8412_CONF__H__

/*
@filename   drv8412_conf.h

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


//��ӡ������Ϣ��־����
//#define DRV8412_PRINT_DEBUE_INFO 1


//�����ṩ��̬����ռ�ĺ������������ʼ��drv8412������
#define DRV8412_MALLOC(type) (type*)malloc(sizeof(type))
    
//�����ṩ�ͷŶ�̬����ռ�ĺ����������㷴��ʼ��TPS546D24A������
#define DRV8412_FREE(ptr)  do{\
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
                            
                            