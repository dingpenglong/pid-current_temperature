/*
@filename	:	ntc.c
@brief		:	NTC�ĺ���ʵ��
@time		:	2024/12/04
@author		: 	�轡

@version    :   1.0
*/

#include <stdio.h>
#include <math.h>
#include "ntc.h"

//��ȡ���϶ȶ�Ӧ�Ŀ������¶�
#define GET_TEMP_K(temp_c) ((temp_c) + 273.15) 

//��ȡ�������¶ȶ�Ӧ�����϶�
#define GET_TEMP_C(temp_k) ((temp_k) - 273.15) 

//�ṩ���û��Ľӿ�
    
/*
    ��ʼ��NTC
    r0;         //�ο��¶��µĵ��裬��λΪK��
    t0;         //�ο��¶ȣ���λΪ��
    coef_temp;  //�¶�ϵ������λΪ������
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
    ����ʼ��NTC
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
    ��NTC�ĵ���ת��Ϊ�¶ȣ��¶ȵ�λΪ���϶�
    rt:NTC����ֵ����λΪK��
*/
static double Ntc_ConvToTemp(ntc_t *dev, const double rt)
{
    double temp_c = GET_TEMP_C(0);
    
    //���ָ��Ϊ�գ����ؾ������
    if(dev == NULL)
        return temp_c;
    
    double t0_k = GET_TEMP_K(dev->t0);
    
    temp_c = (dev->coef_temp * t0_k) / (t0_k * log(rt / dev->r0) + dev->coef_temp);
    temp_c = GET_TEMP_C(temp_c);
    return temp_c;
}


/*
    ��NTC���¶�ת��Ϊ���裬NTC����ֵ��λΪK��
    temp:NTC���¶ȣ��¶ȵ�λΪ���϶�
*/
static double Ntc_ConvToR(ntc_t *dev, const double temp)
{
    double rt = 0;
    
    //���ָ��Ϊ�գ�����0����
    if(dev == NULL)
        return rt;
    
    double rt_temp_k = GET_TEMP_K(temp);
    double r0_temp_k = GET_TEMP_K(dev->t0);
    
    rt = dev->r0 *exp(dev->coef_temp * (r0_temp_k - rt_temp_k) / (rt_temp_k * r0_temp_k));
    return rt;
}

//�ṩ���û��Ĺ��������ӿ�
void NTC_API_INIT(ntc_t *dev)
{
    if(dev == NULL)
        return ;
    
    dev->Init       = Ntc_Init;
    dev->DeInit     = Ntc_DeInit;
    dev->ConvToR    = Ntc_ConvToR;
    dev->ConvToTemp = Ntc_ConvToTemp;
}
