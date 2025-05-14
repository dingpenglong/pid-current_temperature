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
    // 预测步骤
    float x_hat_minus = kf->A * kf->x_hat + kf->B * u; // 预测状态
    float P_minus = kf->A * kf->P * kf->A + kf->Q;     // 预测误差协方差

    // 更新步骤
    kf->K = P_minus * kf->H / (kf->H * P_minus * kf->H + kf->R); // 卡尔曼增益
    kf->x_hat = x_hat_minus + kf->K * (z - kf->H * x_hat_minus); // 更新状态估计
    kf->P = (1 - kf->K * kf->H) * P_minus;                      // 更新误差协方差

    return kf->x_hat; // 返回估计值
}
/*PID参数初始化*/
static void Param_Init(Pid_System_t* dev,int Id,double max,double min,double target_val,Ratio ratio,double step)
{       
        
        
        // 动态申请空间
        dev->pid= (PID*)malloc(sizeof(PID));
		/* 初始化参数 */
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
/*设置用户目标值*/
static void Set_User_Target(Pid_System_t* dev,double temp_val){

  dev->pid->target_val = temp_val;    
}

/*获取用户目标值*/
static float Get_User_Target(Pid_System_t* dev)
{
  return dev->pid->user_target_val;   
}

/*设置目标值*/
static void Set_Target(Pid_System_t* dev)
{   
    float diff = dev->pid->target_val - dev->pid->user_target_val;
    //若差值小于步长 则直接等于目标值
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

/*获取目标值*/
static float Get_Target(Pid_System_t* dev)
{
  return dev->pid->target_val;   
}

/*设置比例、积分、微分系数 */
static void Set_P_I_D(Pid_System_t* dev,double p, double i, double d)
{
		dev->pid->Kp = p;    
		dev->pid->Ki = i;    
		dev->pid->Kd = d;    
}


/*增量式PID*/
static double Incremental_Realize(Pid_System_t* dev,double temp_val) 
{
	//计算误差
	dev->pid->err_now=dev->pid->target_val-temp_val;
    
    double proportion =dev->pid->Kp*dev->pid->err_now ;
    double  integral  = dev->pid->Ki*(dev->pid->err_now - dev->pid->err_last);
    double differential =dev->pid->Kd*(dev->pid->err_now - 2 * dev->pid->err_last + dev->pid->err_last_last);
    //PID计算
	double increment_val = proportion + integral + differential;
	dev->pid->output += increment_val;
    
	dev->pid->err_last_last = dev->pid->err_last;
    dev->pid->err_last = dev->pid->err_now;
    
    //限幅输出
    if(dev->pid->output < dev->pid->out_min)
    {dev->pid->output =dev->pid->out_min;}
    else if(dev->pid->output > dev->pid->out_max)
    {dev->pid->output =dev->pid->out_max;}
    
    
   
	return dev->pid->output;
}

/*向上位机发送PID参数*/
static void  Send_Param(Pid_System_t* dev,UART_HandleTypeDef* huart,uint8_t channel)
{
        float pid_temp[3] = {dev->pid->Kp, dev->pid->Ki, dev->pid->Kd};
		set_computer_value(huart,SEND_P_I_D_CMD, channel, pid_temp, 3);     
}


/*设置挡位*/
static void Set_User_Ratio(Pid_System_t* dev,Ratio ratio)
{   
    //根据挡位设置斜率slope
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



/*PID的API接口函数*/
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

