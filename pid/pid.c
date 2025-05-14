#include "pid.h"
#include "math.h"
#include "protocol.h"


void kalman_init(KalmanFilter *kf, float Q, float R, float A, float B, float H, float P, float x_hat) {
    kf->Q = Q;
    kf->R = R;
    kf->A = A;
    kf->B = B;
    kf->H = H;
    kf->P = P;
    kf->x_hat = x_hat;
    kf->K = 0.0;
}
float kalman_update(KalmanFilter *kf, float z, float u) {
    // Ԥ�ⲽ��
    float x_hat_minus = kf->A * kf->x_hat + kf->B * u; // Ԥ��״̬
    float P_minus = kf->A * kf->P * kf->A + kf->Q;     // Ԥ�����Э����

    // ���²���
    kf->K = P_minus * kf->H / (kf->H * P_minus * kf->H + kf->R); // ����������
    kf->x_hat = x_hat_minus + kf->K * (z - kf->H * x_hat_minus); // ����״̬����
    kf->P = (1 - kf->K * kf->H) * P_minus;                      // �������Э����

    return kf->x_hat; // ���ع���ֵ
}
/*PID������ʼ��*/
static void Param_Init(Pid_System_t* dev,int Id,double max,double min,double target_val,Ratio ratio,double step)
{       
        
        
        // ��̬����ռ�
        dev->pid= (PID*)malloc(sizeof(PID));
		/* ��ʼ������ */
        dev->pid->user_target_val=0.0;
        dev->pid->ID  = Id;
		dev->pid->target_val=target_val;
        dev->pid->actual_val=0.0;
        dev->pid->output =0.0;
    	dev->pid->err_now=0.0;	
        dev->pid->err_last=0.0;
    	dev->pid->err_last_last=0.0;	
        dev->pid->Kp=0.0;
    	dev->pid->Ki=0.0;	
        dev->pid->Kd=0.0;
        dev->pid->out_max = max;
        dev->pid->out_min = min;
        dev->pid->ratio  =ratio;
        dev->Set_User_Ratio(dev,ratio);
        dev->pid->step =step;

}
/*�����û�Ŀ��ֵ*/
static void Set_User_Target(Pid_System_t* dev,double temp_val){

  dev->pid->target_val = temp_val;    
}

/*��ȡ�û�Ŀ��ֵ*/
static float Get_User_Target(Pid_System_t* dev)
{
  return dev->pid->user_target_val;   
}

/*����Ŀ��ֵ*/
static void Set_Target(Pid_System_t* dev)
{   
    float diff = dev->pid->target_val - dev->pid->user_target_val;
    //����ֵС�ڲ��� ��ֱ�ӵ���Ŀ��ֵ
    if(fabs(diff) <= dev->pid->step)
    {
        dev->pid->target_val = dev->pid->user_target_val;
    }

    else if(dev->pid->target_val < dev->pid->user_target_val)
    {
        dev->pid->target_val += (dev->pid->slope)*dev->pid->step; 
      
    }
    else if(dev->pid->target_val > dev->pid->user_target_val)
    {
        dev->pid->target_val -= (dev->pid->slope)*dev->pid->step;  

     }
 }

/*��ȡĿ��ֵ*/
static float Get_Target(Pid_System_t* dev)
{
  return dev->pid->target_val;   
}

/*���ñ��������֡�΢��ϵ�� */
static void Set_P_I_D(Pid_System_t* dev,double p, double i, double d)
{
		dev->pid->Kp = p;    
		dev->pid->Ki = i;    
		dev->pid->Kd = d;    
}


/*����ʽPID*/
static double Incremental_Realize(Pid_System_t* dev,double temp_val) 
{
	//�������
	dev->pid->err_now=dev->pid->target_val-temp_val;
    
    double proportion =dev->pid->Kp*dev->pid->err_now ;
    double  integral  = dev->pid->Ki*(dev->pid->err_now - dev->pid->err_last);
    double differential =dev->pid->Kd*(dev->pid->err_now - 2 * dev->pid->err_last + dev->pid->err_last_last);
    //PID����
	double increment_val = proportion + integral + differential;
	dev->pid->output += increment_val;
    
	dev->pid->err_last_last = dev->pid->err_last;
    dev->pid->err_last = dev->pid->err_now;
    
    //�޷����
    if(dev->pid->output < dev->pid->out_min)
    {dev->pid->output =dev->pid->out_min;}
    else if(dev->pid->output > dev->pid->out_max)
    {dev->pid->output =dev->pid->out_max;}
    
    
   
	return dev->pid->output;
}

/*����λ������PID����*/
static void  Send_Param(Pid_System_t* dev,UART_HandleTypeDef* huart,uint8_t channel)
{
        float pid_temp[3] = {dev->pid->Kp, dev->pid->Ki, dev->pid->Kd};
		set_computer_value(huart,SEND_P_I_D_CMD, channel, pid_temp, 3);     
}


/*���õ�λ*/
static void Set_User_Ratio(Pid_System_t* dev,Ratio ratio)
{   
    //���ݵ�λ����б��slope
    dev->pid->ratio =ratio;
    switch(dev->pid->ratio)
    {
        case Low:
                dev->pid->slope=2;    
                break;
        case Mideum:
                dev->pid->slope=4;
                break;
        case High :
                dev->pid->slope=8;
                break;
        default:
                dev->pid->slope=0;
                break;
        }

}



/*PID��API�ӿں���*/
void PID_API_INIT(Pid_System_t* dev)
{
    dev->Param_Init       = Param_Init;
    dev->Set_User_Target  =Set_User_Target;
    dev->Get_User_Target  =Get_User_Target;
    dev->Set_Target       = Set_Target;
    dev->Get_Target       = Get_Target;
    dev->Set_P_I_D        = Set_P_I_D;
    dev->Incremental_Realize       = Incremental_Realize;
    dev->Send_Param       = Send_Param;
    dev->Set_User_Ratio   = Set_User_Ratio;
}

