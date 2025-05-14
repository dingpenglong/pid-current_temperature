#ifndef __DRV8412_H__
#define __DRV8412_H__
/*
@filename   drv8412.h

@brief		TEC双向12V,5A电流驱动  PWM模式控制

@time		2024/12/29

@author		丁鹏龙

@version    1.0

@attention   使用步骤：
            step1:创建变量、初始化、API申请
            step2：设置电路的drv8412的供电电压----SUPPLY_VOLTAGE
            step3:使用开启、设置等函数
            
*/


#ifdef __cplusplus

extern "C" {
#endif

//引入系统头文件
#include "stm32g4xx_hal.h"
#include "stdbool.h"
#include "delay.h"
#include  "math.h"
//DRV8412是12v的供电电压
#define SUPPLY_VOLTAGE 12.0
/*
    定义操作drv8412时的错误类型
*/
typedef union _DRV8412_Error
{
    struct
    {
        uint8_t dev     : 1; //设备不存在
        uint8_t malloc  : 1; //申请动态空间失败
        uint8_t set     : 1; //参数设置有误
        uint8_t timeout : 1; //操作超时
       uint8_t hardware : 1; //硬件接口无效
    };
    uint8_t data;
}DRV8412_Error_t;   

//drv8412的两个定时器输入
typedef enum{
  pwma,
  pwmb
}pwm_t;

//定义DRV8412设备描述符
typedef struct DRV8412 DRV8412_t;
struct DRV8412
{

    
    //nsleep信号描述
    GPIO_TypeDef*   nsleep_GPIO;    //电源信号所属GPIO
    uint16_t        nsleep_BIT;     //电源信号的位号
    

    
    
    //PWMA信号描述
    TIM_HandleTypeDef*   pwma_TIM;    //PWM信号所属的定时器
    uint32_t             pwma_Channel;     //PWMA信号的通道
    
        
    //PWM信号描述
    TIM_HandleTypeDef*   pwmb_TIM;    //PWM信号所属的定时器
    uint32_t             pwmb_Channel;     //PWMB信号的通道

    //初始化
    DRV8412_Error_t (*Init)(DRV8412_t* dev,  const GPIO_TypeDef* nsleep_GPIO, const uint16_t nsleep_BIT, TIM_HandleTypeDef*   pwma_TIM, uint32_t   pwma_Channel,
    TIM_HandleTypeDef*   pwmb_TIM, uint32_t   pwmb_Channel);
    //反初始化
    DRV8412_Error_t (*DeInit)(DRV8412_t* dev);
     //开启
    DRV8412_Error_t (*On)(DRV8412_t* dev);
    //关闭
    DRV8412_Error_t (*Off)(DRV8412_t* dev);
    //开启PWMA和PWMB 
    DRV8412_Error_t (*Pwm_On)(DRV8412_t* dev,pwm_t pwm);
    //关闭PWM
    DRV8412_Error_t (*Pwm_Off)(DRV8412_t* dev,pwm_t pwm);
    //设置PWM占空比
    DRV8412_Error_t (*Set_Pwm)(DRV8412_t* dev,pwm_t pwm,double duty_cycle);
    //输出电压设置函数
    DRV8412_Error_t (*Set_Voltage)(DRV8412_t* dev,double voltage);
   
};



//公开给用户初始化函数接口
DRV8412_Error_t DRV8412_API_INIT(DRV8412_t *dev);

 #ifdef __cplusplus
}
#endif  
    
#endif /* __DRV8412_H__ */
