#ifndef __TPS546D24A_H__
#define __TPS546D24A_H__
/*
@filename   tps546d24a.h

@brief		����PMBUS���Ƶĺ���Դ

@time		2024/10/10

@author		������

@version    1.1

--------------------------------------------------------

@attention   
			һ��Ҫ��оƬ�ȳ�ʼ�������ú�VOUT֮�󣬲Ž�������
            ��ʼ������ѹ����Ϊ5.5V,��С��ѹ����Ϊ0.5V��Ĭ�ϵ�ѹҪ�����õĵ�ѹ��Χ��
            д��˳���ǵ��ֽ���ǰ�����ֽ��ں�
*/
#include "stm32g4xx_hal.h"
#include "stm32_PMBUS_stack.h"
//#include "app_x-cube-smbus.h"
#ifdef __cplusplus
extern "C" {
#endif

//Linear11���ݸ�ʽ------PMBUS�ڲ������ݸ�ʽת��
typedef struct{
union{
	struct{
		int16_t linear11Y :11;
		int16_t linear11N :5;
	};
	uint16_t  value ;
};
}LINEAR11;

void Linear11_to_data(double *linear_f,LINEAR11 *linear);
#define SALVE_ADDRESS 0x48  //�ӻ���ַ
typedef union
{
    struct
    {
        uint16_t dev     : 1; //�豸������
        uint16_t malloc  : 1; //���붯̬�ռ�ʧ��
		uint16_t Pcontext     : 1; //pcontext��Ч
		uint16_t null_data  : 1; //����Ϊ�ջ��ߴ�С����
		uint16_t size_data  : 1; //���ݴ�С����
        uint16_t set_vout    : 1; //vout������Ч
		uint16_t get_vout    : 1; //vout��ȡ��Ч
		uint16_t set_iout    : 1; //iout������Ч
		uint16_t get_iout    : 1; //iout��ȡ��Ч
        uint16_t vout_max    : 1; //��������ѹ������Ч
        uint16_t vout_min     : 1; //��С�����ѹ������Ч    
        uint16_t init          : 1; //��ʼ������    
    };
    uint16_t data;
}TPS546D24A_Error;  

typedef union {
    struct {
        uint16_t none_of_the_above : 1;     // ���й�������Ĺ���
        uint16_t cml : 1;                   // ͨ�š��洢�����߼�����
        uint16_t temp : 1;                  // �¶ȹ���/����
        uint16_t vin_uv : 1;                // ����Ƿѹ����
        uint16_t iout_oc : 1;               // �����������
        uint16_t vout_ov : 1;               // �����ѹ����
        uint16_t off : 1;                   // ת����Դ�ر�
        uint16_t busy : 1;                  // ������æ���޷���Ӧ
        uint16_t other : 1;                 // ��������
        uint16_t manufacturer : 1;          // �����̶���Ĺ���
        uint16_t input : 1;                 // �������
        uint16_t iout : 1;                  // �����������
        uint16_t vout : 1;                  // �����ѹ����
        uint16_t : 1;                      // ռλ
    };
    uint16_t data; 
} STATUS_Error;

typedef union {
    struct {
        uint8_t VOUT_OVF : 1;      // �����ѹ���ϱ�־
        uint8_t VOUT_OVW : 1;      // �����ѹ�����־
        uint8_t VOUT_UVW : 1;      // ���Ƿѹ�����־
        uint8_t VOUT_UVF : 1;      // ���Ƿѹ���ϱ�־
        uint8_t VOUT_MIN_MAX : 1;  // �����ѹ��С/�����ϱ�־
        uint8_t TON_MAX : 1;       // ���ͨʱ����ϱ�־
        uint8_t unsupported : 1;   // ��֧�ֵ�λ��ʼ��Ϊ0
    } bits;
    uint8_t data;                  // �������������ʾ�����������д
}STATUSVOUT_Error;


typedef struct Smbus_Command_t Smbus_Command;


typedef struct _TPS546d24A_t TPS546d24A_t;


struct _TPS546d24A_t
{	/*command �Ĵ���*/
    Smbus_Command* smbus_command;
	
	/*
		SMBUSջָ��ӿ�
	*/

	SMBUS_StackHandleTypeDef *pcontext;
	
    
    //�����ӿ�
    
  /*��ʼ��TPS54624A*/
    TPS546D24A_Error (* Init)(TPS546d24A_t* dev,SMBUS_StackHandleTypeDef *pcontext,const float vout_max, const float  vout_min,  const float vout_default);
     /*
        ����ʼ��TPS546D24A
    */
    TPS546D24A_Error (* DeInit)(TPS546d24A_t* dev, void (*fun_callback)(void));

    /*
        ���������ѹ
    */
    TPS546D24A_Error(*SetVout)(TPS546d24A_t* dev,  double voutcommand);
	
    /*
		��ȡ�����ѹ
    */
	TPS546D24A_Error(*GetVout)(TPS546d24A_t* dev, double* actual_vout);
	
	 /*
		��ȡ�������
    */
	TPS546D24A_Error(*GetIout)(TPS546d24A_t* dev, double* actual_iout);
	
	 /*
		��ȡоƬ�¶�
    */
	TPS546D24A_Error(*GetTemperature)(TPS546d24A_t* dev, double* temperature);

    /*
		������
    */
    TPS546D24A_Error (* SoftOn)(TPS546d24A_t* dev);
    
	 /*
		��ر�
    */
    TPS546D24A_Error (* SoftOff)(TPS546d24A_t* dev);
	
	/*����VOUTMAX
	*/
    TPS546D24A_Error (* SetVoutMax)(TPS546d24A_t* dev,const double vout_max);
		
	/*����VOUTMIM
	*/
    TPS546D24A_Error (* SetVoutMin)(TPS546d24A_t* dev,const double vout_min);
	
	/*���ÿ���Ƶ��*/
	TPS546D24A_Error (* SetFrequancySwitch)(TPS546d24A_t* dev,  double frequancyswitch);
    
//    /*���õ�����ֵ*/
//    TPS546D24A_Error  (* SetIoutLimit)(TPS546d24A_t* dev,double iout_limit);
	
	/*���õ�����������*/
	TPS546D24A_Error (* SetIoutGain)(TPS546d24A_t* dev,  double ioutgain);
	
	/*���õ�������ƫ��*/
	TPS546D24A_Error (* SetIoutOffset)(TPS546d24A_t* dev,  double ioutoffset);
	
	/*��ѯ�豸״̬����*/
	TPS546D24A_Error(* QueryDevserror)(TPS546d24A_t* dev);
    /*��ѯVout����*/
	TPS546D24A_Error(*QueryVoutError)(TPS546d24A_t* dev);
	/*��ѯ���ô���*/
	void (* QuerySettingserror)(TPS546D24A_Error  error); 
	

	
	
};

/*
    (4)������ʼ��TPS546D24A�����ĺ����ӿ�
*/

TPS546D24A_Error TPS546D24A_API_INIT(TPS546d24A_t* dev);


#ifdef __cplusplus
}
#endif



#endif   /* __TPS546D24A_H__*/


