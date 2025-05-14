/*
@filename	:	ntc.c
@brief		:	NTC的函数实现
@time		:	2024/12/04
@author		: 	黎健

@version    :   1.0
*/

#include <stdio.h>
#include <math.h>
#include "ntc.h"

//获取摄氏度对应的开尔文温度
#define GET_TEMP_K(temp_c) ((temp_c) + 273.15) 

//获取开尔文温度对应的摄氏度
#define GET_TEMP_C(temp_k) ((temp_k) - 273.15) 

//提供给用户的接口
    
/*
    初始化NTC
    r0;         //参考温度下的电阻，单位为KΩ
    t0;         //参考温度，单位为℃
    coef_temp;  //温度系数，单位为开尔文
*/
static void Ntc_Init(ntc_t *dev, const double r0, const double t0, const double coef_temp)
{
    if(dev == NULL)
        return ;
    
    dev->r0 = r0;
    dev->t0 = t0;
    dev->coef_temp = coef_temp;
}

/*
    反初始化NTC
*/
static void Ntc_DeInit(ntc_t *dev)
{
    if(dev == NULL)
        return ;
    
    dev->r0 = 0;
    dev->t0 = 0;
    dev->coef_temp = 0;
}


/*
    将NTC的电阻转换为温度，温度单位为摄氏度
    rt:NTC的阻值，单位为KΩ
*/
static double Ntc_ConvToTemp(ntc_t *dev, const double rt)
{
    double temp_c = GET_TEMP_C(0);
    
    //如果指针为空，返回绝对零度
    if(dev == NULL)
        return temp_c;
    
    double t0_k = GET_TEMP_K(dev->t0);
    
    temp_c = (dev->coef_temp * t0_k) / (t0_k * log(rt / dev->r0) + dev->coef_temp);
    temp_c = GET_TEMP_C(temp_c);
    return temp_c;
}


/*
    将NTC的温度转换为电阻，NTC的阻值单位为KΩ
    temp:NTC的温度，温度单位为摄氏度
*/
static double Ntc_ConvToR(ntc_t *dev, const double temp)
{
    double rt = 0;
    
    //如果指针为空，返回0电阻
    if(dev == NULL)
        return rt;
    
    double rt_temp_k = GET_TEMP_K(temp);
    double r0_temp_k = GET_TEMP_K(dev->t0);
    
    rt = dev->r0 *exp(dev->coef_temp * (r0_temp_k - rt_temp_k) / (rt_temp_k * r0_temp_k));
    return rt;
}

//提供给用户的公开函数接口
void NTC_API_INIT(ntc_t *dev)
{
    if(dev == NULL)
        return ;
    
    dev->Init       = Ntc_Init;
    dev->DeInit     = Ntc_DeInit;
    dev->ConvToR    = Ntc_ConvToR;
    dev->ConvToTemp = Ntc_ConvToTemp;
}
