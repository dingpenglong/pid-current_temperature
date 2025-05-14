/*
@filename	:	ntc.h
@brief		:	�ṩ���ڸ��¶�ϵ����NTC���Ľӿڷ�װ������ͨ����������ʵ���¶Ⱥ͵�ѹ��ת����
@time		:	2024/12/04
@author		: 	�轡

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
    double r0;           //�ο��¶��µĵ��裬��λΪK��
    double t0;           //�ο��¶ȣ���λΪ��
    double coef_temp;    //�¶�ϵ������λΪ������


    //�ṩ���û��Ľӿ�
    
    /*
        ��ʼ��NTC
        r0;         //�ο��¶��µĵ��裬��λΪK��
        t0;         //�ο��¶ȣ���λΪ��
        coef_temp;  //�¶�ϵ������λΪ������
    */
    void (*Init)(ntc_t *dev, const double r0, const double t0, const double coef_temp);
    
    /*
        ����ʼ��NTC
    */
    void (*DeInit)(ntc_t *dev);
    
    
    /*
        ��NTC�ĵ���ת��Ϊ�¶ȣ��¶ȵ�λΪ���϶�
        rt:NTC����ֵ����λΪK��
    */
    double (*ConvToTemp)(ntc_t *dev, const double rt);
    
    
    /*
        ��NTC���¶�ת��Ϊ���裬NTC����ֵ����λΪK��
        temp:NTC���¶ȣ��¶ȵ�λΪ���϶�
    */
    double (*ConvToR)(ntc_t *dev, const double temp);
};

//�ṩ���û��Ĺ��������ӿ�
void NTC_API_INIT(ntc_t *dev);


#ifdef __cplusplus
}
#endif

#endif  /* end of __NTC_H__ */
