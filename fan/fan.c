#include "fan.h"
#include "fan_conf.h"




//打印调试信息
#ifdef FAN_PRINT_DEBUE_INFO 
#define FAN_PRINT_DEBUG(fmt,args...) do{printf("file:%s(%d) func %s:\n", __FILE__,__LINE__,  __FUNCTION__);printf(fmt, ##args);}while(0)
#else
#define FAN_PRINT_DEBUG(fmt,args...) 
#endif
//检查指针非空
#define CHECK_PTR(ptr, param, field) do{\
                                    if(ptr == NULL) \
                                    { \
                                        param.field = 1;\
										FAN_PRINT_DEBUG("pointer %s is NULL.\n", #ptr);\
                                        return param;\
                                    } \
                                }while(0)

                                
/*（1）底层硬件控制*/
//电源控制信号                          
#define  EnablePwr(dev) do{HAL_GPIO_WritePin(dev->pwr_GPIO, dev->pwr_BIT, 1);} while(0) 
#define  DisablePwr(dev) do{HAL_GPIO_WritePin(dev->pwr_GPIO, dev->pwr_BIT, 0);} while(0)


//PWM定时器信号
#define  EnablePwm(dev) do{HAL_TIM_PWM_Start(dev->pwm_TIM,dev->pwm_Channel);} while(0) 
#define  DisablePwm(dev) do{HAL_TIM_PWM_Stop(dev->pwm_TIM,dev->pwm_Channel);} while(0)




/*（2）风扇应用层函数*/

/*初始化*/

static FAN_Error_t Fan_Init(Adjustable_Fan_t* dev,  const GPIO_TypeDef* pwr_GPIO,const uint16_t pwr_BIT,   const TIM_HandleTypeDef*  pwm_TIM, const uint32_t pwm_Channel, 
    const TIM_HandleTypeDef* fg_TIM,const uint16_t fg_Channel){
       
    FAN_Error_t error;
    error.data=0;

    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
        
    //绑定硬件接口
    //电源信号
    dev->pwr_GPIO = (GPIO_TypeDef*)pwr_GPIO;  //电源信号所属GPIO
    dev->pwr_BIT  = pwr_BIT & 0XFFFF;         //电源信号的位号

    // PWM信号
    dev->pwm_TIM = (TIM_HandleTypeDef*)pwm_TIM; // PWM信号所属定时器
    dev->pwm_Channel = pwm_Channel & 0XFFFF;    // PWM信号的通道

    // FG信号
    dev->fg_TIM = (TIM_HandleTypeDef*)fg_TIM;    // FG信号所属定时器
    dev->fg_Channel = fg_Channel & 0xFFFF;            // FG信号的通道
    
    
    //初始化风扇结构体状态
   dev->fan_info = FAN_MALLOC(FAN_Info_t);
   dev->fan_info->pwm_sate=0;
   dev->fan_info->pwr_state=0;
   dev->fan_info->speed_fg=0;
   dev->fan_info->speed_pwm=0;
   
    
    return error;
}
 /*反初始化*/
static FAN_Error_t Fan_DeInit(Adjustable_Fan_t* dev){
       
    FAN_Error_t error;
    error.data=0;

    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
        
    //解邦硬件接口
    //电源信号
    dev->pwr_GPIO = NULL;  //电源信号所属GPIO
    dev->pwr_BIT  = NULL;         //电源信号的位号

    // PWM信号
    dev->pwm_TIM = NULL; // PWM信号所属定时器
    dev->pwm_Channel = NULL;    // PWM信号的通道

    // FG信号
    dev->fg_TIM = NULL;    // FG信号所属定时器
    dev->fg_Channel = NULL;            // FG信号的通道
    
    
    //初始化风扇结构体状态
    dev->fan_info=NULL;
    
    return error;
}

/*风扇开启函数*/
static FAN_Error_t Fan_On(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data=0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
    //检查电源GPIO引脚是否绑定
    CHECK_PTR(dev->pwr_GPIO,error,hardware);
    
    //检查电源PIN是否绑定
    CHECK_PTR(dev->pwr_BIT,error,hardware);

    dev->fan_info->pwr_state = 1;
    EnablePwr(dev);

    return error;
}
/*风扇关闭函数*/
static FAN_Error_t Fan_Off(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data=0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
    //检查电源GPIO引脚是否绑定
    CHECK_PTR(dev->pwr_GPIO,error,hardware);
    
    //检查电源PIN是否绑定
    CHECK_PTR(dev->pwr_BIT,error,hardware);

    dev->fan_info->pwr_state = 1;
    DisablePwr(dev);

    return error;
}
/*PWM开启函数*/
static FAN_Error_t Fan_Pwm_On(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data=0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
    //检查PWM定时器是否绑定
    CHECK_PTR(dev->pwm_TIM,error,hardware);

    dev->fan_info->pwm_sate = 1;
    EnablePwm(dev);

    return error;
}
/*PWM关闭函数*/
static FAN_Error_t Fan_Pwm_Off(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data=0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
    //检查PWM定时器是否绑定
    CHECK_PTR(dev->pwm_TIM,error,hardware);

    dev->fan_info->pwm_sate = 1;
    DisablePwm(dev);

    return error;
}

/*PWM设置函数*/
static FAN_Error_t Fan_Set_Pwm(Adjustable_Fan_t* dev, float duty_cycle)
{
    FAN_Error_t error;
    error.data=0;
        
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
     //检查PWM定时器是否绑定
    CHECK_PTR(dev->pwm_TIM,error,hardware);
    
    //计算pwm波的频率
    dev->fan_info->speed_pwm = (float)(170000000)/((dev->pwm_TIM->Init.Prescaler+1)*(dev->pwm_TIM->Init.Period+1));
    
    
    //PWM波频率大于风扇最大频率
    if(dev->fan_info->speed_pwm >FAN_MAX_PWMSPEED)
    {      
        error.set=1;
        return error;
    }
    
    //计算CRR得到占空比
    uint16_t Comparex =(uint16_t)((duty_cycle)*(dev->pwm_TIM->Init.Period+1));
    __HAL_TIM_SetCompare(dev->pwm_TIM, dev->pwm_Channel, Comparex);    //修改比较值，修改占空比
    return error;
}
/*开启FG信号捕获*/
static FAN_Error_t Fan_Fg_On(Adjustable_Fan_t* dev){
    FAN_Error_t error;
    
    error.data=0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    //开启计数器
    HAL_TIM_Base_Start(dev->fg_TIM);
    //以中断方式开启捕获
    HAL_TIM_IC_Start_IT(dev->fg_TIM, dev->fg_Channel);   

    
    return error;
}
/*关闭FG信号捕获*/
static FAN_Error_t Fan_Fg_Off(Adjustable_Fan_t* dev){
    FAN_Error_t error;
    
    error.data=0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
    //停止输入捕获中断
   HAL_TIM_IC_Stop_IT(dev->fg_TIM, dev->fg_Channel);
    
    
    //停止定时器
   HAL_TIM_Base_Start(dev->fg_TIM);
    
    //fg_speed清零
    dev->fan_info->speed_fg =0;
    return error;
}


/*FG信号捕取*/
static FAN_Error_t Fan_Get_Fg(Adjustable_Fan_t* dev){
    FAN_Error_t error;
    
    error.data=0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
        
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
    
    uint32_t uwIC2Value;
    float  frequency;
    //获取计数器的值
    uwIC2Value = HAL_TIM_ReadCapturedValue(dev->fg_TIM, dev->fg_Channel);
    
    //清空计数器
     __HAL_TIM_SET_COUNTER(dev->fg_TIM, 0);
    
          if(uwIC2Value==0)
          {
              uwIC2Value =1;
          }
          //计算频率
         frequency = 1000000/(uwIC2Value+65535*dev->fan_info->fg_flag);
         dev->fan_info->fg_flag=0;
          //计算转速
          dev->fan_info->speed_fg = 30*frequency;
    return error;
}
 





/*
	（3） 给初始化FAN函数接口
*/
FAN_Error_t FAN_API_INIT(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data = 0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    //绑定函数接口
	dev->Init           =   Fan_Init ;
    dev->DeInit         =   Fan_DeInit;
    dev->Pwr_On         =   Fan_On;
    dev->Pwr_Off        =   Fan_Off;
    dev->Pwm_On         =   Fan_Pwm_On;
    dev->Pwm_Off        =   Fan_Pwm_Off;
    dev->Set_Pwm        =   Fan_Set_Pwm;
    dev->Fg_On          =   Fan_Fg_On;
    dev->Fg_Off         =   Fan_Fg_Off;
    dev->Get_Fg         =   Fan_Get_Fg;
    return error;
}	

