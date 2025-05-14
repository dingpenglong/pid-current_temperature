/*
@filename	:	PID.h
@brief		:	PID算法模块
@time		:	2024/12/2
@author		: 	丁鹏龙

@version    :   V1.0
*/

// <<< Use Configuration Wizard in Context Menu >>>

// <<< end of configuration section >>>

#ifndef __PID_H
#define __PID_H 

#ifdef __cplusplus
extern "C" {
#endif
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>


//检查指针非空
#define CHECK_PTR(ptr, param, field) do{\
                                    if(ptr == NULL) \
                                    { \
                                        param.field = 1;\
										TPS546D24A_PRINT_DEBUG("指针 %s 为空指针。\n", #ptr);\
                                        return param;\
                                    } \
                                }while(0)



/*pid*/


//变化速率挡位
typedef enum{
    Low     =0x1,
    Mideum  =0x2,
    High    =0x3,
}Ratio;

typedef struct
{
     int ID;               //PID系统编号
    double target_val;     //目标值
    Ratio ratio;              //上升档挡位
    int slope;               //上升斜率
    float step;              //步长
    double user_target_val;  //用户设定的实际值
	double actual_val;     //实际值
	double err_now;             //定义当前偏差值
	double err_last;            //定义上一个偏差值
	double err_last_last;       //定义上上一个偏差值
	double Kp, Ki, Kd;          //定义比例、积分、微分系数
    double output;             //输出值
    double out_max;             //最大输出值
    double out_min;             //最小输出

}PID;

typedef struct PID_SYSTEM Pid_System_t;
struct PID_SYSTEM
{
    PID* pid;
    
    void  (*Param_Init) (Pid_System_t* dev,int Id,double max,double min,double target_val,Ratio ratio,double step);
    
    void  (*Set_User_Target)(Pid_System_t* dev,double temp_val);
    
    float (*Get_User_Target)(Pid_System_t* dev);
    
    void  (*Set_Target)(Pid_System_t* dev);
    
    float (*Get_Target)(Pid_System_t* dev);
    
    void  (*Set_P_I_D)(Pid_System_t*dev,double p, double i, double d);
    
    double (*Incremental_Realize)(Pid_System_t* dev,double temp_val);
 
    
    void  (*Send_Param)(Pid_System_t* dev,UART_HandleTypeDef* huart1,uint8_t channel);
    
    void (*Set_User_Ratio)(Pid_System_t* dev,Ratio ratio);
};


//卡尔曼滤波
typedef struct {
    float Q;      // 过程噪声协方差
    float R;      // 测量噪声协方差
    float A;      // 状态转移矩阵
    float B;      // 控制输入矩阵
    float H;      // 测量矩阵
    float P;      // 误差协方差
    float x_hat;  // 状态估计值
    float K;      // 卡尔曼增益
} KalmanFilter;


void PID_API_INIT(Pid_System_t* dev);
void kalman_init(KalmanFilter *kf, float Q, float R, float A, float B, float H, float P, float x_hat);
float kalman_update(KalmanFilter *kf, float z, float u);

#ifdef __cplusplus
}
#endif

#endif  /* end of __PID.H__ */	

