#ifndef __FAN_H__
#define __FAN_H__
/*
@filename   fan.h

@brief		可调速的PWM风扇控制
            PWM频率为25KHZ
@time		2024/11/25

@author		丁鹏龙

@version    1.0

@attention  
            
*/


#ifdef __cplusplus

extern "C" {
#endif

//引入系统头文件
#include "stm32g4xx_hal.h"
#include "stdbool.h"



#define FAN_MAX_PWMSPEED 25000  //风扇最大PWM波频率



typedef struct _FAN_Info
{ 
    bool pwr_state;    //风扇状态 1开启 0关闭
        
    float speed_pwm; //转速信号PWM
    bool  pwm_sate; //是否启动PWM

    float speed_fg;   //转速信号
    uint8_t fg_flag; 

}FAN_Info_t;


/*
    定义操作FAN时的错误类型
*/
typedef union _FAN_Error
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
}FAN_Error_t;   


//定义Fan设备描述符
typedef struct Adjustable_Fan Adjustable_Fan_t;
struct Adjustable_Fan
{
    FAN_Info_t*  fan_info;  //设置风扇基本配置
    
    //电源信号描述
    GPIO_TypeDef*   pwr_GPIO;    //电源信号所属GPIO
    uint16_t        pwr_BIT;     //电源信号的位号
    
    
    //PWM信号描述
    TIM_HandleTypeDef*   pwm_TIM;    //PWM信号所属的定时器
    uint32_t             pwm_Channel;     //PWM信号的通道
    
    //FG信号描述
   TIM_HandleTypeDef*       fg_TIM;    //FG信号所属GPIO
   uint32_t                 fg_Channel;     //FG信号的位号
    
    
    //初始化函数
    FAN_Error_t (*Init)(Adjustable_Fan_t* dev,  const GPIO_TypeDef* pwr_GPIO,const uint16_t pwr_BIT,   const TIM_HandleTypeDef*  pwm_TIM, const uint32_t pwm_Channel, 
    const TIM_HandleTypeDef* fg_TIM,const uint16_t fg_Channel);  
    
    //反初始化函数
     FAN_Error_t (*DeInit)(Adjustable_Fan_t* dev);
    
    //电源开启函数
    FAN_Error_t (*Pwr_On)(Adjustable_Fan_t* dev);
    
    //电源关闭函数
    FAN_Error_t (*Pwr_Off)(Adjustable_Fan_t* dev);
    
    //PWM开启函数
    FAN_Error_t (*Pwm_On)(Adjustable_Fan_t* dev);
    
    //PWM关闭函数
    FAN_Error_t (*Pwm_Off)(Adjustable_Fan_t* dev);
    
    //PWM占空比设置函数
    FAN_Error_t (*Set_Pwm)(Adjustable_Fan_t* dev, float duty_cycle);
    
    //开启FG捕获输入
    FAN_Error_t (*Fg_On)(Adjustable_Fan_t* dev);
    
    //关闭FG捕获输入
    FAN_Error_t (*Fg_Off)(Adjustable_Fan_t* dev);
    
    //获取风扇fg反馈信号函数
    FAN_Error_t (*Get_Fg)(Adjustable_Fan_t* dev);
   
};




//公开给用户初始化函数接口
FAN_Error_t FAN_API_INIT(Adjustable_Fan_t *dev);



 #ifdef __cplusplus
}
#endif  
    
#endif /* __FAN_H__ */

