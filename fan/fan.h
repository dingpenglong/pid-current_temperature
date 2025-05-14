#ifndef __FAN_H__
#define __FAN_H__
/*
@filename   fan.h

@brief		�ɵ��ٵ�PWM���ȿ���
            PWMƵ��Ϊ25KHZ
@time		2024/11/25

@author		������

@version    1.0

@attention  
            
*/


#ifdef __cplusplus

extern "C" {
#endif

//����ϵͳͷ�ļ�
#include "stm32g4xx_hal.h"
#include "stdbool.h"



#define FAN_MAX_PWMSPEED 25000  //�������PWM��Ƶ��



typedef struct _FAN_Info
{ 
    bool pwr_state;    //����״̬ 1���� 0�ر�
        
    float speed_pwm; //ת���ź�PWM
    bool  pwm_sate; //�Ƿ�����PWM

    float speed_fg;   //ת���ź�
    uint8_t fg_flag; 

}FAN_Info_t;


/*
    �������FANʱ�Ĵ�������
*/
typedef union _FAN_Error
{
    struct
    {
        uint8_t dev     : 1; //�豸������
        uint8_t malloc  : 1; //���붯̬�ռ�ʧ��
        uint8_t set     : 1; //������������
        uint8_t timeout : 1; //������ʱ
        uint8_t hardware : 1; //Ӳ���ӿ���Ч
    };
    uint8_t data;
}FAN_Error_t;   


//����Fan�豸������
typedef struct Adjustable_Fan Adjustable_Fan_t;
struct Adjustable_Fan
{
    FAN_Info_t*  fan_info;  //���÷��Ȼ�������
    
    //��Դ�ź�����
    GPIO_TypeDef*   pwr_GPIO;    //��Դ�ź�����GPIO
    uint16_t        pwr_BIT;     //��Դ�źŵ�λ��
    
    
    //PWM�ź�����
    TIM_HandleTypeDef*   pwm_TIM;    //PWM�ź������Ķ�ʱ��
    uint32_t             pwm_Channel;     //PWM�źŵ�ͨ��
    
    //FG�ź�����
   TIM_HandleTypeDef*       fg_TIM;    //FG�ź�����GPIO
   uint32_t                 fg_Channel;     //FG�źŵ�λ��
    
    
    //��ʼ������
    FAN_Error_t (*Init)(Adjustable_Fan_t* dev,  const GPIO_TypeDef* pwr_GPIO,const uint16_t pwr_BIT,   const TIM_HandleTypeDef*  pwm_TIM, const uint32_t pwm_Channel, 
    const TIM_HandleTypeDef* fg_TIM,const uint16_t fg_Channel);  
    
    //����ʼ������
     FAN_Error_t (*DeInit)(Adjustable_Fan_t* dev);
    
    //��Դ��������
    FAN_Error_t (*Pwr_On)(Adjustable_Fan_t* dev);
    
    //��Դ�رպ���
    FAN_Error_t (*Pwr_Off)(Adjustable_Fan_t* dev);
    
    //PWM��������
    FAN_Error_t (*Pwm_On)(Adjustable_Fan_t* dev);
    
    //PWM�رպ���
    FAN_Error_t (*Pwm_Off)(Adjustable_Fan_t* dev);
    
    //PWMռ�ձ����ú���
    FAN_Error_t (*Set_Pwm)(Adjustable_Fan_t* dev, float duty_cycle);
    
    //����FG��������
    FAN_Error_t (*Fg_On)(Adjustable_Fan_t* dev);
    
    //�ر�FG��������
    FAN_Error_t (*Fg_Off)(Adjustable_Fan_t* dev);
    
    //��ȡ����fg�����źź���
    FAN_Error_t (*Get_Fg)(Adjustable_Fan_t* dev);
   
};




//�������û���ʼ�������ӿ�
FAN_Error_t FAN_API_INIT(Adjustable_Fan_t *dev);



 #ifdef __cplusplus
}
#endif  
    
#endif /* __FAN_H__ */

