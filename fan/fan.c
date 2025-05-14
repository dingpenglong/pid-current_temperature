#include "fan.h"
#include "fan_conf.h"




//��ӡ������Ϣ
#ifdef FAN_PRINT_DEBUE_INFO 
#define FAN_PRINT_DEBUG(fmt,args...) do{printf("file:%s(%d) func %s:\n", __FILE__,__LINE__,  __FUNCTION__);printf(fmt, ##args);}while(0)
#else
#define FAN_PRINT_DEBUG(fmt,args...) 
#endif
//���ָ��ǿ�
#define CHECK_PTR(ptr, param, field) do{\
                                    if(ptr == NULL) \
                                    { \
                                        param.field = 1;\
										FAN_PRINT_DEBUG("pointer %s is NULL.\n", #ptr);\
                                        return param;\
                                    } \
                                }while(0)

                                
/*��1���ײ�Ӳ������*/
//��Դ�����ź�                          
#define  EnablePwr(dev) do{HAL_GPIO_WritePin(dev->pwr_GPIO, dev->pwr_BIT, 1);} while(0) 
#define  DisablePwr(dev) do{HAL_GPIO_WritePin(dev->pwr_GPIO, dev->pwr_BIT, 0);} while(0)


//PWM��ʱ���ź�
#define  EnablePwm(dev) do{HAL_TIM_PWM_Start(dev->pwm_TIM,dev->pwm_Channel);} while(0) 
#define  DisablePwm(dev) do{HAL_TIM_PWM_Stop(dev->pwm_TIM,dev->pwm_Channel);} while(0)




/*��2������Ӧ�ò㺯��*/

/*��ʼ��*/

static FAN_Error_t Fan_Init(Adjustable_Fan_t* dev,  const GPIO_TypeDef* pwr_GPIO,const uint16_t pwr_BIT,   const TIM_HandleTypeDef*  pwm_TIM, const uint32_t pwm_Channel, 
    const TIM_HandleTypeDef* fg_TIM,const uint16_t fg_Channel){
       
    FAN_Error_t error;
    error.data=0;

    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
        
    //��Ӳ���ӿ�
    //��Դ�ź�
    dev->pwr_GPIO = (GPIO_TypeDef*)pwr_GPIO;  //��Դ�ź�����GPIO
    dev->pwr_BIT  = pwr_BIT & 0XFFFF;         //��Դ�źŵ�λ��

    // PWM�ź�
    dev->pwm_TIM = (TIM_HandleTypeDef*)pwm_TIM; // PWM�ź�������ʱ��
    dev->pwm_Channel = pwm_Channel & 0XFFFF;    // PWM�źŵ�ͨ��

    // FG�ź�
    dev->fg_TIM = (TIM_HandleTypeDef*)fg_TIM;    // FG�ź�������ʱ��
    dev->fg_Channel = fg_Channel & 0xFFFF;            // FG�źŵ�ͨ��
    
    
    //��ʼ�����Ƚṹ��״̬
   dev->fan_info = FAN_MALLOC(FAN_Info_t);
   dev->fan_info->pwm_sate=0;
   dev->fan_info->pwr_state=0;
   dev->fan_info->speed_fg=0;
   dev->fan_info->speed_pwm=0;
   
    
    return error;
}
 /*����ʼ��*/
static FAN_Error_t Fan_DeInit(Adjustable_Fan_t* dev){
       
    FAN_Error_t error;
    error.data=0;

    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
        
    //���Ӳ���ӿ�
    //��Դ�ź�
    dev->pwr_GPIO = NULL;  //��Դ�ź�����GPIO
    dev->pwr_BIT  = NULL;         //��Դ�źŵ�λ��

    // PWM�ź�
    dev->pwm_TIM = NULL; // PWM�ź�������ʱ��
    dev->pwm_Channel = NULL;    // PWM�źŵ�ͨ��

    // FG�ź�
    dev->fg_TIM = NULL;    // FG�ź�������ʱ��
    dev->fg_Channel = NULL;            // FG�źŵ�ͨ��
    
    
    //��ʼ�����Ƚṹ��״̬
    dev->fan_info=NULL;
    
    return error;
}

/*���ȿ�������*/
static FAN_Error_t Fan_On(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //����ԴGPIO�����Ƿ��
    CHECK_PTR(dev->pwr_GPIO,error,hardware);
    
    //����ԴPIN�Ƿ��
    CHECK_PTR(dev->pwr_BIT,error,hardware);

    dev->fan_info->pwr_state = 1;
    EnablePwr(dev);

    return error;
}
/*���ȹرպ���*/
static FAN_Error_t Fan_Off(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //����ԴGPIO�����Ƿ��
    CHECK_PTR(dev->pwr_GPIO,error,hardware);
    
    //����ԴPIN�Ƿ��
    CHECK_PTR(dev->pwr_BIT,error,hardware);

    dev->fan_info->pwr_state = 1;
    DisablePwr(dev);

    return error;
}
/*PWM��������*/
static FAN_Error_t Fan_Pwm_On(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //���PWM��ʱ���Ƿ��
    CHECK_PTR(dev->pwm_TIM,error,hardware);

    dev->fan_info->pwm_sate = 1;
    EnablePwm(dev);

    return error;
}
/*PWM�رպ���*/
static FAN_Error_t Fan_Pwm_Off(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //���PWM��ʱ���Ƿ��
    CHECK_PTR(dev->pwm_TIM,error,hardware);

    dev->fan_info->pwm_sate = 1;
    DisablePwm(dev);

    return error;
}

/*PWM���ú���*/
static FAN_Error_t Fan_Set_Pwm(Adjustable_Fan_t* dev, float duty_cycle)
{
    FAN_Error_t error;
    error.data=0;
        
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
     //���PWM��ʱ���Ƿ��
    CHECK_PTR(dev->pwm_TIM,error,hardware);
    
    //����pwm����Ƶ��
    dev->fan_info->speed_pwm = (float)(170000000)/((dev->pwm_TIM->Init.Prescaler+1)*(dev->pwm_TIM->Init.Period+1));
    
    
    //PWM��Ƶ�ʴ��ڷ������Ƶ��
    if(dev->fan_info->speed_pwm >FAN_MAX_PWMSPEED)
    {      
        error.set=1;
        return error;
    }
    
    //����CRR�õ�ռ�ձ�
    uint16_t Comparex =(uint16_t)((duty_cycle)*(dev->pwm_TIM->Init.Period+1));
    __HAL_TIM_SetCompare(dev->pwm_TIM, dev->pwm_Channel, Comparex);    //�޸ıȽ�ֵ���޸�ռ�ձ�
    return error;
}
/*����FG�źŲ���*/
static FAN_Error_t Fan_Fg_On(Adjustable_Fan_t* dev){
    FAN_Error_t error;
    
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    //����������
    HAL_TIM_Base_Start(dev->fg_TIM);
    //���жϷ�ʽ��������
    HAL_TIM_IC_Start_IT(dev->fg_TIM, dev->fg_Channel);   

    
    return error;
}
/*�ر�FG�źŲ���*/
static FAN_Error_t Fan_Fg_Off(Adjustable_Fan_t* dev){
    FAN_Error_t error;
    
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    //ֹͣ���벶���ж�
   HAL_TIM_IC_Stop_IT(dev->fg_TIM, dev->fg_Channel);
    
    
    //ֹͣ��ʱ��
   HAL_TIM_Base_Start(dev->fg_TIM);
    
    //fg_speed����
    dev->fan_info->speed_fg =0;
    return error;
}


/*FG�źŲ�ȡ*/
static FAN_Error_t Fan_Get_Fg(Adjustable_Fan_t* dev){
    FAN_Error_t error;
    
    error.data=0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
        
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    
    
    uint32_t uwIC2Value;
    float  frequency;
    //��ȡ��������ֵ
    uwIC2Value = HAL_TIM_ReadCapturedValue(dev->fg_TIM, dev->fg_Channel);
    
    //��ռ�����
     __HAL_TIM_SET_COUNTER(dev->fg_TIM, 0);
    
          if(uwIC2Value==0)
          {
              uwIC2Value =1;
          }
          //����Ƶ��
         frequency = 1000000/(uwIC2Value+65535*dev->fan_info->fg_flag);
         dev->fan_info->fg_flag=0;
          //����ת��
          dev->fan_info->speed_fg = 30*frequency;
    return error;
}
 





/*
	��3�� ����ʼ��FAN�����ӿ�
*/
FAN_Error_t FAN_API_INIT(Adjustable_Fan_t* dev)
{
    FAN_Error_t error;
    error.data = 0;
    
    //���豸�����ڣ�ֱ�ӷ���
    CHECK_PTR(dev, error, dev);
    //�󶨺����ӿ�
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

