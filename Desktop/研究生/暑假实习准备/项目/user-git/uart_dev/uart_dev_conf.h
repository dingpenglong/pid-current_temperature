/*
@filename	:	uart_dev_conf.h
@brief		:	��������uart_dev������ĳЩ��Ϊ,�����ṩ��̬�ڴ��������
@time		:	2024/12/02
@author		: 	�轡

@version    :   1.0
*/

#ifndef __UART_DEV_CONF_H__
#define __UART_DEV_CONF_H__ 

#ifdef __cplusplus
extern "C" {
#endif

//�����ṩ��̬����ռ�ĺ������������ʼ����������������
#define UART_DEV_MALLOC(type, num) (type*)malloc(sizeof(type) * (num))
    
//�����ṩ�ͷŶ�̬����ռ�ĺ����������㷴��ʼ��M601����������
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
