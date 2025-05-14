#ifndef __ADS1220_H__
#define __ADS1220_H__
/*
@filename   ads1220.h

@brief		基于SPI的ADS1220驱动头文件，需要支持HAL库

@time		2024/08/25

@author		黎建

@version    1.0

@attention  这里SPI接口最高时钟速率为50MHz，该器件的 SPI 兼容串行接口适用于读取转换数据、读写器件配置寄存器以及控制器件工作状态。
            仅支持 SPI 模式1（CPOL = 0，CPHA = 1）。该接口由五条控制线（CS#、SCLK、DIN、DOUT/DRDY# 和 DRDY#）组成。
            
            注意，本驱动仅支持4线制（CS#、SCLK、DIN、DOUT）硬件SPI接口，片选信号由HAL库自动控制
            
            注意，由于使用了DRDY#信号，实际上没有用到RDATA命令
            
            注意，本驱动使用内部时钟源 4.096MHz
       
            注意，本驱动不负责初始化SPI硬件接口，也不负责初始化DRDY#信号的硬件接口，这两者应由用户参照芯片手册要求自行初始化！
            可以回调函数的形式初始化两者的硬件接口。
            

*/
#ifdef __cplusplus
extern "C" {
#endif

//引入系统头文件
#include "stm32g4xx_hal.h"

/*
    （1）定义ADS1220的寄存器列表和对应寄存器的字段。
*/

//寄存器列表
typedef enum _ADS1220_RegList
{
    REG0    = 0x01,
    REG1    = 0x02,
    REG2    = 0x04,
    REG3    = 0x08,
    ALLREG  = 0x0F  //所有寄存器
}ADS1220_RegList_t;

//寄存器0的字段描述
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

//寄存器1的字段描述
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

//寄存器2的字段描述
typedef union _ADS1220_Reg2
{
    struct
    {
        uint8_t idac    : 3;
        uint8_t psw     : 1;
        uint8_t fir     : 2; //芯片手册上的50/60[1:0]
        uint8_t vref    : 2;
    };
    
    uint8_t data;
}ADS1220_Reg2_t;

//寄存器3的字段描述
typedef union _ADS1220_Reg3
{
    struct
    {
        uint8_t         : 1; //占位
        uint8_t drdym   : 1;
        uint8_t i2mux   : 3; 
        uint8_t i1mux   : 3;
    };
    
    uint8_t data;
}ADS1220_Reg3_t;

//总寄存器字段描述
typedef struct _ADS1220_Reg
{
    ADS1220_Reg0_t reg0;
    ADS1220_Reg1_t reg1;
    ADS1220_Reg2_t reg2;
    ADS1220_Reg3_t reg3;
}ADS1220_Reg_t; 


/*
    （2）定义寄存器设置的常量
*/

/*
    MUX：输入多路复用器配置，这些位配置输入多路复用器。
    对于 AINN = AVSS 的设置，PGA 必须禁用 (PGA_BYPASS = 1)，并且仅可使用增益 1、2 和 4。

    测量模拟电源 (MUX[3:0] = 1101) 时，得出的转换结果约为 (AVDD – AVSS) / 4。
    无论是否在配置寄存器中选择基准源 (VREF[1:0])，该器件均使用 2.048V 内部基准电压进行测量。
    监测两外部基准电压源 (MUX[3:0] = 1100) 的其中之一时，结果约为 (V(REFPx) – V(REFNx)) / 4。
    REFPx 和 REFNx表示在配置寄存器中选择的外部基准输入对 (VREF[1:0])。该器件自动使用内部基准进行测量。
*/
typedef enum _ADS1220_MUX
{
    AIN0_AIN1 = 0x0 ,   //默认设置, AINP=AIN0， AINN=AIN1
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
    REFP_REFN = 0xc ,   //(V(REFPx) – V(REFNx)) / 4， 监视参考电源电压（旁路 PGA）
    AVDD_AVSS = 0xd ,   //(AVDD – AVSS) / 4 ，监视模拟电源电压（旁路 PGA）
    ALL_MIDDLE= 0xe     //AINP 和 AINN 短接至 (AVDD + AVSS) / 2，用于偏移校准
}ADS1220_MUX_t;

/*
    GAIN：增益配置，这些位用于配置器件增益。
    在不使用 PGA 的情况下，可使用增益 1、2 和 4。在这种情况下，通过开关电容结构获得增益。
    当配置为GAIN_X时， 实际增益为：2^(X-1)倍
*/

typedef enum _ADS1220_GAIN
{
    GAIN_1 = 0, //增益为1，默认设置
    GAIN_2 ,    //增益为2,以下以此类推
    GAIN_4 ,
    GAIN_8 ,
    GAIN_16 ,
    GAIN_32 ,
    GAIN_64 ,
    GAIN_128 
}ADS1220_GAIN_t;

/*
    PGA_BYPASS: 禁用和旁路内部低噪声PGA。
    禁用 PGA 会降低整体功耗，并可将共模电压范围 (VCM) 扩展为 AVSS – 0.1V 至AVDD + 0.1V。
    只能针对增益 1、2 和 4 禁用 PGA。
    无论 PGA_BYPASS 设置如何，都始终针对增益设置 8 至 128 启用 PGA。
*/
typedef enum _ADS1220_PGA_BYPASS
{
    PGA_ON = 0, //启用PGA（默认设置）
    PGA_OFF     //禁用（旁路）PGA
}ADS1220_PGA_BYPASS_t;

/*
    DR: 数据速率，这些位控制数据速率设置，取决于所选工作模式。
    芯片收据手册的表18列出了正常模式、占空比模式和 Turbo 模式对应的位设置。
    表18提供的数据速率使用内部振荡器或 4.096MHz 外部时钟进行计算。
    如果使用的是频率不为 4.096MHz 的外部时钟，则数据速率会按外部时钟频率成比例缩放。
*/
typedef enum _ADS1220_DR
{
    //正常模式
    NORMAL_20SPS    = 0,
    NORMAL_45SPS    = 1,
    NORMAL_90SPS    = 2,
    NORMAL_175SPS   = 3,
    NORMAL_330SPS   = 4,
    NORMAL_600SPS   = 5,
    NORMAL_1000SPS  = 6,
    
    //占空比模式
    DUTY_5SPS       = 0,
    DUTY_11_25SPS   = 1,
    DUTY_22_5SPS    = 2,
    DUTY_44SPS      = 3,
    DUTY_82_5SPS    = 4,
    DUTY_150SPS     = 5,
    DUTY_250SPS     = 6,
    
    //TURBO模式
    TURBO_40SPS    = 0,
    TURBO_90SPS    = 1,
    TURBO_180SPS   = 2,
    TURBO_350SPS   = 3,
    TURBO_660SPS   = 4,
    TURBO_1200SPS  = 5,
    TURBO_2000SPS  = 6
    
}ADS1220_DR_t;

/*
    MODE：工作模式，这些位控制器件所处的工作模式。
    
*/
typedef enum _ADS1220_MODE
{
    NORMAL = 0 ,    //正常模式（256kHz 调制器时钟，默认设置）
    DUTY ,          //占空比模式（内部占空比 1:4）
    TURBO           //Turbo 模式（512kHz 调制器时钟）
}ADS1220_MODE_t;

/*
    CM：转换模式，此位用于为器件设置转换模式。
*/
typedef enum _ADS1220_CM
{
    SINGLE = 0 ,    //单次模式（默认设置）
    CONTINUE        //连续转换模式
}ADS1220_CM_t;

/*
    TS：温度传感器模式，此位用于启用内部温度传感器以及将器件置于温度传感器模式下。
    启用温度传感器模式后，配置寄存器 0 的设置不会产生任何影响，
    器件会使用内部基准进行测量。
*/
typedef enum _ADS1220_TS
{
    TS_OFF = 0 ,    //禁用温度传感器（默认设置）
    TS_ON           //启用温度传感器
}ADS1220_TS_t;


/*
    BCS：烧毁电流源，此位用于控制 10μA 烧毁电流源。
    烧毁电流源可用于检测传感器故障（例如，传感器断路和短路）。
    TI 建议在执行精密测量的过程中禁用烧毁电流源，并且仅在测试传感器故障条件时进行启用。
*/
typedef enum _ADS1220_BCS
{
    BCS_OFF = 0 ,    //电流源关断（默认设置）
    BCS_ON           //电流源接通
}ADS1220_BCS_t;

/*
    VREF：基准电压选择，这些位用于选择转换所使用的基准电压源。
    
*/
typedef enum _ADS1220_VREF
{
    VREF_INTERNAL = 0 ,  //选择 2.048V 内部基准电压（默认设置）
    VREF_REF0 ,          //使用专用 REFP0 和 REFN0 输入选择的外部基准电压
    VREF_REF1 ,          //使用 AIN0/REFP1 和 AIN3/REFN1 输入选择的外部基准电压
    VREF_POWER           //用作基准的模拟电源 (AVDD – AVSS)
}ADS1220_VREF_t;

/*
    50/60：FIR 滤波器配置，这些位用于为内部 FIR 滤波器配置滤波器系数。
    在正常模式下，这些位仅与 20SPS 设置结合使用；
    在占空比模式下，这些位仅与5SPS 设置结合使用。对于所有其他数据速率，这些位均设置为 00。
*/
typedef enum _ADS1220_FIR
{
    FIR_OFF = 0 ,   //无 50Hz 或 60Hz 抑制（默认设置）
    FIR_ALL ,       //同时抑制 50Hz 和 60Hz
    FIR_50_Hz ,     //只抑制 50Hz
    FIR_60_Hz       //只抑制 60Hz
}ADS1220_FIR_t;

/*
    PSW：低侧电源开关配置，此位用于配置 AIN3/REFN1 和 AVSS 之间连接的低侧开关的行为。
    
*/
typedef enum _ADS1220_PSW
{
    PSW_OFF = 0 ,   //开关始终处于断开状态（默认设置）
    PSW_ON ,        //开关会在发送 START/SYNC 命令时自动闭合，并在发出 POWERDOWN 命令时自动断开。
}ADS1220_PSW_t;

/*
    IDAC：IDAC 电流设置，这些位用于为 IDAC1 和 IDAC2 激励电流源设置电流。
    
*/
typedef enum _ADS1220_IDAC
{
    IDAC_OFF = 0 ,   //关断（默认设置）
    IDAC_10_UA ,      //10uA
    IDAC_50_UA ,      //50uA
    IDAC_100_UA ,     //100uA
    IDAC_250_UA ,     //250uA
    IDAC_500_UA ,     //500uA
    IDAC_1000_UA ,    //1000uA
    IDAC_1500_UA      //1500uA
}ADS1220_IDAC_t;

/*
    IDACx路由配置，这些位用于选择 IDACx 将路由到的通道。
*/
typedef enum _ADS1220_IDACMUX
{
    MUX_OFF = 0,    //IDACx 已禁用（默认设置）
    AIN0 ,          //IDACx 已连接至 AIN0/REFP1
    AIN1 ,          //IDACx 已连接至 AIN1
    AIN2 ,          //IDACx 已连接至 AIN2
    AIN3 ,          //IDACx 已连接至 AIN3/REFN1
    REFP0 ,         //IDACx 已连接至 REFP0
    REFN0           //IDACx 已连接至 REFN0
}ADS1220_IDACMUX_t;

/*
    DRDYM：DRDY# 模式，该位用于控制新数据就绪时 DOUT/DRDY# 引脚的行为。
*/
typedef enum _ADS1220_DRDYM
{
    DRDYM_ONLY = 0,   //仅专用 DRDY# 引脚用于指示数据何时就绪（默认设置）
    DRDYM_BOTH ,      //同时通过 DOUT/DRDY# 和 DRDY# 指示数据就绪
}ADS1220_DRDYM_t;

/*
    (3)定义操作ADS1220时的错误类型
*/
typedef union _ADS1220_Error
{
    struct
    {
        uint8_t dev     : 1; //设备不存在
        uint8_t malloc  : 1; //申请动态空间失败
        uint8_t spi     : 1; //spi接口无效 
        uint8_t drdy    : 1; //DRDY#信号接口无效
        uint8_t set     : 1; //参数设置有误
        uint8_t timeout : 1; //操作超时
    };
    uint8_t data;
}ADS1220_Error_t;   

/*
    （4）定义ADS1220设备描述符
*/

//系统监视信息
typedef struct _ADS1220_Monitor
{
    //芯片的模拟供电电压 AVDD - AVSS
    double pwr;
    
    //芯片的外部基准电压 VREFPx - VREFNx
    double vref[2];
    
    //芯片的当前温度
    double temp;
    
    //芯片的电压偏移，实际转换的DA数字量应减去该偏移量
    int32_t volt_bias;
}ADS1220_Monitor_t;


//定义ADS1220设备描述符
typedef struct _ADS1220 ADS1220_t;

struct _ADS1220
{
    //寄存器结构体，用户通过读写该结构体来配置芯片
    ADS1220_Reg_t regs;
    
    //系统监视信息
    ADS1220_Monitor_t monitor;
    
    //DRDY#信号描述
    GPIO_TypeDef*   drdy_GPIO;    //DRDY#信号所属GPIO
    uint16_t        drdy_BIT;     //DRDY#信号的位号
    
    //SPI接口描述符
    SPI_HandleTypeDef* hspi;
    
    //用户接口
    
    /*
        初始化设备
    */
    ADS1220_Error_t (*Init)(ADS1220_t* dev, const SPI_HandleTypeDef* hspi, 
    const GPIO_TypeDef* drdy_GPIO, const uint32_t drdy_BIT, void(*Func_CallBack)(void));
   
    /*
        反初始化设备
    */
    ADS1220_Error_t (*DeInit)(ADS1220_t* dev, void(*Func_CallBack)(void));
    
    /*
        从ADS1220芯片读1至4个寄存器值，注意reg_list可以使用或运算来同时读多个寄存器的值
        注意，成功调用这一函数后将同步更新相应的regs的成员
    */
    ADS1220_Error_t (*ReadReg)(ADS1220_t* dev, const ADS1220_RegList_t reg_list);
    
    /*
        从ADS1220芯片写1至4个寄存器值，注意reg_list可以使用或运算来同时写多个寄存器的值
        注意，成功调用这一函数后将同步更新相应的regs的成员
    */
    ADS1220_Error_t (*WriteReg)(ADS1220_t* dev, const ADS1220_RegList_t reg_list);
    
    /*
        开始一次转换
    */
    ADS1220_Error_t (*Start)(ADS1220_t* dev);
    
    /*
        停止转换
        若芯片工作于单次转换模式，则不会执行任何操作，因为单次模式下转换完后自动停止
        若芯片处于连续转换模式，则向芯片发出掉电命令
    */
    ADS1220_Error_t (*Stop)(ADS1220_t* dev);
    
    /*
        重置芯片
    */
    ADS1220_Error_t (*Reset)(ADS1220_t* dev);
   
    /*
    尝试读取一次ADC的转换数据直至超时，调用该函数前必须保证ADC已处于正常转换状态
    读取的ADC转换后的原始数据，此时data指针指向 int32_t 类型,高8位无效；
    */
    ADS1220_Error_t (*GetAdcData)(ADS1220_t* dev, int32_t* data, const uint32_t max_wait_ms);
    
    /*
    将ADC转换后的24bit数据根据给定配置转为模拟电压值
    data_in的高8位无效；
    */
    ADS1220_Error_t (*ConvData)(ADS1220_t* dev, const int32_t* data_in, double* data_out);
    
    /*
    获取模拟电源电压，采集次数由samp_cnt指定，计算其平均值
    测量模拟电源 (MUX[3:0] = 1101) 时，得出的转换结果约为 (AVDD – AVSS) / 4。
    无论是否在配置寄存器中选择基准源 (VREF[1:0])，该器件均使用 2.048V 内部基准电压进行测量。
	
	注意，调用该函数后将使ADC芯片处于掉电状态
    */
    ADS1220_Error_t (*GetPwrVolt)(ADS1220_t* dev, const uint8_t samp_cnt);
     
    /*
    获取外部基准源电源电压，采集次数由samp_cnt指定，计算其平均值
    调用前提是需要设置基准源为外部基准
	
	注意，调用该函数后将使ADC芯片处于掉电状态
    */
    ADS1220_Error_t (*GetRefVolt)(ADS1220_t* dev, const uint8_t samp_cnt);
     
     /*
    获取温度，采集次数由samp_cnt指定，计算其平均值
 
    ADS1220 集成了一个精密温度传感器。通过将配置寄存器的 TS 位置 1 可使能温度传感器模式。
    在温度传感器模式下，配置寄存器 0 的设置不产生任何影响，该器件使用内部基准进行测量，与所选基准电压源无关。
    温度读数过程与模拟输入启动并读取转换结果的过程相同。温度数据以 14 位结果呈现，与 24 位转换结果左对齐。
    数据从最高有效字节 (MSB) 开始输出。当读取这三个数据字节，前 14 位用于指定温度测量结果。
    一个 14 位 LSB 等于0.03125°C。负数以二进制补码形式表示，如芯片手册的表 12 所示。

	注意，调用该函数后将使ADC芯片处于掉电状态

    */
    ADS1220_Error_t (*GetTemp)(ADS1220_t* dev, const uint8_t samp_cnt);
	
	/*
    进行偏移校准，采集次数由samp_cnt指定，计算其平均值。
	
	注意，调用该函数后将使ADC芯片处于掉电状态
	*/

	ADS1220_Error_t (*GetBias)(ADS1220_t* dev, const uint8_t samp_cnt);
};

/*
    (5)给出初始化ADS1220驱动的函数接口
*/

ADS1220_Error_t ADS1220_API_INIT(ADS1220_t* dev);


#ifdef __cplusplus
}
#endif

#endif /* __ADS1220_H__ */


