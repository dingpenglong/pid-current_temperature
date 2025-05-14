#ifndef __ADS1220_CONF_H__
#define __ADS1220_CONF_H__
/*
@filename   ads1220_conf.h

@brief		基于SPI的ADS1220驱动配置头文件，需要支持HAL库

@time		2024/08/25

@author		丁鹏龙

@version    1.0

@attention  使用本驱动时，必须实现本文件所列出的宏函数

*/
#ifdef __cplusplus
extern "C" {
#endif

//引入系统头文件
#include <stdint.h>
#include "delay.h"

//打印调试信息日志开关
#define ADS1220_PRINT_DEBUE_INFO 1

//必须提供延时1ms的函数,以供满足信号时序
#define ADS1220_DELAY_1MS do{delay_ms(1);}while(0)
//#define ADS1220_DELAY_1MS do{HAL_Delay(1);}while(0)



#ifdef __cplusplus
}
#endif

#endif /* __ADS1220_CONF_H__ */