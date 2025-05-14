#ifndef __DRV8412_H__
#define __DRV8412_H__
/*
@filename   drv8412.h

@brief		TEC˫��12V,5A��������  PWMģʽ����

@time		2024/12/29

@author		������

@version    1.0

@attention   ʹ�ò��裺
            step1:������������ʼ����API����
            step2�����õ�·��drv8412�Ĺ����ѹ----SUPPLY_VOLTAGE
            step3:ʹ�ÿ��������õȺ���
            
*/


#ifdef __cplusplus

extern "C" {
#endif

//����ϵͳͷ�ļ�
#include "stm32g4xx_hal.h"
#include "stdbool.h"
#include "delay.h"
#include  "math.h"
//DRV8412��12v�Ĺ����ѹ
#define SUPPLY_VOLTAGE 12.0
/*
    �������drv8412ʱ�Ĵ�������
*/
typedef union _DRV8412_Error
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
}DRV8412_Error_t;   

//drv8412��������ʱ������
typedef enum{
  pwma,
  pwmb
}pwm_t;

//����DRV8412�豸������
typedef struct DRV8412 DRV8412_t;
struct DRV8412
{

    
    //nsleep�ź�����
    GPIO_TypeDef*   nsleep_GPIO;    //��Դ�ź�����GPIO
    uint16_t        nsleep_BIT;     //��Դ�źŵ�λ��
    

    
    
    //PWMA�ź�����
    TIM_HandleTypeDef*   pwma_TIM;    //PWM�ź������Ķ�ʱ��
    uint32_t             pwma_Channel;     //PWMA�źŵ�ͨ��
    
        
    //PWM�ź�����
    TIM_HandleTypeDef*   pwmb_TIM;    //PWM�ź������Ķ�ʱ��
    uint32_t             pwmb_Channel;     //PWMB�źŵ�ͨ��

    //��ʼ��
    DRV8412_Error_t (*Init)(DRV8412_t* dev,  const GPIO_TypeDef* nsleep_GPIO, const uint16_t nsleep_BIT, TIM_HandleTypeDef*   pwma_TIM, uint32_t   pwma_Channel,
    TIM_HandleTypeDef*   pwmb_TIM, uint32_t   pwmb_Channel);
    //����ʼ��
    DRV8412_Error_t (*DeInit)(DRV8412_t* dev);
     //����
    DRV8412_Error_t (*On)(DRV8412_t* dev);
    //�ر�
    DRV8412_Error_t (*Off)(DRV8412_t* dev);
    //����PWMA��PWMB 
    DRV8412_Error_t (*Pwm_On)(DRV8412_t* dev,pwm_t pwm);
    //�ر�PWM
    DRV8412_Error_t (*Pwm_Off)(DRV8412_t* dev,pwm_t pwm);
    //����PWMռ�ձ�
    DRV8412_Error_t (*Set_Pwm)(DRV8412_t* dev,pwm_t pwm,double duty_cycle);
    //�����ѹ���ú���
    DRV8412_Error_t (*Set_Voltage)(DRV8412_t* dev,double voltage);
   
};



//�������û���ʼ�������ӿ�
DRV8412_Error_t DRV8412_API_INIT(DRV8412_t *dev);

 #ifdef __cplusplus
}
#endif  
    
#endif /* __DRV8412_H__ */
