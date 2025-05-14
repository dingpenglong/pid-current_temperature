#ifndef __ADS1220_CONF_H__
#define __ADS1220_CONF_H__
/*
@filename   ads1220_conf.h

@brief		����SPI��ADS1220��������ͷ�ļ�����Ҫ֧��HAL��

@time		2024/08/25

@author		������

@version    1.0

@attention  ʹ�ñ�����ʱ������ʵ�ֱ��ļ����г��ĺ꺯��

*/
#ifdef __cplusplus
extern "C" {
#endif

//����ϵͳͷ�ļ�
#include <stdint.h>
#include "delay.h"

//��ӡ������Ϣ��־����
#define ADS1220_PRINT_DEBUE_INFO 1

//�����ṩ��ʱ1ms�ĺ���,�Թ������ź�ʱ��
#define ADS1220_DELAY_1MS do{delay_ms(1);}while(0)
//#define ADS1220_DELAY_1MS do{HAL_Delay(1);}while(0)



#ifdef __cplusplus
}
#endif

#endif /* __ADS1220_CONF_H__ */