/*
@filename	:	PID.h
@brief		:	PID�㷨ģ��
@time		:	2024/12/2
@author		: 	������

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


//���ָ��ǿ�
#define CHECK_PTR(ptr, param, field) do{\
                                    if(ptr == NULL) \
                                    { \
                                        param.field = 1;\
										TPS546D24A_PRINT_DEBUG("ָ�� %s Ϊ��ָ�롣\n", #ptr);\
                                        return param;\
                                    } \
                                }while(0)



/*pid*/


//�仯���ʵ�λ
typedef enum{
    Low     =0x1,
    Mideum  =0x2,
    High    =0x3,
}Ratio;

typedef struct
{
     int ID;               //PIDϵͳ���
    double target_val;     //Ŀ��ֵ
    Ratio ratio;              //��������λ
    int slope;               //����б��
    float step;              //����
    double user_target_val;  //�û��趨��ʵ��ֵ
	double actual_val;     //ʵ��ֵ
	double err_now;             //���嵱ǰƫ��ֵ
	double err_last;            //������һ��ƫ��ֵ
	double err_last_last;       //��������һ��ƫ��ֵ
	double Kp, Ki, Kd;          //������������֡�΢��ϵ��
    double output;             //���ֵ
    double out_max;             //������ֵ
    double out_min;             //��С���

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


//�������˲�
typedef struct {
    float Q;      // ��������Э����
    float R;      // ��������Э����
    float A;      // ״̬ת�ƾ���
    float B;      // �����������
    float H;      // ��������
    float P;      // ���Э����
    float x_hat;  // ״̬����ֵ
    float K;      // ����������
} KalmanFilter;


void PID_API_INIT(Pid_System_t* dev);
void kalman_init(KalmanFilter *kf, float Q, float R, float A, float B, float H, float P, float x_hat);
float kalman_update(KalmanFilter *kf, float z, float u);

#ifdef __cplusplus
}
#endif

#endif  /* end of __PID.H__ */	

