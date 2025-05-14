/*
@filename	:	ntc.h
@brief		:	提供关于负温度系数（NTC）的接口封装。可以通过本驱动来实现温度和电压的转换。
@time		:	2024/12/04
@author		: 	黎健

@version    :   1.0 
*/

// <<< Use Configuration Wizard in Context Menu >>>

// <<< end of configuration section >>>

#ifndef __NTC_H__
#define __NTC_H__ 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ntc_t ntc_t;

struct _ntc_t
{
    double r0;           //参考温度下的电阻，单位为KΩ
    double t0;           //参考温度，单位为℃
    double coef_temp;    //温度系数，单位为开尔文


    //提供给用户的接口
    
    /*
        初始化NTC
        r0;         //参考温度下的电阻，单位为KΩ
        t0;         //参考温度，单位为℃
        coef_temp;  //温度系数，单位为开尔文
    */
    void (*Init)(ntc_t *dev, const double r0, const double t0, const double coef_temp);
    
    /*
        反初始化NTC
    */
    void (*DeInit)(ntc_t *dev);
    
    
    /*
        将NTC的电阻转换为温度，温度单位为摄氏度
        rt:NTC的阻值，单位为KΩ
    */
    double (*ConvToTemp)(ntc_t *dev, const double rt);
    
    
    /*
        将NTC的温度转换为电阻，NTC的阻值，单位为KΩ
        temp:NTC的温度，温度单位为摄氏度
    */
    double (*ConvToR)(ntc_t *dev, const double temp);
};

//提供给用户的公开函数接口
void NTC_API_INIT(ntc_t *dev);


#ifdef __cplusplus
}
#endif

#endif  /* end of __NTC_H__ */
