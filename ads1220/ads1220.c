#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "ads1220.h"
#include "ads1220_conf.h"


//��ӡ������Ϣ
#ifdef ADS1220_PRINT_DEBUE_INFO 
#define ADS1220_PRINT_DEBUG(fmt,args...) do{printf("file:%s(%d) func %s:\n", __FILE__,__LINE__,  __FUNCTION__);printf(fmt, ##args);}while(0)
#else
#define ADS1220_PRINT_DEBUG(fmt,args...) 
#endif

/*
    ��1������ADS1220�ĵײ�������
*/


//������
typedef enum _ADS1220_CMD
{
    CMD_RESET       = 0x06, //��λ����
    CMD_START       = 0x08, //����������ת��
    CMD_POWERDOWN   = 0x02, //�������ģʽ
    CMD_RDATA       = 0x10, //ͨ�������ȡ����
    CMD_RREG        = 0x20, //��ȡ nn �Ĵ�������ʼ��ַ��rr��: 0010 rrnn, ��������rr = ���üĴ�����00 �� 11����nn = �ֽ��� �C 1��00 �� 11��  
    CMD_WREG        = 0x40, //д�� nn �Ĵ�������ʼ��ַ��rr��: 0100 rrnn, ��������rr = ���üĴ�����00 �� 11����nn = �ֽ��� �C 1��00 �� 11��
}ADS1220_CMD_t;

//ϵͳ���������
typedef enum _ADS1220_SysMon_CMD
{
	SYSMON_CMD_AVDD,  //���ģ���Դ��ѹ��AVDD - AVSS
	SYSMON_CMD_REF0,  //����ⲿ��׼��ѹ0��VREFP0 - VREFN0	
	SYSMON_CMD_REF1,  //����ⲿ��׼��ѹ1��VREFP0 - VREFN0
	SYSMON_CMD_BIAS,  //ƫ��У׼
	SYSMON_CMD_TEMP,  //�¶ȴ���
}ADS1220_SysMon_CMD_t;

//����ADCоƬ����

//�ڲ���׼��ѹԴΪ 2.048V
#define ADS1220_INTERNAL_VREF 2.048 

//ADC���24λ�з������������ֵ����2^23 - 1
#define ADS1220_MAX_ADC_DATA  0X800000 //2^23 - 1
#define ADS1220_TEMP_FACTOR   0.03125

/*
    ��2��ʵ�ֶ�ADS1220оƬ�ĵײ��������д�Ĵ����������Լ����������������ADS1220_CMD_t�г������
*/

//���ָ��ǿգ���Ϊ���򽫶�Ӧ��־λ��1
#define CHECK_PTR(ptr, param, field)    do{\
                                            if(ptr == NULL) \
                                            { \
                                                param.field = 1;\
												ADS1220_PRINT_DEBUG("ָ�� %s Ϊ��ָ�롣\n", #ptr);\
                                                return param;\
                                            } \
                                        }while(0)
                                                              
                                        
//��������                              
static ADS1220_Error_t ADS1220_SendCMD(const ADS1220_t* dev, const ADS1220_CMD_t cmd)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //���SPI�ӿڷǿ�
    CHECK_PTR(dev->hspi, error, spi);
    
    //���������ʱ100ms
    uint8_t data = cmd;
    HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(dev->hspi, &data, 1, 100);
    
    //����SPI�����״̬��
    switch(spi_status)
    {
        case HAL_OK:
            break;
        case HAL_ERROR:
            error.spi = 1;
            break;
        case HAL_BUSY:
            error.spi = 1;
            break;
       case HAL_TIMEOUT:
            error.timeout = 1;
            break;
    }
    
    return error;
}    
                                    
//����оƬ
//static ADS1220_Error_t ADS1220_Reset(ADS1220_t* dev)
//{
//    ADS1220_Error_t error;
//    
//	//������оƬ֮ǰ���оƬ�Ƿ��ϵ����
//	int i=0;
//	while(HAL_GPIO_ReadPin(dev->drdy_GPIO, dev->drdy_BIT))
//	{
//		ADS1220_DELAY_1MS;
//		
//		i++;
//		
//		//�������100ms��δ����DRDY#�źţ����������ܲ������ϵ縴λ״̬
//		if(i == 100)
//		{
//			ADS1220_PRINT_DEBUG("оƬ��100ms��δ����ϵ縴λ���������͸�λ����\n");
//			ADS1220_SendCMD(dev, CMD_RESET);
//		}
//		
//		//����1��δ���óɹ�,����Ϊ��ʱ
//		else if(i == 1000)
//		{
//			ADS1220_PRINT_DEBUG("оƬ��1000ms��δ����ϵ縴λ����λ��ʱ\n");
//			error.timeout = 1;
//			return error;
//		}		
//	}
//	
//    error = ADS1220_SendCMD(dev, CMD_RESET);
//	
//	//���оƬ�Ƿ��ϵ����
//	i=0;
//	while(HAL_GPIO_ReadPin(dev->drdy_GPIO, dev->drdy_BIT))
//	{
//		ADS1220_DELAY_1MS;
//		i++;
//		
//		if(i == 1000)
//		{
//			ADS1220_PRINT_DEBUG("���͸�λ�����оƬ��1000ms��δ����ϵ縴λ���ȴ���ʱ\n");
//			error.timeout = 1;
//			return error;
//		}
//	}    
//	ADS1220_PRINT_DEBUG("Time = %d\n",i);
//    return error;
//}


static ADS1220_Error_t ADS1220_Reset(ADS1220_t* dev)
{
    ADS1220_Error_t error;
    
	//������оƬ֮ǰ���оƬ�Ƿ��ϵ����
    /*
    �������ϵ������ִ�и�λ����λ���̺�ʱԼ 50us��
    �ϵ�󣬸�������Ĭ�ϼĴ�������ִ�е���ת����Ȼ�����͹���״̬��
    ���ת����DRDY �����ɸߵ�ƽת��Ϊ�͵�ƽ��
    */
    // ��ADS1220�������ϵ�͸�λ�����У���DRDY�ź�һ��Ϊ�ߵ�ƽ
    // �����������ֲᣬ������Ҫ�����ʱ100msֱ����⵽DRDY�ź��½���
    uint16_t i=0;
    while(HAL_GPIO_ReadPin(dev->drdy_GPIO, dev->drdy_BIT))
	{
		ADS1220_DELAY_1MS;
        i++;
        
        // ������100ms��δ��⵽DRDY�ź�Ϊ�͵�ƽ��˵��ADS1220�Ѿ��������У���δ����ת��
        if(i >= 100)
            break;
	}
	
    dev->regs.reg1.cm = CONTINUE;
    dev->WriteReg(dev,REG1);
    // �ֶ����͸�λ����,�ȴ�1ms�ٷ��Ϳ�ʼת������
    error = ADS1220_SendCMD(dev, CMD_RESET);
    for(int i =0; i<100;i++)
    ADS1220_DELAY_1MS; 
	// ���оƬ�Ƿ�λ��
    dev->ReadReg(dev,REG1); 
    if(dev->regs.reg1.cm != SINGLE)
    {
       ADS1220_PRINT_DEBUG("���͸�λ�����оƬ��1000ms��δ����ϵ縴λ���ȴ���ʱ\n");
       error.timeout =1;
       return error;
    }
    return error;
}

//����������ת��
static ADS1220_Error_t ADS1220_Start(ADS1220_t* dev)
{
    return ADS1220_SendCMD(dev, CMD_START);
}    

//�������ģʽ
static ADS1220_Error_t ADS1220_PowerDown(ADS1220_t* dev)
{
    return  ADS1220_SendCMD(dev, CMD_POWERDOWN);
}

//ָʾADC�ṩ����ת������
static ADS1220_Error_t ADS1220_ReadData(ADS1220_t* dev)
{
    return  ADS1220_SendCMD(dev, CMD_RDATA);
}

//��ȡ�Ĵ�������
static ADS1220_Error_t ADS1220_ReadReg(ADS1220_t* dev, const ADS1220_RegList_t reg_list)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //�������ȡ��ֱ�ӷ���
    if(reg_list == 0)
    {
        error.set = 1;
        return error;
    }
    
    //���SPI�ӿڷǿ�
    CHECK_PTR(dev->hspi, error, spi);
    
    //�����Ҫ��ȡ�ļĴ���
    uint8_t tx_data[2] = {CMD_RREG, 0XFF};
    uint8_t rx_data[2] = {0XFF, 0XFF};
    
    for(uint8_t i=1, ind=0; i<=reg_list; i=i<<1)
    {
        if(reg_list & i)
        {
            //��������������
            tx_data[0] = CMD_RREG | (ind<<2);
            
            //ȫ˫�����������յ�����Ч����λ��rx_data[1]�У���ʱ1000ms
            HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(dev->hspi, tx_data, rx_data, 2, 1000);
            
            //����SPI�����״̬��
            switch(spi_status)
            {
                case HAL_OK:
                    break;
                
                case HAL_ERROR:
                    error.spi = 1;
                    break;
                
                case HAL_BUSY:
                    error.spi = 1;
                    break;
                
                case HAL_TIMEOUT:
                    error.timeout = 1;
                    break;
            }
            
            //������ڴ�����Ϊ���ζ�ȡ��Ч
            if(error.data != 0)
            {
                ind++;
                continue;
            }
            
            //���ݼĴ���ƫ�Ƶ�ַ��ֵ
            ADS1220_RegList_t reg = 1<<ind;
            switch(reg)
            {
                case REG0:
                    dev->regs.reg0.data = rx_data[1];    
                    break;
                
                case REG1:
                    dev->regs.reg1.data = rx_data[1];
                    break;
                
                case REG2:
                    dev->regs.reg2.data = rx_data[1];
                    break;
                
                case REG3:
                    dev->regs.reg3.data = rx_data[1];
                    break;
                
                default:
                    break;
            }
        }
        
        //�Ĵ���ƫ�Ƶ�ַ����
        ind++;
    }
    
    return error;
}

//д��Ĵ�������
static ADS1220_Error_t ADS1220_WriteReg(ADS1220_t* dev, const ADS1220_RegList_t reg_list)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //������д�룬ֱ�ӷ���
    if(reg_list == 0)
    {
        error.set = 1;
        return error;
    }
    
    //���SPI�ӿڷǿ�
    CHECK_PTR(dev->hspi, error, spi);
    
    //�����Ҫд��ļĴ���
    uint8_t tx_data[2] = {CMD_WREG, 0XFF};

    
    for(uint8_t i=1, ind=0; i<=reg_list; i=i<<1)
    {
        if(reg_list & i)
        {
            //��������������
            tx_data[0] = CMD_WREG | (ind<<2);
            
            //���ݼĴ���ƫ�Ƶ�ַ��ֵ
            ADS1220_RegList_t reg = 1<<ind;
            switch(reg)
            {
                case REG0:
                     tx_data[1] = dev->regs.reg0.data;    
                    break;
                
                case REG1:
                    tx_data[1] = dev->regs.reg1.data;  
                    break;
                
                case REG2:
                    tx_data[1] = dev->regs.reg2.data;  
                    break;
                
                case REG3:
                    tx_data[1] = dev->regs.reg3.data;  
                    break;
                
                default:
                    break;
            }
            
            //�������ݣ���ʱ200ms
            HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(dev->hspi, tx_data, 2, 200);
            
            //����SPI�����״̬��
            switch(spi_status)
            {
                case HAL_OK:
                    break;
                
                case HAL_ERROR:
                    error.spi = 1;
                    break;
                
                case HAL_BUSY:
                    error.spi = 1;
                    break;
                
                case HAL_TIMEOUT:
                    error.timeout = 1;
                    break;
            }
        }
        
        //�Ĵ���ƫ�Ƶ�ַ����
        ind++;
    }
    
    return error;
}


/*
    ��3��ʵ��Ӧ�ò�ӿ�
*/

/*
    ��ʼ���豸   
*/
static ADS1220_Error_t ADS1220_Init(ADS1220_t* dev, const SPI_HandleTypeDef* hspi, 
    const GPIO_TypeDef* drdy_GPIO, const uint32_t drdy_BIT, void(*Func_CallBack)(void))
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //���SPI�ӿڷǿ�
    CHECK_PTR(hspi, error, spi);
    
    //���DRDY�źŽӿڷǿ�
    CHECK_PTR(drdy_GPIO, error, drdy);
    
    //��Ӳ���ӿ�
    //DRDY#�ź�
    dev->drdy_GPIO = (GPIO_TypeDef*)drdy_GPIO;  //DRDY#�ź�����GPIO
    dev->drdy_BIT  = drdy_BIT & 0XFFFF;         //DRDY#�źŵ�λ��
    
    //SPI�ӿ�
    dev->hspi = (SPI_HandleTypeDef*)hspi;
    
    //��ʼ���Ĵ����ṹ���Ա��Ĭ��ֵ����оƬ�����ֲ�
    dev->regs.reg0.data = 0;
    dev->regs.reg1.data = 0;
    dev->regs.reg2.data = 0;
    dev->regs.reg3.data = 0;
    
    //��ʼ���������ṹ���Ա
    dev->monitor.pwr = 0;
    dev->monitor.temp = 0;
    dev->monitor.vref[0] = 0;
    dev->monitor.vref[1] = 0;
    
    //����оƬ
    error = dev->Reset(dev);
    
	//��ȡоƬ��������
	dev->GetBias(dev, 5);
	dev->GetPwrVolt(dev, 5);     
    dev->GetTemp(dev, 5);
    
    //�����ڻص����������ûص�����
    if(Func_CallBack != NULL)
        Func_CallBack();
    
    return error;
}


/*
    ����ʼ���豸
*/
static ADS1220_Error_t ADS1220_DeInit(ADS1220_t* dev, void(*Func_CallBack)(void))
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //����оƬ
    error = ADS1220_Reset(dev);
    
    //���Ӳ���ӿ�
    //DRDY#�ź�
    dev->drdy_GPIO = NULL;
    dev->drdy_BIT  = NULL;
    
    //SPI�ӿ�
    dev->hspi = NULL;
    
    //��ʼ���Ĵ����ṹ���Ա��Ĭ��ֵ����оƬ�����ֲ�
    dev->regs.reg0.data = 0;
    dev->regs.reg1.data = 0;
    dev->regs.reg2.data = 0;
    dev->regs.reg3.data = 0;
    
    //��ʼ���������ṹ���Ա
    dev->monitor.pwr = 0;
    dev->monitor.temp = 0;
    dev->monitor.vref[0] = 0;
    dev->monitor.vref[1] = 0;
    dev->monitor.volt_bias = 0;
    
    //�����ָ��
    dev->Init       = NULL;
    dev->DeInit     = NULL;
    dev->ReadReg    = NULL;
    dev->WriteReg   = NULL;
    dev->Start      = NULL;
    dev->Stop       = NULL;
    dev->Reset      = NULL;
    dev->GetAdcData = NULL;
    dev->ConvData   = NULL;
    dev->GetPwrVolt = NULL;
    dev->GetRefVolt = NULL;
    dev->GetTemp    = NULL;
	dev->GetBias	= NULL;
    
    //�����ڻص����������ûص�����
    if(Func_CallBack != NULL)
        Func_CallBack();
    
    return error;
}

/*
    ֹͣת��
    ��оƬ�����ڵ���ת��ģʽ���򲻻�ִ���κβ�������Ϊ����ģʽ��ת������Զ�ֹͣ
    ��оƬ��������ת��ģʽ������оƬ������������
*/
static ADS1220_Error_t ADS1220_Stop(ADS1220_t* dev)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //���оƬ����״̬
    error = ADS1220_ReadReg(dev, REG1);
    if(error.data)
        return error;
    
    if(dev->regs.reg1.cm == SINGLE)
        return error;
    
    //��������ת��ģʽ��������������
    error = ADS1220_PowerDown(dev);
    
    return error;
}

/*
    ���Զ�ȡһ��ADC��ת������ֱ����ʱ�����øú���ǰ���뱣֤ADC�Ѵ�������ת��״̬
    ��ȡ��ADCת�����ԭʼ���ݣ���ʱdataָ��ָ�� int32_t ����,��8λ��Ч��
*/
static ADS1220_Error_t ADS1220_GetAdcData(ADS1220_t* dev, int32_t* data, const uint32_t max_wait_ms)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //���SPI�ӿڷǿ�
    CHECK_PTR(dev->hspi, error, spi);
    
    //���DRDY#�źŽӿ�
    CHECK_PTR(dev->drdy_GPIO, error, drdy);
    
    //���dataָ��ǿ�
    CHECK_PTR(data, error, set);
    
    //�ȴ� DRDY#�ź���Ч���͵�ƽ��
    uint32_t time = max_wait_ms;
    
    

	
    do
    {   
        if(!HAL_GPIO_ReadPin(dev->drdy_GPIO, dev->drdy_BIT))
            break;
        
        ADS1220_DELAY_1MS;
        time--;
        
    }while(time>0);
    
    //����ʱ��ֱ���˳�
    if(time == 0)
    {   
		ADS1220_PRINT_DEBUG("оƬ��%dms��δ����DRDY#�źţ���ȡADת������ʧ��\n", max_wait_ms);
        error.timeout = 1;
        return error;
    }
    
    //��ʼ��ȡ
    uint8_t rx_data[3]={0};
    uint8_t tx_data[3] = {0XFF, 0XFF, 0XFF};
    HAL_SPI_TransmitReceive(dev->hspi, tx_data, rx_data, 3,100);

    //ת��
    *data = (rx_data[0]<<24) + (rx_data[1]<<16) + (rx_data[2]<<8);
    if(*data <0 ){
        printf("error\n");
    }
    *data = *data >> 8;
    return error;
}

/*
    ��ADCת�����24bit���ݸ��ݸ�������תΪģ���ѹֵ
    data_in�ĸ�8λ��Ч��
*/
static ADS1220_Error_t ADS1220_ConvData(ADS1220_t* dev, const int32_t* data_in, double* data_out)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
    //��� data_in ָ��ǿ�
    CHECK_PTR(data_in, error, set);
    
    //��� data_out ָ��ǿ�
    CHECK_PTR(data_out, error, set);
    
    //ת�����ݣ���Ҫȷ����׼��ѹԴ��Դ������ϵ��
    ADS1220_VREF_t vref_set = dev->regs.reg2.vref;
    double ref_volt = 0;
    
    switch(vref_set)
    {
        case VREF_INTERNAL:
            ref_volt = ADS1220_INTERNAL_VREF;
            break;
        
        case VREF_REF0:
            ref_volt = dev->monitor.vref[0];
            break;
        
        case VREF_REF1:
            ref_volt = dev->monitor.vref[1];
            break;
        
        case VREF_POWER:
            ref_volt = dev->monitor.pwr;
            break;
        
        default:
            error.set = 1;
            return error;
            break;
    }
    
    //�ȼ�ȥƫ����
    *data_out = *data_in - dev->monitor.volt_bias;
    
    //�ٽ�������תΪģ����
    *data_out = (*data_out * ref_volt) / ADS1220_MAX_ADC_DATA;
    
    //���������
    //���PGA�򿪣��Ҳ�Ϊ1������Ҫ��������
    //���PGA�������4����ô����PGA�Ƿ�򿪣�����������Ч��
	
	//������������źţ���PGA���ã�����ֻ����1\2\4
	if(dev->regs.reg0.mux >= AIN0_AVSS && dev->regs.reg0.mux <= AIN3_AVSS)
	{
		if(dev->regs.reg0.gain >= GAIN_8)
			dev->regs.reg0.gain = GAIN_4;
		else if(dev->regs.reg0.gain != GAIN_1)
			*data_out /= pow(2, dev->regs.reg0.gain);
	}
    
	//�������ʹ��PGA
	else if(dev->regs.reg0.pga_bypass == PGA_ON && dev->regs.reg0.gain != GAIN_1)
        *data_out /= pow(2, dev->regs.reg0.gain);
     
    return error;
}

/*
    ����ϵͳ����ƫ��У׼���ɼ�������samp_cntָ����������ƽ��ֵ��
    ���幦������ cmd 

    ������ʹ�ü�⹦��ʱ����������üĴ����������������Զ���· PGA ������������Ϊ 1��
    ��ע�⣬ϵͳ��⹦�ܽ��ṩ���Խ�������Ǿ��ܲ�����
*/

static ADS1220_Error_t ADS1220_VoltMon(ADS1220_t* dev, const ADS1220_SysMon_CMD_t cmd, const uint8_t samp_cnt)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
     
    //���������� 
    if(samp_cnt == 0)
    {
        error.set = 1;
        return error;
    }
    
    //�ȱ��浱ǰ����
    const ADS1220_Reg_t regs_backup = dev->regs;
    
    //���� cmd ���üĴ���0
    switch(cmd)
    {
        case SYSMON_CMD_BIAS:
             dev->regs.reg0.mux = ALL_MIDDLE;
			break;
        
        case SYSMON_CMD_REF0:
            dev->regs.reg0.mux = REFP_REFN;
            dev->regs.reg2.vref = VREF_REF0;
			break;
        
        case SYSMON_CMD_REF1:
            dev->regs.reg0.mux = REFP_REFN;
            dev->regs.reg2.vref = VREF_REF1;
			break;
        
        case SYSMON_CMD_AVDD:
            dev->regs.reg0.mux = AVDD_AVSS;
			break;
		
		case SYSMON_CMD_TEMP://�����¶ȴ���ģʽ���Ĵ���0��������Ӱ��
			break;
    }
    
    //��������ת��ģʽ
    dev->regs.reg1.cm = CONTINUE;
    
    //����ռ�ձ�ģʽ�����������
    dev->regs.reg1.mode = DUTY;
    dev->regs.reg1.dr = DUTY_5SPS; //��������FIR�˲���
	
	if(cmd == SYSMON_CMD_TEMP)	
		dev->regs.reg1.ts = TS_ON;//�����¶ȴ���ģʽ
	else
		dev->regs.reg1.ts = TS_OFF;//�ر��¶ȴ���ģʽ
	
    //�ر��ջٵ���Դ
    dev->regs.reg1.bcs = BCS_OFF;
    
    //����FIR�˲���, ͬʱ����50Hz��60Hz�ĸ���
    dev->regs.reg2.fir = FIR_ALL;
    
    //�رռ�������Դ
    dev->regs.reg2.idac = IDAC_OFF;
    dev->regs.reg3.i1mux = MUX_OFF;
    dev->regs.reg3.i2mux = MUX_OFF;
    
    //д��������
    error.data |= ADS1220_WriteReg(dev, ALLREG).data;  
          
    if(error.data)
    {
        //��ԭ����
        dev->regs = regs_backup;
        ADS1220_WriteReg(dev, ALLREG);
        return error;
    }   
    
    //�����������START����
    error.data |= ADS1220_Start(dev).data;
    if(error.data)
    {
        //��ԭ����
        dev->regs = regs_backup;
        ADS1220_WriteReg(dev, ALLREG);
        return error;
    }   
    
    //��ʼ����
    double data = 0.0;
    uint8_t success_times = 0;
    int32_t da_data = 0;
	
    for(uint8_t i=0; i<samp_cnt; i++)
    {
		error = ADS1220_GetAdcData(dev, &da_data, 1000);
        if(error.data)
		{       
			ADS1220_PRINT_DEBUG("�ɼ�����ʧ�ܣ������룺%d\n", error.data);	
			continue;
		}
			
        
        //�ɹ��ɼ���һ������
        success_times++;
		
		 switch(cmd)
		{
			case SYSMON_CMD_BIAS:
				data += da_data; //�����ƫ��У׼��ֱ����Ӽ���
				break;
        
			case SYSMON_CMD_REF0:       
			case SYSMON_CMD_REF1:
			case SYSMON_CMD_AVDD:
				data += (da_data * ADS1220_INTERNAL_VREF * 4.0) / ADS1220_MAX_ADC_DATA;//����ǲ��ѹ�������ת��
				break;
		
			case SYSMON_CMD_TEMP:
				data += ((da_data<<8) >> 18) * ADS1220_TEMP_FACTOR; //����ǲ��¶ȣ���ô�����¶�ת��ϵ��
			break;
		}
    }
         
    //��û�вɼ��ɹ�������ֱ�ӷ��س�ʱ
    if(!success_times)
    {
        error.timeout = 1;
        return error;
    }    
    
    //����ģ���Դ��ѹ
    if(success_times > 1)
        data /= success_times;
    
    //����volt_type���¼�����ֵ
    switch(cmd)
    {
        case SYSMON_CMD_BIAS:
             dev->monitor.volt_bias = (int32_t)round(data);
			ADS1220_PRINT_DEBUG("ƫ��У׼���ɹ��ʣ�%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
        
        case SYSMON_CMD_REF0:
            dev->monitor.vref[0] = data;
			ADS1220_PRINT_DEBUG("����ⲿ��׼��ѹ0���ɹ��ʣ�%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
        
        case SYSMON_CMD_REF1:
            dev->monitor.vref[1] = data;
			ADS1220_PRINT_DEBUG("����ⲿ��׼��ѹ1���ɹ��ʣ�%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
        
        case SYSMON_CMD_AVDD:
            dev->monitor.pwr= data;
			ADS1220_PRINT_DEBUG("���ģ�⹩���ѹ���ɹ��ʣ�%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
		
		case SYSMON_CMD_TEMP:
			dev->monitor.temp = data;
			ADS1220_PRINT_DEBUG("���оƬ�¶ȣ��ɹ��ʣ�%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
    }
    
    //��ԭ����
    dev->regs = regs_backup;
    ADS1220_WriteReg(dev, ALLREG);
    
    //�����������ֹͣת��
    ADS1220_PowerDown(dev);
	 
    return error;
}

/*
    ��ȡģ���Դ��ѹ���ɼ�������samp_cntָ����������ƽ��ֵ
    ����ģ���Դ (MUX[3:0] = 1101) ʱ���ó���ת�����ԼΪ (AVDD �C AVSS) / 4��
    �����Ƿ������üĴ�����ѡ���׼Դ (VREF[1:0])����������ʹ�� 2.048V �ڲ���׼��ѹ���в�����
*/
static ADS1220_Error_t ADS1220_GetPwrVolt(ADS1220_t* dev, const uint8_t samp_cnt)
 {
    return ADS1220_VoltMon(dev, SYSMON_CMD_AVDD, samp_cnt);
 }
 
 /*
    ��ȡ�ⲿ��׼Դ��Դ��ѹ���ɼ�������samp_cntָ����������ƽ��ֵ
    ����ǰ������Ҫ���û�׼ԴΪ�ⲿ��׼
*/
static ADS1220_Error_t ADS1220_GetRefVolt(ADS1220_t* dev, const uint8_t samp_cnt)
 {
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);
    
	ADS1220_VREF_t vref_type = dev->regs.reg2.vref;
	switch(vref_type)
	{
		case VREF_REF0:
			error = ADS1220_VoltMon(dev, SYSMON_CMD_REF0, samp_cnt);
			break;
		
		case VREF_REF1:
			error = ADS1220_VoltMon(dev, SYSMON_CMD_REF1, samp_cnt);
			break;
		
		default:
			error.set = 1; //ֻ��ʹ�����ⲿ��׼��ѹԴ�ż��
			break;
	}
    
    return error;
 }
 
 /*
    ��ȡ�¶ȣ��ɼ�������samp_cntָ����������ƽ��ֵ
 
    ADS1220 ������һ�������¶ȴ�������ͨ�������üĴ����� TS λ�� 1 ��ʹ���¶ȴ�����ģʽ��
    ���¶ȴ�����ģʽ�£����üĴ��� 0 �����ò������κ�Ӱ�죬������ʹ���ڲ���׼���в���������ѡ��׼��ѹԴ�޹ء�
    �¶ȶ���������ģ��������������ȡת������Ĺ�����ͬ���¶������� 14 λ������֣��� 24 λת���������롣
    ���ݴ������Ч�ֽ� (MSB) ��ʼ���������ȡ�����������ֽڣ�ǰ 14 λ����ָ���¶Ȳ��������
    һ�� 14 λ LSB ����0.03125��C�������Զ����Ʋ�����ʽ��ʾ����оƬ�ֲ�ı� 12 ��ʾ��

 */
static ADS1220_Error_t ADS1220_GetTemp(ADS1220_t* dev, const uint8_t samp_cnt)
 {
    return ADS1220_VoltMon(dev, SYSMON_CMD_TEMP, samp_cnt);
 }
 
 /*
    ����ƫ��У׼���ɼ�������samp_cntָ����������ƽ��ֵ��
*/

static ADS1220_Error_t ADS1220_GetBias(ADS1220_t* dev, const uint8_t samp_cnt)
{
	return ADS1220_VoltMon(dev, SYSMON_CMD_BIAS, samp_cnt);
}

/*
    (4)������ʼ��ADS1220�����ĺ����ӿ�
*/

ADS1220_Error_t ADS1220_API_INIT(ADS1220_t* dev)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //����豸ָ��ǿ�
    CHECK_PTR(dev, error, dev);

    dev->Init       = ADS1220_Init;
    dev->DeInit     = ADS1220_DeInit;
    dev->ReadReg    = ADS1220_ReadReg;
    dev->WriteReg   = ADS1220_WriteReg;
    dev->Start      = ADS1220_Start;
    dev->Stop       = ADS1220_Stop;
    dev->Reset      = ADS1220_Reset;
    dev->GetAdcData = ADS1220_GetAdcData;
    dev->ConvData   = ADS1220_ConvData;
    dev->GetPwrVolt = ADS1220_GetPwrVolt;
    dev->GetRefVolt = ADS1220_GetRefVolt;
    dev->GetTemp    = ADS1220_GetTemp;
	dev->GetBias    = ADS1220_GetBias;
    
    return error;
}


    