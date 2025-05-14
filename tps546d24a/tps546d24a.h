#ifndef __TPS546D24A_H__
#define __TPS546D24A_H__
/*
@filename   tps546d24a.h

@brief		基于PMBUS控制的恒流源

@time		2024/10/10

@author		丁鹏龙

@version    1.1

--------------------------------------------------------

@attention   
			一定要对芯片先初始化，设置好VOUT之后，才进行软开启
            初始化最大电压设置为5.5V,最小电压设置为0.5V，默认电压要在设置的电压范围内
            写入顺序是低字节在前，高字节在后
*/
#include "stm32g4xx_hal.h"
#include "stm32_PMBUS_stack.h"
//#include "app_x-cube-smbus.h"
#ifdef __cplusplus
extern "C" {
#endif

//Linear11数据格式------PMBUS内部的数据格式转换
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
#define SALVE_ADDRESS 0x48  //从机地址
typedef union
{
    struct
    {
        uint16_t dev     : 1; //设备不存在
        uint16_t malloc  : 1; //申请动态空间失败
		uint16_t Pcontext     : 1; //pcontext无效
		uint16_t null_data  : 1; //数据为空或者大小错误
		uint16_t size_data  : 1; //数据大小错误
        uint16_t set_vout    : 1; //vout设置无效
		uint16_t get_vout    : 1; //vout获取无效
		uint16_t set_iout    : 1; //iout设置无效
		uint16_t get_iout    : 1; //iout获取无效
        uint16_t vout_max    : 1; //最大输出电压设置无效
        uint16_t vout_min     : 1; //最小输出电压设置无效    
        uint16_t init          : 1; //初始化错误    
    };
    uint16_t data;
}TPS546D24A_Error;  

typedef union {
    struct {
        uint16_t none_of_the_above : 1;     // 下列故障以外的故障
        uint16_t cml : 1;                   // 通信、存储器、逻辑故障
        uint16_t temp : 1;                  // 温度故障/警告
        uint16_t vin_uv : 1;                // 输入欠压故障
        uint16_t iout_oc : 1;               // 输出过流故障
        uint16_t vout_ov : 1;               // 输出过压故障
        uint16_t off : 1;                   // 转换电源关闭
        uint16_t busy : 1;                  // 器件繁忙且无法响应
        uint16_t other : 1;                 // 其他故障
        uint16_t manufacturer : 1;          // 制造商定义的故障
        uint16_t input : 1;                 // 输入故障
        uint16_t iout : 1;                  // 输出电流故障
        uint16_t vout : 1;                  // 输出电压故障
        uint16_t : 1;                      // 占位
    };
    uint16_t data; 
} STATUS_Error;

typedef union {
    struct {
        uint8_t VOUT_OVF : 1;      // 输出过压故障标志
        uint8_t VOUT_OVW : 1;      // 输出过压警告标志
        uint8_t VOUT_UVW : 1;      // 输出欠压警告标志
        uint8_t VOUT_UVF : 1;      // 输出欠压故障标志
        uint8_t VOUT_MIN_MAX : 1;  // 输出电压最小/最大故障标志
        uint8_t TON_MAX : 1;       // 最大导通时间故障标志
        uint8_t unsupported : 1;   // 不支持的位，始终为0
    } bits;
    uint8_t data;                  // 联合体的整数表示，方便整体读写
}STATUSVOUT_Error;


typedef struct Smbus_Command_t Smbus_Command;


typedef struct _TPS546d24A_t TPS546d24A_t;


struct _TPS546d24A_t
{	/*command 寄存器*/
    Smbus_Command* smbus_command;
	
	/*
		SMBUS栈指针接口
	*/

	SMBUS_StackHandleTypeDef *pcontext;
	
    
    //操作接口
    
  /*初始化TPS54624A*/
    TPS546D24A_Error (* Init)(TPS546d24A_t* dev,SMBUS_StackHandleTypeDef *pcontext,const float vout_max, const float  vout_min,  const float vout_default);
     /*
        反初始化TPS546D24A
    */
    TPS546D24A_Error (* DeInit)(TPS546d24A_t* dev, void (*fun_callback)(void));

    /*
        设置输出电压
    */
    TPS546D24A_Error(*SetVout)(TPS546d24A_t* dev,  double voutcommand);
	
    /*
		获取输出电压
    */
	TPS546D24A_Error(*GetVout)(TPS546d24A_t* dev, double* actual_vout);
	
	 /*
		获取输出电流
    */
	TPS546D24A_Error(*GetIout)(TPS546d24A_t* dev, double* actual_iout);
	
	 /*
		获取芯片温度
    */
	TPS546D24A_Error(*GetTemperature)(TPS546d24A_t* dev, double* temperature);

    /*
		软启动
    */
    TPS546D24A_Error (* SoftOn)(TPS546d24A_t* dev);
    
	 /*
		软关闭
    */
    TPS546D24A_Error (* SoftOff)(TPS546d24A_t* dev);
	
	/*设置VOUTMAX
	*/
    TPS546D24A_Error (* SetVoutMax)(TPS546d24A_t* dev,const double vout_max);
		
	/*设置VOUTMIM
	*/
    TPS546D24A_Error (* SetVoutMin)(TPS546d24A_t* dev,const double vout_min);
	
	/*设置开关频率*/
	TPS546D24A_Error (* SetFrequancySwitch)(TPS546d24A_t* dev,  double frequancyswitch);
    
//    /*设置电流阈值*/
//    TPS546D24A_Error  (* SetIoutLimit)(TPS546d24A_t* dev,double iout_limit);
	
	/*设置电流修正增益*/
	TPS546D24A_Error (* SetIoutGain)(TPS546d24A_t* dev,  double ioutgain);
	
	/*设置电流修正偏移*/
	TPS546D24A_Error (* SetIoutOffset)(TPS546d24A_t* dev,  double ioutoffset);
	
	/*查询设备状态错误*/
	TPS546D24A_Error(* QueryDevserror)(TPS546d24A_t* dev);
    /*查询Vout错误*/
	TPS546D24A_Error(*QueryVoutError)(TPS546d24A_t* dev);
	/*查询设置错误*/
	void (* QuerySettingserror)(TPS546D24A_Error  error); 
	

	
	
};

/*
    (4)给出初始化TPS546D24A驱动的函数接口
*/

TPS546D24A_Error TPS546D24A_API_INIT(TPS546d24A_t* dev);


#ifdef __cplusplus
}
#endif



#endif   /* __TPS546D24A_H__*/


