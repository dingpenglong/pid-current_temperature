#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "ads1220.h"
#include "ads1220_conf.h"


//打印调试信息
#ifdef ADS1220_PRINT_DEBUE_INFO 
#define ADS1220_PRINT_DEBUG(fmt,args...) do{printf("file:%s(%d) func %s:\n", __FILE__,__LINE__,  __FUNCTION__);printf(fmt, ##args);}while(0)
#else
#define ADS1220_PRINT_DEBUG(fmt,args...) 
#endif

/*
    （1）定义ADS1220的底层命令码
*/


//命令码
typedef enum _ADS1220_CMD
{
    CMD_RESET       = 0x06, //复位器件
    CMD_START       = 0x08, //启动或重启转换
    CMD_POWERDOWN   = 0x02, //进入掉电模式
    CMD_RDATA       = 0x10, //通过命令读取数据
    CMD_RREG        = 0x20, //读取 nn 寄存器（起始地址：rr）: 0010 rrnn, 操作数：rr = 配置寄存器（00 至 11），nn = 字节数 – 1（00 至 11）  
    CMD_WREG        = 0x40, //写入 nn 寄存器（起始地址：rr）: 0100 rrnn, 操作数：rr = 配置寄存器（00 至 11），nn = 字节数 – 1（00 至 11）
}ADS1220_CMD_t;

//系统监测命令码
typedef enum _ADS1220_SysMon_CMD
{
	SYSMON_CMD_AVDD,  //监测模拟电源电压：AVDD - AVSS
	SYSMON_CMD_REF0,  //监测外部基准电压0：VREFP0 - VREFN0	
	SYSMON_CMD_REF1,  //监测外部基准电压1：VREFP0 - VREFN0
	SYSMON_CMD_BIAS,  //偏移校准
	SYSMON_CMD_TEMP,  //温度传感
}ADS1220_SysMon_CMD_t;

//定义ADC芯片常量

//内部基准电压源为 2.048V
#define ADS1220_INTERNAL_VREF 2.048 

//ADC输出24位有符号数，其最大值就是2^23 - 1
#define ADS1220_MAX_ADC_DATA  0X800000 //2^23 - 1
#define ADS1220_TEMP_FACTOR   0.03125

/*
    （2）实现对ADS1220芯片的底层操作：读写寄存器操作，以及特殊命令操作（即ADS1220_CMD_t列出的命令）
*/

//检查指针非空，若为空则将对应标志位置1
#define CHECK_PTR(ptr, param, field)    do{\
                                            if(ptr == NULL) \
                                            { \
                                                param.field = 1;\
												ADS1220_PRINT_DEBUG("指针 %s 为空指针。\n", #ptr);\
                                                return param;\
                                            } \
                                        }while(0)
                                                              
                                        
//发送命令                              
static ADS1220_Error_t ADS1220_SendCMD(const ADS1220_t* dev, const ADS1220_CMD_t cmd)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
    
    //检查SPI接口非空
    CHECK_PTR(dev->hspi, error, spi);
    
    //发送命令，限时100ms
    uint8_t data = cmd;
    HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(dev->hspi, &data, 1, 100);
    
    //解析SPI传输的状态码
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
                                    
//重置芯片
//static ADS1220_Error_t ADS1220_Reset(ADS1220_t* dev)
//{
//    ADS1220_Error_t error;
//    
//	//在重置芯片之前检查芯片是否上电完成
//	int i=0;
//	while(HAL_GPIO_ReadPin(dev->drdy_GPIO, dev->drdy_BIT))
//	{
//		ADS1220_DELAY_1MS;
//		
//		i++;
//		
//		//如果超过100ms都未拉低DRDY#信号，则器件可能不处于上电复位状态
//		if(i == 100)
//		{
//			ADS1220_PRINT_DEBUG("芯片在100ms内未完成上电复位，主动发送复位命令\n");
//			ADS1220_SendCMD(dev, CMD_RESET);
//		}
//		
//		//超过1秒未重置成功,则视为超时
//		else if(i == 1000)
//		{
//			ADS1220_PRINT_DEBUG("芯片在1000ms内未完成上电复位，复位超时\n");
//			error.timeout = 1;
//			return error;
//		}		
//	}
//	
//    error = ADS1220_SendCMD(dev, CMD_RESET);
//	
//	//检查芯片是否上电完成
//	i=0;
//	while(HAL_GPIO_ReadPin(dev->drdy_GPIO, dev->drdy_BIT))
//	{
//		ADS1220_DELAY_1MS;
//		i++;
//		
//		if(i == 1000)
//		{
//			ADS1220_PRINT_DEBUG("发送复位命令后，芯片在1000ms内未完成上电复位，等待超时\n");
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
    
	//在重置芯片之前检查芯片是否上电完成
    /*
    器件在上电过程中执行复位。复位过程耗时约 50us。
    上电后，该器件以默认寄存器设置执行单次转换，然后进入低功耗状态。
    完成转换后，DRDY 引脚由高电平转换为低电平。
    */
    // 若ADS1220正处于上电和复位过程中，则DRDY信号一定为高电平
    // 根据其数据手册，这里需要最多延时100ms直至检测到DRDY信号下降沿
    uint16_t i=0;
    while(HAL_GPIO_ReadPin(dev->drdy_GPIO, dev->drdy_BIT))
	{
		ADS1220_DELAY_1MS;
        i++;
        
        // 若超过100ms还未检测到DRDY信号为低电平，说明ADS1220已经正常运行，且未进行转换
        if(i >= 100)
            break;
	}
	
    dev->regs.reg1.cm = CONTINUE;
    dev->WriteReg(dev,REG1);
    // 手动发送复位命令,等待1ms再发送开始转换命令
    error = ADS1220_SendCMD(dev, CMD_RESET);
    for(int i =0; i<100;i++)
    ADS1220_DELAY_1MS; 
	// 检查芯片是否复位完
    dev->ReadReg(dev,REG1); 
    if(dev->regs.reg1.cm != SINGLE)
    {
       ADS1220_PRINT_DEBUG("发送复位命令后，芯片在1000ms内未完成上电复位，等待超时\n");
       error.timeout =1;
       return error;
    }
    return error;
}

//启动或重启转换
static ADS1220_Error_t ADS1220_Start(ADS1220_t* dev)
{
    return ADS1220_SendCMD(dev, CMD_START);
}    

//进入掉电模式
static ADS1220_Error_t ADS1220_PowerDown(ADS1220_t* dev)
{
    return  ADS1220_SendCMD(dev, CMD_POWERDOWN);
}

//指示ADC提供最新转换数据
static ADS1220_Error_t ADS1220_ReadData(ADS1220_t* dev)
{
    return  ADS1220_SendCMD(dev, CMD_RDATA);
}

//读取寄存器数据
static ADS1220_Error_t ADS1220_ReadReg(ADS1220_t* dev, const ADS1220_RegList_t reg_list)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
    
    //若无需读取，直接返回
    if(reg_list == 0)
    {
        error.set = 1;
        return error;
    }
    
    //检查SPI接口非空
    CHECK_PTR(dev->hspi, error, spi);
    
    //检查需要读取的寄存器
    uint8_t tx_data[2] = {CMD_RREG, 0XFF};
    uint8_t rx_data[2] = {0XFF, 0XFF};
    
    for(uint8_t i=1, ind=0; i<=reg_list; i=i<<1)
    {
        if(reg_list & i)
        {
            //构造完整命令字
            tx_data[0] = CMD_RREG | (ind<<2);
            
            //全双工操作，接收到的有效数据位于rx_data[1]中，限时1000ms
            HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(dev->hspi, tx_data, rx_data, 2, 1000);
            
            //解析SPI传输的状态码
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
            
            //如果存在错误，视为本次读取无效
            if(error.data != 0)
            {
                ind++;
                continue;
            }
            
            //根据寄存器偏移地址赋值
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
        
        //寄存器偏移地址自增
        ind++;
    }
    
    return error;
}

//写入寄存器数据
static ADS1220_Error_t ADS1220_WriteReg(ADS1220_t* dev, const ADS1220_RegList_t reg_list)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
    
    //若无需写入，直接返回
    if(reg_list == 0)
    {
        error.set = 1;
        return error;
    }
    
    //检查SPI接口非空
    CHECK_PTR(dev->hspi, error, spi);
    
    //检查需要写入的寄存器
    uint8_t tx_data[2] = {CMD_WREG, 0XFF};

    
    for(uint8_t i=1, ind=0; i<=reg_list; i=i<<1)
    {
        if(reg_list & i)
        {
            //构造完整命令字
            tx_data[0] = CMD_WREG | (ind<<2);
            
            //根据寄存器偏移地址赋值
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
            
            //发送数据，限时200ms
            HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(dev->hspi, tx_data, 2, 200);
            
            //解析SPI传输的状态码
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
        
        //寄存器偏移地址自增
        ind++;
    }
    
    return error;
}


/*
    （3）实现应用层接口
*/

/*
    初始化设备   
*/
static ADS1220_Error_t ADS1220_Init(ADS1220_t* dev, const SPI_HandleTypeDef* hspi, 
    const GPIO_TypeDef* drdy_GPIO, const uint32_t drdy_BIT, void(*Func_CallBack)(void))
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
    
    //检查SPI接口非空
    CHECK_PTR(hspi, error, spi);
    
    //检查DRDY信号接口非空
    CHECK_PTR(drdy_GPIO, error, drdy);
    
    //绑定硬件接口
    //DRDY#信号
    dev->drdy_GPIO = (GPIO_TypeDef*)drdy_GPIO;  //DRDY#信号所属GPIO
    dev->drdy_BIT  = drdy_BIT & 0XFFFF;         //DRDY#信号的位号
    
    //SPI接口
    dev->hspi = (SPI_HandleTypeDef*)hspi;
    
    //初始化寄存器结构体成员，默认值依据芯片数据手册
    dev->regs.reg0.data = 0;
    dev->regs.reg1.data = 0;
    dev->regs.reg2.data = 0;
    dev->regs.reg3.data = 0;
    
    //初始化监视器结构体成员
    dev->monitor.pwr = 0;
    dev->monitor.temp = 0;
    dev->monitor.vref[0] = 0;
    dev->monitor.vref[1] = 0;
    
    //重置芯片
    error = dev->Reset(dev);
    
	//获取芯片监视数据
	dev->GetBias(dev, 5);
	dev->GetPwrVolt(dev, 5);     
    dev->GetTemp(dev, 5);
    
    //若存在回调函数，调用回调函数
    if(Func_CallBack != NULL)
        Func_CallBack();
    
    return error;
}


/*
    反初始化设备
*/
static ADS1220_Error_t ADS1220_DeInit(ADS1220_t* dev, void(*Func_CallBack)(void))
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
    
    //重置芯片
    error = ADS1220_Reset(dev);
    
    //解绑硬件接口
    //DRDY#信号
    dev->drdy_GPIO = NULL;
    dev->drdy_BIT  = NULL;
    
    //SPI接口
    dev->hspi = NULL;
    
    //初始化寄存器结构体成员，默认值依据芯片数据手册
    dev->regs.reg0.data = 0;
    dev->regs.reg1.data = 0;
    dev->regs.reg2.data = 0;
    dev->regs.reg3.data = 0;
    
    //初始化监视器结构体成员
    dev->monitor.pwr = 0;
    dev->monitor.temp = 0;
    dev->monitor.vref[0] = 0;
    dev->monitor.vref[1] = 0;
    dev->monitor.volt_bias = 0;
    
    //解绑函数指针
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
    
    //若存在回调函数，调用回调函数
    if(Func_CallBack != NULL)
        Func_CallBack();
    
    return error;
}

/*
    停止转换
    若芯片工作于单次转换模式，则不会执行任何操作，因为单次模式下转换完后自动停止
    若芯片处于连续转换模式，则向芯片发出掉电命令
*/
static ADS1220_Error_t ADS1220_Stop(ADS1220_t* dev)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
    
    //检查芯片工作状态
    error = ADS1220_ReadReg(dev, REG1);
    if(error.data)
        return error;
    
    if(dev->regs.reg1.cm == SINGLE)
        return error;
    
    //处于连续转换模式，发出掉电命令
    error = ADS1220_PowerDown(dev);
    
    return error;
}

/*
    尝试读取一次ADC的转换数据直至超时，调用该函数前必须保证ADC已处于正常转换状态
    读取的ADC转换后的原始数据，此时data指针指向 int32_t 类型,高8位无效；
*/
static ADS1220_Error_t ADS1220_GetAdcData(ADS1220_t* dev, int32_t* data, const uint32_t max_wait_ms)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
    
    //检查SPI接口非空
    CHECK_PTR(dev->hspi, error, spi);
    
    //检查DRDY#信号接口
    CHECK_PTR(dev->drdy_GPIO, error, drdy);
    
    //检查data指针非空
    CHECK_PTR(data, error, set);
    
    //等待 DRDY#信号有效（低电平）
    uint32_t time = max_wait_ms;
    
    

	
    do
    {   
        if(!HAL_GPIO_ReadPin(dev->drdy_GPIO, dev->drdy_BIT))
            break;
        
        ADS1220_DELAY_1MS;
        time--;
        
    }while(time>0);
    
    //若超时，直接退出
    if(time == 0)
    {   
		ADS1220_PRINT_DEBUG("芯片在%dms内未拉低DRDY#信号，获取AD转换数据失败\n", max_wait_ms);
        error.timeout = 1;
        return error;
    }
    
    //开始读取
    uint8_t rx_data[3]={0};
    uint8_t tx_data[3] = {0XFF, 0XFF, 0XFF};
    HAL_SPI_TransmitReceive(dev->hspi, tx_data, rx_data, 3,100);

    //转换
    *data = (rx_data[0]<<24) + (rx_data[1]<<16) + (rx_data[2]<<8);
    if(*data <0 ){
        printf("error\n");
    }
    *data = *data >> 8;
    return error;
}

/*
    将ADC转换后的24bit数据根据给定配置转为模拟电压值
    data_in的高8位无效；
*/
static ADS1220_Error_t ADS1220_ConvData(ADS1220_t* dev, const int32_t* data_in, double* data_out)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
    
    //检查 data_in 指针非空
    CHECK_PTR(data_in, error, set);
    
    //检查 data_out 指针非空
    CHECK_PTR(data_out, error, set);
    
    //转换数据，需要确定基准电压源来源和增益系数
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
    
    //先减去偏移量
    *data_out = *data_in - dev->monitor.volt_bias;
    
    //再将数字量转为模拟量
    *data_out = (*data_out * ref_volt) / ADS1220_MAX_ADC_DATA;
    
    //最后考虑增益
    //如果PGA打开，且不为1，则需要除以增益
    //如果PGA增益大于4，那么无论PGA是否打开，都会有增益效果
	
	//如果测量单端信号，则PGA禁用，增益只能是1\2\4
	if(dev->regs.reg0.mux >= AIN0_AVSS && dev->regs.reg0.mux <= AIN3_AVSS)
	{
		if(dev->regs.reg0.gain >= GAIN_8)
			dev->regs.reg0.gain = GAIN_4;
		else if(dev->regs.reg0.gain != GAIN_1)
			*data_out /= pow(2, dev->regs.reg0.gain);
	}
    
	//否则必须使能PGA
	else if(dev->regs.reg0.pga_bypass == PGA_ON && dev->regs.reg0.gain != GAIN_1)
        *data_out /= pow(2, dev->regs.reg0.gain);
     
    return error;
}

/*
    进行系统监测和偏移校准，采集次数由samp_cnt指定，计算其平均值。
    具体功能依据 cmd 

    无论在使用监测功能时如何设置配置寄存器，该器件均会自动旁路 PGA 并将增益设置为 1。
    请注意，系统监测功能仅提供粗略结果，并非精密测量。
*/

static ADS1220_Error_t ADS1220_VoltMon(ADS1220_t* dev, const ADS1220_SysMon_CMD_t cmd, const uint8_t samp_cnt)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
    CHECK_PTR(dev, error, dev);
     
    //检查采样次数 
    if(samp_cnt == 0)
    {
        error.set = 1;
        return error;
    }
    
    //先保存当前配置
    const ADS1220_Reg_t regs_backup = dev->regs;
    
    //依据 cmd 配置寄存器0
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
		
		case SYSMON_CMD_TEMP://若是温度传感模式，寄存器0的配置无影响
			break;
    }
    
    //开启连续转换模式
    dev->regs.reg1.cm = CONTINUE;
    
    //开启占空比模式，增加信噪比
    dev->regs.reg1.mode = DUTY;
    dev->regs.reg1.dr = DUTY_5SPS; //便于设置FIR滤波器
	
	if(cmd == SYSMON_CMD_TEMP)	
		dev->regs.reg1.ts = TS_ON;//开启温度传感模式
	else
		dev->regs.reg1.ts = TS_OFF;//关闭温度传感模式
	
    //关闭烧毁电流源
    dev->regs.reg1.bcs = BCS_OFF;
    
    //开启FIR滤波器, 同时抑制50Hz和60Hz的干扰
    dev->regs.reg2.fir = FIR_ALL;
    
    //关闭激励电流源
    dev->regs.reg2.idac = IDAC_OFF;
    dev->regs.reg3.i1mux = MUX_OFF;
    dev->regs.reg3.i2mux = MUX_OFF;
    
    //写入新配置
    error.data |= ADS1220_WriteReg(dev, ALLREG).data;  
          
    if(error.data)
    {
        //还原设置
        dev->regs = regs_backup;
        ADS1220_WriteReg(dev, ALLREG);
        return error;
    }   
    
    //随后立即发送START命令
    error.data |= ADS1220_Start(dev).data;
    if(error.data)
    {
        //还原设置
        dev->regs = regs_backup;
        ADS1220_WriteReg(dev, ALLREG);
        return error;
    }   
    
    //开始测量
    double data = 0.0;
    uint8_t success_times = 0;
    int32_t da_data = 0;
	
    for(uint8_t i=0; i<samp_cnt; i++)
    {
		error = ADS1220_GetAdcData(dev, &da_data, 1000);
        if(error.data)
		{       
			ADS1220_PRINT_DEBUG("采集数据失败，错误码：%d\n", error.data);	
			continue;
		}
			
        
        //成功采集到一次数据
        success_times++;
		
		 switch(cmd)
		{
			case SYSMON_CMD_BIAS:
				data += da_data; //如果是偏移校准，直接相加即可
				break;
        
			case SYSMON_CMD_REF0:       
			case SYSMON_CMD_REF1:
			case SYSMON_CMD_AVDD:
				data += (da_data * ADS1220_INTERNAL_VREF * 4.0) / ADS1220_MAX_ADC_DATA;//如果是测电压，则进行转换
				break;
		
			case SYSMON_CMD_TEMP:
				data += ((da_data<<8) >> 18) * ADS1220_TEMP_FACTOR; //如果是测温度，那么乘以温度转换系数
			break;
		}
    }
         
    //若没有采集成功过，则直接返回超时
    if(!success_times)
    {
        error.timeout = 1;
        return error;
    }    
    
    //更新模拟电源电压
    if(success_times > 1)
        data /= success_times;
    
    //依据volt_type更新监测对象值
    switch(cmd)
    {
        case SYSMON_CMD_BIAS:
             dev->monitor.volt_bias = (int32_t)round(data);
			ADS1220_PRINT_DEBUG("偏移校准，成功率：%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
        
        case SYSMON_CMD_REF0:
            dev->monitor.vref[0] = data;
			ADS1220_PRINT_DEBUG("监测外部基准电压0，成功率：%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
        
        case SYSMON_CMD_REF1:
            dev->monitor.vref[1] = data;
			ADS1220_PRINT_DEBUG("监测外部基准电压1，成功率：%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
        
        case SYSMON_CMD_AVDD:
            dev->monitor.pwr= data;
			ADS1220_PRINT_DEBUG("监测模拟供电电压，成功率：%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
		
		case SYSMON_CMD_TEMP:
			dev->monitor.temp = data;
			ADS1220_PRINT_DEBUG("监测芯片温度，成功率：%.2f%%\n", (success_times*100.0)/samp_cnt);
			break;
    }
    
    //还原设置
    dev->regs = regs_backup;
    ADS1220_WriteReg(dev, ALLREG);
    
    //发出掉电命令，停止转换
    ADS1220_PowerDown(dev);
	 
    return error;
}

/*
    获取模拟电源电压，采集次数由samp_cnt指定，计算其平均值
    测量模拟电源 (MUX[3:0] = 1101) 时，得出的转换结果约为 (AVDD – AVSS) / 4。
    无论是否在配置寄存器中选择基准源 (VREF[1:0])，该器件均使用 2.048V 内部基准电压进行测量。
*/
static ADS1220_Error_t ADS1220_GetPwrVolt(ADS1220_t* dev, const uint8_t samp_cnt)
 {
    return ADS1220_VoltMon(dev, SYSMON_CMD_AVDD, samp_cnt);
 }
 
 /*
    获取外部基准源电源电压，采集次数由samp_cnt指定，计算其平均值
    调用前提是需要设置基准源为外部基准
*/
static ADS1220_Error_t ADS1220_GetRefVolt(ADS1220_t* dev, const uint8_t samp_cnt)
 {
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
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
			error.set = 1; //只有使用了外部基准电压源才监测
			break;
	}
    
    return error;
 }
 
 /*
    获取温度，采集次数由samp_cnt指定，计算其平均值
 
    ADS1220 集成了一个精密温度传感器。通过将配置寄存器的 TS 位置 1 可使能温度传感器模式。
    在温度传感器模式下，配置寄存器 0 的设置不产生任何影响，该器件使用内部基准进行测量，与所选基准电压源无关。
    温度读数过程与模拟输入启动并读取转换结果的过程相同。温度数据以 14 位结果呈现，与 24 位转换结果左对齐。
    数据从最高有效字节 (MSB) 开始输出。当读取这三个数据字节，前 14 位用于指定温度测量结果。
    一个 14 位 LSB 等于0.03125°C。负数以二进制补码形式表示，如芯片手册的表 12 所示。

 */
static ADS1220_Error_t ADS1220_GetTemp(ADS1220_t* dev, const uint8_t samp_cnt)
 {
    return ADS1220_VoltMon(dev, SYSMON_CMD_TEMP, samp_cnt);
 }
 
 /*
    进行偏移校准，采集次数由samp_cnt指定，计算其平均值。
*/

static ADS1220_Error_t ADS1220_GetBias(ADS1220_t* dev, const uint8_t samp_cnt)
{
	return ADS1220_VoltMon(dev, SYSMON_CMD_BIAS, samp_cnt);
}

/*
    (4)给出初始化ADS1220驱动的函数接口
*/

ADS1220_Error_t ADS1220_API_INIT(ADS1220_t* dev)
{
    ADS1220_Error_t error;
    error.data = 0;
    
    //检查设备指针非空
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


    