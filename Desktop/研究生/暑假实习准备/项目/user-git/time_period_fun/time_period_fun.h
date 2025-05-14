/*
@filename	:	time_period_fun.h
@brief		:	定时器调用头文件，用于编写定时器每次需要执行的操作
@time		:	 2024/12/3
@author		: 	丁鹏龙

@version    :    V1.0
*/

// <<< Use Configuration Wizard in Context Menu >>>

// <<< end of configuration section >>>


#ifndef __TIME_PERIOD_FUN__
#define __TIME_PERIOD_FUN__ 

#include "tps546d24a.h"
#include "ads1220.h"
#include "drv8412.h"
#include "fan.h"
#include "pid.h"
#include "math.h"
#include "protocol.h"
#include "ntc.h"
#include  "stdio.h"
#ifdef __cplusplus
extern "C" {
#endif


void  Pump_Driver(TPS546d24A_t* tps546d24a,ADS1220_t* ads1220,DRV8412_t* drv8412,Pid_System_t* pid_system1,Pid_System_t* pid_system2,ntc_t* ntc);






#ifdef __cplusplus
}
#endif




#endif  /* end of __time_period_fun.h__ */	
