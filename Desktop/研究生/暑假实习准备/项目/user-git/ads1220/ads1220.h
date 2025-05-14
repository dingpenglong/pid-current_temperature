#ifndef __ADS1220_H__
#define __ADS1220_H__
/*
@filename   ads1220.h

@brief		����SPI��ADS1220����ͷ�ļ�����Ҫ֧��HAL��

@time		2024/08/25

@author		�轨

@version    1.0

@attention  ����SPI�ӿ����ʱ������Ϊ50MHz���������� SPI ���ݴ��нӿ������ڶ�ȡת�����ݡ���д�������üĴ����Լ�������������״̬��
            ��֧�� SPI ģʽ1��CPOL = 0��CPHA = 1�����ýӿ������������ߣ�CS#��SCLK��DIN��DOUT/DRDY# �� DRDY#����ɡ�
            
            ע�⣬��������֧��4���ƣ�CS#��SCLK��DIN��DOUT��Ӳ��SPI�ӿڣ�Ƭѡ�ź���HAL���Զ�����
            
            ע�⣬����ʹ����DRDY#�źţ�ʵ����û���õ�RDATA����
            
            ע�⣬������ʹ���ڲ�ʱ��Դ 4.096MHz
       
            ע�⣬�������������ʼ��SPIӲ���ӿڣ�Ҳ�������ʼ��DRDY#�źŵ�Ӳ���ӿڣ�������Ӧ���û�����оƬ�ֲ�Ҫ�����г�ʼ����
            ���Իص���������ʽ��ʼ�����ߵ�Ӳ���ӿڡ�
            

*/
#ifdef __cplusplus
extern "C" {
#endif

//����ϵͳͷ�ļ�
#include "stm32g4xx_hal.h"

/*
    ��1������ADS1220�ļĴ����б�Ͷ�Ӧ�Ĵ������ֶΡ�
*/

//�Ĵ����б�
typedef enum _ADS1220_RegList
{
    REG0    = 0x01,
    REG1    = 0x02,
    REG2    = 0x04,
    REG3    = 0x08,
    ALLREG  = 0x0F  //���мĴ���
}ADS1220_RegList_t;

//�Ĵ���0���ֶ�����
typedef union _ADS1220_Reg0
{
    struct
    {
        uint8_t pga_bypass  : 1;
        uint8_t gain        : 3;
        uint8_t mux         : 4;
    };
    
    uint8_t data;
}ADS1220_Reg0_t;

//�Ĵ���1���ֶ�����
typedef union _ADS1220_Reg1
{
    struct
    {
        uint8_t bcs     : 1;
        uint8_t ts      : 1;
        uint8_t cm      : 1;
        uint8_t mode    : 2;
        uint8_t dr      : 3;
    };
    
    uint8_t data;
}ADS1220_Reg1_t;

//�Ĵ���2���ֶ�����
typedef union _ADS1220_Reg2
{
    struct
    {
        uint8_t idac    : 3;
        uint8_t psw     : 1;
        uint8_t fir     : 2; //оƬ�ֲ��ϵ�50/60[1:0]
        uint8_t vref    : 2;
    };
    
    uint8_t data;
}ADS1220_Reg2_t;

//�Ĵ���3���ֶ�����
typedef union _ADS1220_Reg3
{
    struct
    {
        uint8_t         : 1; //ռλ
        uint8_t drdym   : 1;
        uint8_t i2mux   : 3; 
        uint8_t i1mux   : 3;
    };
    
    uint8_t data;
}ADS1220_Reg3_t;

//�ܼĴ����ֶ�����
typedef struct _ADS1220_Reg
{
    ADS1220_Reg0_t reg0;
    ADS1220_Reg1_t reg1;
    ADS1220_Reg2_t reg2;
    ADS1220_Reg3_t reg3;
}ADS1220_Reg_t; 


/*
    ��2������Ĵ������õĳ���
*/

/*
    MUX�������·���������ã���Щλ���������·��������
    ���� AINN = AVSS �����ã�PGA ������� (PGA_BYPASS = 1)�����ҽ���ʹ������ 1��2 �� 4��

    ����ģ���Դ (MUX[3:0] = 1101) ʱ���ó���ת�����ԼΪ (AVDD �C AVSS) / 4��
    �����Ƿ������üĴ�����ѡ���׼Դ (VREF[1:0])����������ʹ�� 2.048V �ڲ���׼��ѹ���в�����
    ������ⲿ��׼��ѹԴ (MUX[3:0] = 1100) ������֮һʱ�����ԼΪ (V(REFPx) �C V(REFNx)) / 4��
    REFPx �� REFNx��ʾ�����üĴ�����ѡ����ⲿ��׼����� (VREF[1:0])���������Զ�ʹ���ڲ���׼���в�����
*/
typedef enum _ADS1220_MUX
{
    AIN0_AIN1 = 0x0 ,   //Ĭ������, AINP=AIN0�� AINN=AIN1
    AIN0_AIN2 = 0x1 ,
    AIN0_AIN3 =	0x2 ,
    AIN1_AIN2 = 0x3 ,
    AIN1_AIN3 = 0x4 ,
    AIN2_AIN3 = 0x5 ,
    AIN1_AIN0 = 0x6 ,
    AIN3_AIN2 = 0x7 ,
    AIN0_AVSS = 0x8 ,
    AIN1_AVSS = 0x9 ,
    AIN2_AVSS = 0xa ,
    AIN3_AVSS = 0xb ,
    REFP_REFN = 0xc ,   //(V(REFPx) �C V(REFNx)) / 4�� ���Ӳο���Դ��ѹ����· PGA��
    AVDD_AVSS = 0xd ,   //(AVDD �C AVSS) / 4 ������ģ���Դ��ѹ����· PGA��
    ALL_MIDDLE= 0xe     //AINP �� AINN �̽��� (AVDD + AVSS) / 2������ƫ��У׼
}ADS1220_MUX_t;

/*
    GAIN���������ã���Щλ���������������档
    �ڲ�ʹ�� PGA ������£���ʹ������ 1��2 �� 4������������£�ͨ�����ص��ݽṹ������档
    ������ΪGAIN_Xʱ�� ʵ������Ϊ��2^(X-1)��
*/

typedef enum _ADS1220_GAIN
{
    GAIN_1 = 0, //����Ϊ1��Ĭ������
    GAIN_2 ,    //����Ϊ2,�����Դ�����
    GAIN_4 ,
    GAIN_8 ,
    GAIN_16 ,
    GAIN_32 ,
    GAIN_64 ,
    GAIN_128 
}ADS1220_GAIN_t;

/*
    PGA_BYPASS: ���ú���·�ڲ�������PGA��
    ���� PGA �ή�����幦�ģ����ɽ���ģ��ѹ��Χ (VCM) ��չΪ AVSS �C 0.1V ��AVDD + 0.1V��
    ֻ��������� 1��2 �� 4 ���� PGA��
    ���� PGA_BYPASS ������Σ���ʼ������������� 8 �� 128 ���� PGA��
*/
typedef enum _ADS1220_PGA_BYPASS
{
    PGA_ON = 0, //����PGA��Ĭ�����ã�
    PGA_OFF     //���ã���·��PGA
}ADS1220_PGA_BYPASS_t;

/*
    DR: �������ʣ���Щλ���������������ã�ȡ������ѡ����ģʽ��
    оƬ�վ��ֲ�ı�18�г�������ģʽ��ռ�ձ�ģʽ�� Turbo ģʽ��Ӧ��λ���á�
    ��18�ṩ����������ʹ���ڲ������� 4.096MHz �ⲿʱ�ӽ��м��㡣
    ���ʹ�õ���Ƶ�ʲ�Ϊ 4.096MHz ���ⲿʱ�ӣ����������ʻᰴ�ⲿʱ��Ƶ�ʳɱ������š�
*/
typedef enum _ADS1220_DR
{
    //����ģʽ
    NORMAL_20SPS    = 0,
    NORMAL_45SPS    = 1,
    NORMAL_90SPS    = 2,
    NORMAL_175SPS   = 3,
    NORMAL_330SPS   = 4,
    NORMAL_600SPS   = 5,
    NORMAL_1000SPS  = 6,
    
    //ռ�ձ�ģʽ
    DUTY_5SPS       = 0,
    DUTY_11_25SPS   = 1,
    DUTY_22_5SPS    = 2,
    DUTY_44SPS      = 3,
    DUTY_82_5SPS    = 4,
    DUTY_150SPS     = 5,
    DUTY_250SPS     = 6,
    
    //TURBOģʽ
    TURBO_40SPS    = 0,
    TURBO_90SPS    = 1,
    TURBO_180SPS   = 2,
    TURBO_350SPS   = 3,
    TURBO_660SPS   = 4,
    TURBO_1200SPS  = 5,
    TURBO_2000SPS  = 6
    
}ADS1220_DR_t;

/*
    MODE������ģʽ����Щλ�������������Ĺ���ģʽ��
    
*/
typedef enum _ADS1220_MODE
{
    NORMAL = 0 ,    //����ģʽ��256kHz ������ʱ�ӣ�Ĭ�����ã�
    DUTY ,          //ռ�ձ�ģʽ���ڲ�ռ�ձ� 1:4��
    TURBO           //Turbo ģʽ��512kHz ������ʱ�ӣ�
}ADS1220_MODE_t;

/*
    CM��ת��ģʽ����λ����Ϊ��������ת��ģʽ��
*/
typedef enum _ADS1220_CM
{
    SINGLE = 0 ,    //����ģʽ��Ĭ�����ã�
    CONTINUE        //����ת��ģʽ
}ADS1220_CM_t;

/*
    TS���¶ȴ�����ģʽ����λ���������ڲ��¶ȴ������Լ������������¶ȴ�����ģʽ�¡�
    �����¶ȴ�����ģʽ�����üĴ��� 0 �����ò�������κ�Ӱ�죬
    ������ʹ���ڲ���׼���в�����
*/
typedef enum _ADS1220_TS
{
    TS_OFF = 0 ,    //�����¶ȴ�������Ĭ�����ã�
    TS_ON           //�����¶ȴ�����
}ADS1220_TS_t;


/*
    BCS���ջٵ���Դ����λ���ڿ��� 10��A �ջٵ���Դ��
    �ջٵ���Դ�����ڼ�⴫�������ϣ����磬��������·�Ͷ�·����
    TI ������ִ�о��ܲ����Ĺ����н����ջٵ���Դ�����ҽ��ڲ��Դ�������������ʱ�������á�
*/
typedef enum _ADS1220_BCS
{
    BCS_OFF = 0 ,    //����Դ�ضϣ�Ĭ�����ã�
    BCS_ON           //����Դ��ͨ
}ADS1220_BCS_t;

/*
    VREF����׼��ѹѡ����Щλ����ѡ��ת����ʹ�õĻ�׼��ѹԴ��
    
*/
typedef enum _ADS1220_VREF
{
    VREF_INTERNAL = 0 ,  //ѡ�� 2.048V �ڲ���׼��ѹ��Ĭ�����ã�
    VREF_REF0 ,          //ʹ��ר�� REFP0 �� REFN0 ����ѡ����ⲿ��׼��ѹ
    VREF_REF1 ,          //ʹ�� AIN0/REFP1 �� AIN3/REFN1 ����ѡ����ⲿ��׼��ѹ
    VREF_POWER           //������׼��ģ���Դ (AVDD �C AVSS)
}ADS1220_VREF_t;

/*
    50/60��FIR �˲������ã���Щλ����Ϊ�ڲ� FIR �˲��������˲���ϵ����
    ������ģʽ�£���Щλ���� 20SPS ���ý��ʹ�ã�
    ��ռ�ձ�ģʽ�£���Щλ����5SPS ���ý��ʹ�á��������������������ʣ���Щλ������Ϊ 00��
*/
typedef enum _ADS1220_FIR
{
    FIR_OFF = 0 ,   //�� 50Hz �� 60Hz ���ƣ�Ĭ�����ã�
    FIR_ALL ,       //ͬʱ���� 50Hz �� 60Hz
    FIR_50_Hz ,     //ֻ���� 50Hz
    FIR_60_Hz       //ֻ���� 60Hz
}ADS1220_FIR_t;

/*
    PSW���Ͳ��Դ�������ã���λ�������� AIN3/REFN1 �� AVSS ֮�����ӵĵͲ࿪�ص���Ϊ��
    
*/
typedef enum _ADS1220_PSW
{
    PSW_OFF = 0 ,   //����ʼ�մ��ڶϿ�״̬��Ĭ�����ã�
    PSW_ON ,        //���ػ��ڷ��� START/SYNC ����ʱ�Զ��պϣ����ڷ��� POWERDOWN ����ʱ�Զ��Ͽ���
}ADS1220_PSW_t;

/*
    IDAC��IDAC �������ã���Щλ����Ϊ IDAC1 �� IDAC2 ��������Դ���õ�����
    
*/
typedef enum _ADS1220_IDAC
{
    IDAC_OFF = 0 ,   //�ضϣ�Ĭ�����ã�
    IDAC_10_UA ,      //10uA
    IDAC_50_UA ,      //50uA
    IDAC_100_UA ,     //100uA
    IDAC_250_UA ,     //250uA
    IDAC_500_UA ,     //500uA
    IDAC_1000_UA ,    //1000uA
    IDAC_1500_UA      //1500uA
}ADS1220_IDAC_t;

/*
    IDACx·�����ã���Щλ����ѡ�� IDACx ��·�ɵ���ͨ����
*/
typedef enum _ADS1220_IDACMUX
{
    MUX_OFF = 0,    //IDACx �ѽ��ã�Ĭ�����ã�
    AIN0 ,          //IDACx �������� AIN0/REFP1
    AIN1 ,          //IDACx �������� AIN1
    AIN2 ,          //IDACx �������� AIN2
    AIN3 ,          //IDACx �������� AIN3/REFN1
    REFP0 ,         //IDACx �������� REFP0
    REFN0           //IDACx �������� REFN0
}ADS1220_IDACMUX_t;

/*
    DRDYM��DRDY# ģʽ����λ���ڿ��������ݾ���ʱ DOUT/DRDY# ���ŵ���Ϊ��
*/
typedef enum _ADS1220_DRDYM
{
    DRDYM_ONLY = 0,   //��ר�� DRDY# ��������ָʾ���ݺ�ʱ������Ĭ�����ã�
    DRDYM_BOTH ,      //ͬʱͨ�� DOUT/DRDY# �� DRDY# ָʾ���ݾ���
}ADS1220_DRDYM_t;

/*
    (3)�������ADS1220ʱ�Ĵ�������
*/
typedef union _ADS1220_Error
{
    struct
    {
        uint8_t dev     : 1; //�豸������
        uint8_t malloc  : 1; //���붯̬�ռ�ʧ��
        uint8_t spi     : 1; //spi�ӿ���Ч 
        uint8_t drdy    : 1; //DRDY#�źŽӿ���Ч
        uint8_t set     : 1; //������������
        uint8_t timeout : 1; //������ʱ
    };
    uint8_t data;
}ADS1220_Error_t;   

/*
    ��4������ADS1220�豸������
*/

//ϵͳ������Ϣ
typedef struct _ADS1220_Monitor
{
    //оƬ��ģ�⹩���ѹ AVDD - AVSS
    double pwr;
    
    //оƬ���ⲿ��׼��ѹ VREFPx - VREFNx
    double vref[2];
    
    //оƬ�ĵ�ǰ�¶�
    double temp;
    
    //оƬ�ĵ�ѹƫ�ƣ�ʵ��ת����DA������Ӧ��ȥ��ƫ����
    int32_t volt_bias;
}ADS1220_Monitor_t;


//����ADS1220�豸������
typedef struct _ADS1220 ADS1220_t;

struct _ADS1220
{
    //�Ĵ����ṹ�壬�û�ͨ����д�ýṹ��������оƬ
    ADS1220_Reg_t regs;
    
    //ϵͳ������Ϣ
    ADS1220_Monitor_t monitor;
    
    //DRDY#�ź�����
    GPIO_TypeDef*   drdy_GPIO;    //DRDY#�ź�����GPIO
    uint16_t        drdy_BIT;     //DRDY#�źŵ�λ��
    
    //SPI�ӿ�������
    SPI_HandleTypeDef* hspi;
    
    //�û��ӿ�
    
    /*
        ��ʼ���豸
    */
    ADS1220_Error_t (*Init)(ADS1220_t* dev, const SPI_HandleTypeDef* hspi, 
    const GPIO_TypeDef* drdy_GPIO, const uint32_t drdy_BIT, void(*Func_CallBack)(void));
   
    /*
        ����ʼ���豸
    */
    ADS1220_Error_t (*DeInit)(ADS1220_t* dev, void(*Func_CallBack)(void));
    
    /*
        ��ADS1220оƬ��1��4���Ĵ���ֵ��ע��reg_list����ʹ�û�������ͬʱ������Ĵ�����ֵ
        ע�⣬�ɹ�������һ������ͬ��������Ӧ��regs�ĳ�Ա
    */
    ADS1220_Error_t (*ReadReg)(ADS1220_t* dev, const ADS1220_RegList_t reg_list);
    
    /*
        ��ADS1220оƬд1��4���Ĵ���ֵ��ע��reg_list����ʹ�û�������ͬʱд����Ĵ�����ֵ
        ע�⣬�ɹ�������һ������ͬ��������Ӧ��regs�ĳ�Ա
    */
    ADS1220_Error_t (*WriteReg)(ADS1220_t* dev, const ADS1220_RegList_t reg_list);
    
    /*
        ��ʼһ��ת��
    */
    ADS1220_Error_t (*Start)(ADS1220_t* dev);
    
    /*
        ֹͣת��
        ��оƬ�����ڵ���ת��ģʽ���򲻻�ִ���κβ�������Ϊ����ģʽ��ת������Զ�ֹͣ
        ��оƬ��������ת��ģʽ������оƬ������������
    */
    ADS1220_Error_t (*Stop)(ADS1220_t* dev);
    
    /*
        ����оƬ
    */
    ADS1220_Error_t (*Reset)(ADS1220_t* dev);
   
    /*
    ���Զ�ȡһ��ADC��ת������ֱ����ʱ�����øú���ǰ���뱣֤ADC�Ѵ�������ת��״̬
    ��ȡ��ADCת�����ԭʼ���ݣ���ʱdataָ��ָ�� int32_t ����,��8λ��Ч��
    */
    ADS1220_Error_t (*GetAdcData)(ADS1220_t* dev, int32_t* data, const uint32_t max_wait_ms);
    
    /*
    ��ADCת�����24bit���ݸ��ݸ�������תΪģ���ѹֵ
    data_in�ĸ�8λ��Ч��
    */
    ADS1220_Error_t (*ConvData)(ADS1220_t* dev, const int32_t* data_in, double* data_out);
    
    /*
    ��ȡģ���Դ��ѹ���ɼ�������samp_cntָ����������ƽ��ֵ
    ����ģ���Դ (MUX[3:0] = 1101) ʱ���ó���ת�����ԼΪ (AVDD �C AVSS) / 4��
    �����Ƿ������üĴ�����ѡ���׼Դ (VREF[1:0])����������ʹ�� 2.048V �ڲ���׼��ѹ���в�����
	
	ע�⣬���øú�����ʹADCоƬ���ڵ���״̬
    */
    ADS1220_Error_t (*GetPwrVolt)(ADS1220_t* dev, const uint8_t samp_cnt);
     
    /*
    ��ȡ�ⲿ��׼Դ��Դ��ѹ���ɼ�������samp_cntָ����������ƽ��ֵ
    ����ǰ������Ҫ���û�׼ԴΪ�ⲿ��׼
	
	ע�⣬���øú�����ʹADCоƬ���ڵ���״̬
    */
    ADS1220_Error_t (*GetRefVolt)(ADS1220_t* dev, const uint8_t samp_cnt);
     
     /*
    ��ȡ�¶ȣ��ɼ�������samp_cntָ����������ƽ��ֵ
 
    ADS1220 ������һ�������¶ȴ�������ͨ�������üĴ����� TS λ�� 1 ��ʹ���¶ȴ�����ģʽ��
    ���¶ȴ�����ģʽ�£����üĴ��� 0 �����ò������κ�Ӱ�죬������ʹ���ڲ���׼���в���������ѡ��׼��ѹԴ�޹ء�
    �¶ȶ���������ģ��������������ȡת������Ĺ�����ͬ���¶������� 14 λ������֣��� 24 λת���������롣
    ���ݴ������Ч�ֽ� (MSB) ��ʼ���������ȡ�����������ֽڣ�ǰ 14 λ����ָ���¶Ȳ��������
    һ�� 14 λ LSB ����0.03125��C�������Զ����Ʋ�����ʽ��ʾ����оƬ�ֲ�ı� 12 ��ʾ��

	ע�⣬���øú�����ʹADCоƬ���ڵ���״̬

    */
    ADS1220_Error_t (*GetTemp)(ADS1220_t* dev, const uint8_t samp_cnt);
	
	/*
    ����ƫ��У׼���ɼ�������samp_cntָ����������ƽ��ֵ��
	
	ע�⣬���øú�����ʹADCоƬ���ڵ���״̬
	*/

	ADS1220_Error_t (*GetBias)(ADS1220_t* dev, const uint8_t samp_cnt);
};

/*
    (5)������ʼ��ADS1220�����ĺ����ӿ�
*/

ADS1220_Error_t ADS1220_API_INIT(ADS1220_t* dev);


#ifdef __cplusplus
}
#endif

#endif /* __ADS1220_H__ */


