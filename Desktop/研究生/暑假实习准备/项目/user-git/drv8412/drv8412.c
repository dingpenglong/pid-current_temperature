#include "drv8412.h"
#include "drv8412_conf.h"


//��ӡ������Ϣ
#ifdef DRV8412_PRINT_DEBUE_INFO 
#define DRV8412_PRINT_DEBUG(fmt,args...) do{printf("file:%s(%d) func %s:\n", __FILE__,__LINE__,  __FUNCTION__);printf(fmt, ##args);}while(0)
#else
#define DRV8412_PRINT_DEBUG(fmt,args...) 
#endif
//���ָ��ǿ�
#define CHECK_PTR(ptr, param, field) do{\
                                    if(ptr == NULL) \
                                    { \
                                        param.field = 1;\
										DRV8412_PRINT_DEBUG("pointer %s is NULL.\n", #ptr);\
                                        return param;\
                                    } \
                                }while(0)

//nSLEEP�����ź�                          
#define  EnableNsleep(dev) do{HAL_GPIO_WritePin(dev->nsleep_GPIO, dev->nsleep_BIT, 0);delay_us(1);} while(0) 
#define  DisableNsleep(dev) do{HAL_GPIO_WritePin(dev->nsleep_GPIO, dev->nsleep_BIT, 1);delay_us(1);} while(0)

//PWMA�ź�
#define  EnablePwma(dev) do{HAL_TIM_PWM_Start(dev->pwma_TIM,dev->pwma_Channel);delay_us(1);} while(0)   
#define  DisablePwma(dev) do{HAL_TIM_PWM_Start(dev->pwma_TIM,dev->pwma_Channel);delay_us(1);} while(0)  

//PWMB�ź�
#define  EnablePwmb(dev) do{HAL_TIM_PWM_Start(dev->pwmb_TIM,dev->pwmb_Channel);delay_us(1);} while(0) 
#define  DisablePwmb(dev) do{HAL_TIM_PWM_Stop(dev->pwmb_TIM,dev->pwmb_Channel);delay_us(1);} while(0)


/*��ʼ��*/

static DRV8412_Error_t Drv8412_Init(DRV8412_t* dev,  const GPIO_TypeDef* nsleep_GPIO, const uint16_t nsleep_BIT, TIM_HandleTypeDef*   pwma_TIM, uint32_t   pwma_Channel,
    TIM_HandleTypeDef*   pwmb_TIM, uint32_t   pwmb_Channel )
{
       
    DRV8412_Error_t error;
    error.data=0;
    
    
    //��Ӳ���ӿ�
    //nSleep�ź�
    dev->nsleep_GPIO = (GPIO_TypeDef*)nsleep_GPIO;  //nSleep�ź�����GPIO
    dev->nsleep_BIT  = nsleep_BIT & 0XFFFF;         //nSleep�źŵ�λ��
   

    // PWMA�ź�
    dev->pwma_TIM = (TIM_HandleTypeDef*)pwma_TIM; // PWMA�ź�������ʱ��
    dev->pwma_Channel = pwma_Channel & 0XFFFF;    // PWMA�źŵ�ͨ��
    
    //PWMB�ź�
    dev->pwmb_TIM = (TIM_HandleTypeDef*)pwma_TIM; // PWMB�ź�������ʱ��
    dev->pwmb_Channel = pwmb_Channel & 0XFFFF;    // PWMB�źŵ�ͨ��
    return error;
}

/*����ʼ��*/
static DRV8412_Error_t Drv8412_DeInit(DRV8412_t* dev){
    DRV8412_Error_t error;
    error.data=0;
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //���Ӳ���ӿ�
    //nSleep�ź�
    dev->nsleep_GPIO = NULL;
    dev->nsleep_BIT  = NULL;
;
    
    //PMWA�ź�
    dev->pwma_TIM=NULL;
    dev->pwma_Channel=NULL;
    
        
    //PMWB�ź�
    dev->pwmb_TIM=NULL;
    dev->pwmb_Channel=NULL;
    
    return error;
}

/*��Դ��������*/
static DRV8412_Error_t Drv8412_On(DRV8412_t* dev)
{
    DRV8412_Error_t error;
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //���nsleep�Ƿ��
    CHECK_PTR(dev->nsleep_GPIO,error,hardware);
    
    CHECK_PTR(dev->nsleep_BIT,error,hardware);
    
    DisableNsleep(dev);
    return error;
}

/*��Դ�رպ���*/
static DRV8412_Error_t Drv8412_Off(DRV8412_t* dev)
{
    DRV8412_Error_t error;
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //���nsleep�Ƿ��
    CHECK_PTR(dev->nsleep_GPIO,error,hardware);
    
    CHECK_PTR(dev->nsleep_BIT,error,hardware);
    
    EnableNsleep(dev);
    return error;
}

/*PWM���Ų�������*/
static DRV8412_Error_t Drv8412_Pwm_On(DRV8412_t* dev,pwm_t pwm)
{
    DRV8412_Error_t error;
    error.data =0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //���PWM��ʱ���Ƿ��
    CHECK_PTR(dev->pwma_TIM,error,hardware);
    CHECK_PTR(dev->pwmb_TIM,error,hardware);

    switch(pwm)
    {
    case pwma: 
        EnablePwma(dev);
        break;
    case pwmb: 
        EnablePwmb(dev);
        break;
    }
    return error;
}


/*PWM���ú���*/
static DRV8412_Error_t Drv8412_Set_Pwm(DRV8412_t* dev,pwm_t pwm, double duty_cycle)
{
    DRV8412_Error_t error;
    error.data=0;
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    //���PWM��ʱ���Ƿ��
    CHECK_PTR(dev->pwma_TIM,error,hardware);
    CHECK_PTR(dev->pwmb_TIM,error,hardware);
    //�ж�ռ�ձȵ�ֵ�Ƿ���ȷ
    if(duty_cycle <= 0 || duty_cycle >= 100){
        error.set =1;
        return error;
    }

    uint16_t Comparex = (uint16_t)(duty_cycle * (dev->pwma_TIM->Init.Period + 1) / 100);
    switch (pwm)
    {
    case pwma:
        __HAL_TIM_SetCompare(dev->pwma_TIM, dev->pwma_Channel, Comparex);    //�޸ıȽ�ֵ���޸�ռ�ձ�
        break;
    
   case pwmb:
        __HAL_TIM_SetCompare(dev->pwma_TIM, dev->pwmb_Channel, Comparex);    //�޸ıȽ�ֵ���޸�ռ�ձ�
        break;
    }
    
    return error;
}


static DRV8412_Error_t Drv8412_Pwm_Off(DRV8412_t* dev,pwm_t pwm)
{
    DRV8412_Error_t error;
    error.data =0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //���PWM��ʱ���Ƿ��
    CHECK_PTR(dev->pwma_TIM,error,hardware);
    CHECK_PTR(dev->pwmb_TIM,error,hardware);
        switch(pwm)
    {
    case pwma: 
        Drv8412_Set_Pwm(dev,pwma,0.01);
        DisablePwma(dev);
        break;
    case pwmb: 
        Drv8412_Set_Pwm(dev,pwmb,0.01);
        DisablePwmb(dev);
        break;
    }
    return error;
}

/*�����ѹ���ú���*/
static DRV8412_Error_t Drv8412_Set_Voltage(DRV8412_t* dev,double voltage){
    DRV8412_Error_t error;
    error.data=0;
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);

    //�ж����ù����ѹ�Ƿ���ȷ
    if(SUPPLY_VOLTAGE <= 0 || SUPPLY_VOLTAGE >52)
    {
        error.set =1;
        return error;
    }
    double supply_vlotage = SUPPLY_VOLTAGE;
    double duty_cycle = (fabs(voltage)/supply_vlotage)*100;
    if(voltage >0)
    {
        Drv8412_Set_Pwm(dev,pwma,duty_cycle);
        Drv8412_Set_Pwm(dev,pwmb,0);
    }else 
    {    
        Drv8412_Set_Pwm(dev,pwma,duty_cycle);
        Drv8412_Set_Pwm(dev,pwmb,100);
         
    }

    return error;
}


/*
	 ����ʼ��DRV8412�����ӿ�
*/
DRV8412_Error_t DRV8412_API_INIT(DRV8412_t* dev)
{
    DRV8412_Error_t error;
    error.data = 0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
	
    //�󶨺����ӿ�
	dev->Init           =   Drv8412_Init ;
    dev->DeInit         =   Drv8412_DeInit;
    dev->On             =   Drv8412_On;
    dev->Off            =   Drv8412_Off;
    dev->Pwm_On        =   Drv8412_Pwm_On;
    dev->Pwm_Off       =   Drv8412_Pwm_Off;
    dev->Set_Pwm       =   Drv8412_Set_Pwm;
    dev->Set_Voltage   =   Drv8412_Set_Voltage;
    return error;
}	

