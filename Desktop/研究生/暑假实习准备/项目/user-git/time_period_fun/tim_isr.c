#include "tim.h"
#include "time_period_fun.h"

extern TPS546d24A_t tps546d24a;
extern ADS1220_t  ads1220;
extern DRV8412_t drv8412;
extern Adjustable_Fan_t fan1;
extern Adjustable_Fan_t fan2;
extern ntc_t ntc;
extern Pid_System_t pid_system1;
extern Pid_System_t pid_system2;

/*��ʱ������ж��¼�*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

    if(htim == (&htim8)){
        
        if(htim->Channel == TIM_CHANNEL_1){
        //����1������1;
        fan1.fan_info->fg_flag +=1;
        }
        else if(htim->Channel == TIM_CHANNEL_2){
            //����2������1;
            fan2.fan_info->fg_flag +=1;
        }
        
    }
    else if(htim == (&htim3)){
       Pump_Driver(&tps546d24a,&ads1220,&drv8412,&pid_system1, &pid_system2,&ntc);
    }

}

//�����жϻص�����
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

  if(htim == (&htim8)) 
  {
      if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
      {
         fan1.Get_Fg(&fan1);
       }
       else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
      {
         fan2.Get_Fg(&fan2);
       }
   }
  
}


