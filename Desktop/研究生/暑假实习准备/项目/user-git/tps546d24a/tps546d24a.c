#include "tps546d24a.h"
#include "stdio.h"
#include "string.h"
#include "tps546d24a_conf.h"
#include "math.h"
#include "stdlib.h"
#include "app_X-CUBE-SMBUS.h"

extern SMBUS_StackHandleTypeDef context;
extern SMBUS_StackHandleTypeDef *pcontext;

struct Smbus_Command_t
{
	st_command_t on_off_config;
	st_command_t iout_cal_gain;
	st_command_t iout_cal_offset;
	st_command_t phase;
	st_command_t pin_detect_override;
	st_command_t mfr_specific_29;
	st_command_t vout_scaleloop;
	st_command_t vout_max;
	st_command_t vout_min;
	st_command_t frequency_switch;
	st_command_t operation;
	st_command_t vout_command;
	st_command_t read_vout;
	st_command_t read_iout;
	st_command_t read_temperature;
	st_command_t status_word;
    st_command_t status_vout;
//    st_command_t  iout_oc_fault_response;
//    st_command_t  iout_oc_warn_limit;
};

const st_command_t COMMANDS_TAB[]={
	{0X02, READ_OR_WRITE,  2, 1},//on_off_config
	{0X04, READ,  2, 1},         //Phase
	{0X38, READ_OR_WRITE,  3, 2}, //Iout_Cal_Gain
	{0X39, READ_OR_WRITE,  3, 2},//Iout_Cal_Offset
	{0XEE, READ_OR_WRITE,  3, 2},// Pin_Detect_Override
	{0XED, READ_OR_WRITE,  3, 2},//MFR_SPECIFIC_29
	{0x29, READ_OR_WRITE, 3, 2 },//Vout_ScaleLoop
	{0X24,READ_OR_WRITE, 3, 2 }, //Vout_Max
	{0X2B, READ_OR_WRITE,  3, 2}, //VOUT_Min
	{0X33, READ_OR_WRITE,  3, 2}, //FREQUENCY_SWITCH
	{0X21, READ_OR_WRITE,  3, 2},//VOUT_COMMAND
    {0X8b, READ,  1, 2},      //READ_VOUT
	{0X8c, READ,  1, 2},      //READ_IOUT
	{0X01, READ_OR_WRITE,  2, 1},//OPERATION
	{0X8D,READ,1,2},     //READ_Temperature
	{0X79,READ_OR_WRITE,3,2},//Status_Word
    {0X7A,READ_OR_WRITE,3,2}, //Status_Vout
     //    {0X47,READ_OR_WRITE,3,2},//IOUT_OC_FAULT_RESPONSE
//    {0X46, READ_OR_WRITE, 2,1},    //IOUT_OC_FAULT_LIMIT
};

void Linear11_to_data(double *linear_f,LINEAR11 *linear){
	 int  N =0;
	 int  Y =0;

	N= linear->linear11N ;
    Y = linear->linear11Y;
	*linear_f =(double)Y;
	*linear_f = *linear_f * pow(2,N);
}
//打印调试信息
#ifdef TPS546D24A_PRINT_DEBUG_INFO 
#define TPS546D24A_PRINT_DEBUG(fmt,args...) do{printf("file:%s(%d) func %s:\n", __FILE__,__LINE__,  __FUNCTION__);printf(fmt, ##args);}while(0)
#else
#define TPS546D24A_PRINT_DEBUG(fmt,args...) 
#endif

//检查指针非空
#define CHECK_PTR(ptr, param, field) do{\
                                    if(ptr == NULL) \
                                    { \
                                        param.field = 1;\
										TPS546D24A_PRINT_DEBUG("指针 %s 为空指针。\n", #ptr);\
                                        return param;\
                                    } \
                                }while(0)


/*
   (1)实现TPS546D24A底层通信
								*/
static TPS546D24A_Error Tps546d24a_Smbus_Write(TPS546d24A_t* dev, SMBUS_StackHandleTypeDef *pcontext,st_command_t *pCommand, uint8_t* data, size_t byte_num);
static TPS546D24A_Error Tps546d24a_Smbus_Read(TPS546d24A_t* dev, SMBUS_StackHandleTypeDef *pcontext,st_command_t *pCommand, uint8_t* data, size_t byte_num);


								
static TPS546D24A_Error Tps546d24a_Smbus_Write(TPS546d24A_t* dev, SMBUS_StackHandleTypeDef *pcontext,st_command_t *pCommand, uint8_t* data, size_t byte_num)
{
    TPS546D24A_Error error;
    error.data = 0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    

    
    
	/*若pcontext为空，直接返回*/
	CHECK_PTR(dev->pcontext, error, Pcontext);
	
	
	/*判断data非空*/
	if( data ==NULL){
		error.null_data=1;
	}
	
	/*判断data数据大小错误*/
	if(byte_num >STACK_NBYTE_SIZE){
	     error.size_data=1;
	}
	
	/*判断smbus栈状态*/
	while(STACK_SMBUS_IsReady(pcontext) != SMBUS_SMS_READY);

	
	//获取栈状态
	uint8_t *piobuf=STACK_SMBUS_GetBuffer( pcontext );
	/*向缓冲区写入数据*/
	memmove(piobuf, data, byte_num);
    /*判断栈状态*/
    while( ( (pcontext->StateMachine) & SMBUS_SMS_ACTIVE_MASK ) != 0U );
    /*发送数据*/
	STACK_SMBUS_HostCommand((SMBUS_StackHandleTypeDef *)pcontext, (st_command_t *)pCommand, (uint16_t)SALVE_ADDRESS, WRITE); 
//    /*判断smbus栈状态*/
	while(!STACK_SMBUS_IsReady(pcontext)){
        MX_SMBUS_Error_Check(pcontext);
    };

  
    return error;
}

static TPS546D24A_Error Tps546d24a_Smbus_Read(TPS546d24A_t* dev, SMBUS_StackHandleTypeDef *pcontext,st_command_t *pCommand, uint8_t* data, size_t byte_num)
{
    TPS546D24A_Error error;
    error.data = 0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
    
    
	/*若pcontext为空，直接返回*/
	CHECK_PTR(dev->pcontext, error, Pcontext);
	
	
	/*判断data非空*/
	if( data ==NULL){
		error.null_data=1;
	}
	
	/*判断data数据大小错误*/
	if(byte_num >STACK_NBYTE_SIZE){
	     error.size_data=1;
	}
	
	/*判断smbus栈状态*/
	 while( ( (pcontext->StateMachine) & SMBUS_SMS_ACTIVE_MASK ) != 0U );
	
	/*接收数据*/
	HAL_StatusTypeDef state=STACK_SMBUS_HostCommand(pcontext,(st_command_t *)pCommand, (uint16_t)SALVE_ADDRESS,READ );
	while(!STACK_SMBUS_IsReady(pcontext)){
        MX_SMBUS_Error_Check(pcontext);
    };
	if( state==HAL_OK)
	{	
		memmove(data, STACK_SMBUS_GetBuffer( pcontext), pCommand->cmnd_master_Rx_size);
		while(STACK_SMBUS_IsBusy(pcontext) == SMBUS_SMS_BUSY);
	
	}
    return error;
}

/*
   (2)实现提供给用户调用的应用层接口
								*/

// Inint
static TPS546D24A_Error TPS546D24A_Init(TPS546d24A_t* dev,SMBUS_StackHandleTypeDef *pcontext,const float vout_max,
	const  float vout_min,const float vout_default){
		
	TPS546D24A_Error error;
	error.data = 0;
	
	dev->pcontext=pcontext;

	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
    
    
	/*若设备没有绑定pcontext，直接返回*/
	CHECK_PTR(dev->pcontext, error, Pcontext);
	
	//command 动态申请空间
	dev->smbus_command= TPS546D24A_MALLOC(Smbus_Command);
	CHECK_PTR(dev->smbus_command, error, malloc);
	

	/*默认输出电压大于5.5V 且小于0.5V,则报错*/
	if((vout_default>5.5 || vout_default< 0.25))
{
	TPS546D24A_PRINT_DEBUG("The default vout(%lfV) is illegal.", vout_default);
	error.set_vout = 1;
	return error;
}
	if((vout_min>5.5 || vout_min< 0.25))
{
	TPS546D24A_PRINT_DEBUG("The default voutmin(%lfV) is illegal.", vout_min);
	error.vout_min = 1;
	return error;
}
	if((vout_max>5.5 || vout_max< 0.25))
{
	TPS546D24A_PRINT_DEBUG("The default voutnax(%lfV) is illegal.", vout_max);
	error.vout_max = 1;
	return error;
}
	//初始化软起动和软关闭寄存器
	dev->smbus_command->on_off_config = COMMANDS_TAB[0];
	uint8_t on_off_config1[1]={0x1b};
    uint8_t on_off_config2[1]={0x00};
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->on_off_config,on_off_config1,1).data;
    error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->on_off_config,on_off_config2,1).data;
	error.init = 1;
    error.init &= memcmp(on_off_config1,on_off_config2,sizeof(on_off_config1));
    	
 
	
	//初始化scale_loop寄存器
	dev->smbus_command->vout_scaleloop = COMMANDS_TAB[6];
	uint8_t voutscaleloop1[2]={0x01,0xE8};  //0.125
    uint8_t voutscaleloop2[2]={0x00,0x00};
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->vout_scaleloop,voutscaleloop1,2).data;
    error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->vout_scaleloop,voutscaleloop2,2).data;
    
    error.init |= memcmp(voutscaleloop1,voutscaleloop2,sizeof(voutscaleloop1));
	
	//设置输出电压最小值
	dev->smbus_command->vout_min = COMMANDS_TAB[8];
	uint16_t temp=0;
	temp =(uint16_t )round(vout_min/pow(2, -9));
	uint8_t data1[2]={0x00,0x00};
    uint8_t data2[2]={0x00,0x00};
	data1[0]=temp&0xff;
	data1[1]=temp>>8;
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->vout_min,data1,2).data;//设置VOUT_MIN
    error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->vout_min,data2,2).data;
	error.init |= memcmp(data1,data2,sizeof(data1));
    
	//设置输出电压最大值
	dev->smbus_command->vout_max =COMMANDS_TAB[7];
	temp=0;
	temp =(uint16_t )round(vout_max/pow(2, -9));
	data1[0]= 0x00;
	data1[1]=0x00;
	data1[0]=temp&0xff;
	data1[1]=temp>>8;
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->vout_max,data1,2).data;//设置VOUT_MAX
    data2[0]= 0x00;
	data2[1]=0x00;
    error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->vout_max,data2,2).data;
    error.init |= memcmp(data1,data2,sizeof(data1));
	
	//设置默认电平输出
	dev->smbus_command->vout_command = COMMANDS_TAB[10];
	temp=0;
	temp =(uint16_t )round(vout_default/pow(2, -9));
	data1[0]= 0x00;
	data1[1]=0x00;
	data1[0]=temp&0xff;
	data1[1]=temp>>8;
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->vout_command,data1,2).data;//设置VOUT_COMMAND
    data2[0]=0x00;
    data2[1]=0x00;
    error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->vout_command,data2,2).data;//设置VOUT_COMMAND
    error.init |= memcmp(data1,data2,sizeof(data1));
	
   
    //启动电平覆盖引脚寄存器
	dev->smbus_command->pin_detect_override = COMMANDS_TAB[4];
	uint8_t pindetectoverride1[2]={0x2c,0x1F};
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->pin_detect_override,pindetectoverride1,2).data;
    uint8_t pindetectoverride2[2]={};
	error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->pin_detect_override,pindetectoverride2,2).data;
    error.init |= memcmp(pindetectoverride1,pindetectoverride2,sizeof(pindetectoverride1));
    
    
    
	//配置READVOUT、RAEDIOUT、OPERATION寄存器
	dev->smbus_command->iout_cal_gain = COMMANDS_TAB[2];
	dev->smbus_command->iout_cal_offset = COMMANDS_TAB[3];
	dev->smbus_command->frequency_switch = COMMANDS_TAB[9];
	dev->smbus_command->read_vout = COMMANDS_TAB[11];
	dev->smbus_command->read_iout = COMMANDS_TAB[12];
	dev->smbus_command->operation = COMMANDS_TAB[13];
	dev->smbus_command->read_temperature = COMMANDS_TAB[14];
	dev->smbus_command->status_word = COMMANDS_TAB[15];	
    dev->smbus_command->status_vout =COMMANDS_TAB[16];
//    dev->smbus_command->iout_oc_warn_limit =COMMANDS_TAB[17];
//	//调用回调函数，用户可在回调函数中初始化相关硬件接口
//	if(fun_callback != NULL)
//     fun_callback();

	return error;
	}

static TPS546D24A_Error TPS546D24A_DeInit(TPS546d24A_t* dev, void (*fun_callback)(void)){
	TPS546D24A_Error error;
	error.data = 0;
	//若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);

	//释放寄存器空间
	TPS546D24A_FREE(dev->smbus_command);

	return error;
}


//SetVout
static TPS546D24A_Error	TPS546D24A_SetVout(TPS546d24A_t* dev,  double vout_command){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
	/*默认输出电压大于5.5V 且小于0.5V,则报错*/
	if(vout_command>5.5)
    {
        vout_command =5.5;
    }
    else if(vout_command<0.25) 
    {  
        vout_command=0.25;
    }
     
	uint16_t temp=0;
	dev->smbus_command->vout_command = COMMANDS_TAB[10];
	temp =(uint16_t )round(vout_command/pow(2, -9));
	uint8_t data[2]={0x00,0x00};
	data[0]=temp&0xff;
	data[1]=temp>>8;
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->vout_command,data,2).data;//设置VOUT_COMMAND
	return error;
}


//GetVout
static TPS546D24A_Error	TPS546D24A_GetVout(TPS546d24A_t* dev, double* actual_vout){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	uint8_t reci_data[2]={0};
	error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->read_vout,reci_data,2).data;
	uint16_t vout=0;
	vout=reci_data[1]<<8|reci_data[0];
	*actual_vout=(float)vout*pow(2, -9);

	return error;
}

//GetIout
static TPS546D24A_Error	TPS546D24A_GetIout(TPS546d24A_t* dev,double *actual_iout){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	uint8_t reci_data[2]={0};
	error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->read_iout,reci_data,2).data;
	uint16_t iout=0;
	iout=reci_data[1]<<8|reci_data[0];
	double Iout;
	LINEAR11 linear11;
	linear11.value = iout;
	Linear11_to_data(&Iout,&linear11);
	*actual_iout = Iout;

	return error;
}
//GetTemperature
static TPS546D24A_Error TPS546D24A_GetTemperature(TPS546d24A_t* dev, double* temperature){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    uint8_t reci_data[2]={0};
	error.data |=Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->read_temperature,reci_data,2).data;
	uint16_t t=0;
	t=reci_data[1]<<8|reci_data[0];
	double T;
	LINEAR11 linear11;
	linear11.value = t;
	Linear11_to_data(&T,&linear11);
	*temperature = T;
	return error;
}
//SoftOn
static TPS546D24A_Error	TPS546D24A_SoftOn(TPS546d24A_t* dev){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	uint8_t operation[1]={0x84};
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->operation,operation,1).data;
	return error;
}


//SoftOff
static TPS546D24A_Error	TPS546D24A_SoftOff(TPS546d24A_t* dev){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	uint8_t operation[1]={0x04};
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->operation,operation,1).data;
	return error;
}
//SetVoutMax
static TPS546D24A_Error TPS546D24A_SetVoutMax(TPS546d24A_t* dev,const double vout_max){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	
	if((vout_max>5.5 || vout_max< 0.5))
	{
	TPS546D24A_PRINT_DEBUG("The voutmax input(%lfV) is illegal.", vout_max);
	error.vout_max = 1;
	return error;
	}
	uint16_t temp=0;
	temp =(uint16_t )round(vout_max/pow(2, -9));
	uint8_t data[2]= {0x00,0x00};
	data[0]=temp&0xff;
	data[1]=temp>>8;
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->vout_max,data,2).data;
	return error;
}
//SetVoutMin
static TPS546D24A_Error TPS546D24A_SetVoutMin(TPS546d24A_t* dev,const double vout_min){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
		if((vout_min>5.5 || vout_min< 0.5))
	{
	TPS546D24A_PRINT_DEBUG("The voutmin input(%lfV) is illegal.", vout_min);
	error.vout_min = 1;
	return error;
	}
	uint16_t temp=0;
	temp =(uint16_t )round(vout_min/pow(2, -9));
	uint8_t data[2]= {0x00,0x00};
	data[0]=temp&0xff;
	data[1]=temp>>8;
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->vout_min,data,2).data;
	return error;
}

//SetFrequancySwitch
static TPS546D24A_Error TPS546D24A_SetFrequancySwitch(TPS546d24A_t* dev, double frequancyswitch){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	/*开关频率225k-1.5Mhz*/
	if(frequancyswitch>1500){
		frequancyswitch = 1500; 
	}else if(frequancyswitch <225){
		frequancyswitch = 225; 
	}
	uint16_t temp=0;
	temp =(uint16_t )(frequancyswitch);
	uint8_t data[2]= {0x00,0x00};
	data[0]=temp&0xff;
	data[1]=temp>>8;
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->frequency_switch,data,2).data;
	data[0]=0x00;
	data[1]=0x00;
	Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->frequency_switch,data,2);
	return error;
}

//SetIoutGain
static TPS546D24A_Error TPS546D24A_SetIoutGain(TPS546d24A_t* dev, double ioutgain){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	if(ioutgain >1.984){
		ioutgain =1.984;
	}else if(ioutgain <0){
		ioutgain =1;
	}
	uint16_t temp=0;
	temp =(uint16_t )round(ioutgain/pow(2, -6));
	uint8_t data[2]= {0x00,0xd0};
	data[0]=temp&0xff;
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->iout_cal_gain,data,2).data;
	
	return error;
}
//SetIoutOffset
static TPS546D24A_Error TPS546D24A_SetIoutOffset(TPS546d24A_t* dev, double ioutoffset){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	if(ioutoffset >127){
		ioutoffset =127;
	}
	uint16_t temp=0;
	temp =(uint16_t )round(ioutoffset/pow(2, -4));
	uint8_t data[2]= {0x00,0xe0};
	data[0]=temp&0xff;
	data[1]=data[1]|(temp>>8);
	error.data |=Tps546d24a_Smbus_Write(dev,dev->pcontext,&dev->smbus_command->iout_cal_offset,data,2).data;
	
	return error;
}

//QueryDevserror
static TPS546D24A_Error  Tps546d24a_QueryDevserror(TPS546d24A_t* dev){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	
	STATUS_Error  status_error ={0} ;
	uint8_t data[2]= {0x00,0x00};
	Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->status_word,data,2);
	uint16_t status=(uint16_t)(data[1]<<8)|(data[0]);
	status_error.data=status;
    
// 检查每个故障位
    if (status_error.none_of_the_above) {
        printf("Other above fault: \n");
    }
    if (status_error.cml) {
        printf("Communication, memory, logic faults\n");
    }
    if (status_error.temp) {
        printf("Temperature failure/warning\n");
    }
    if (status_error.vin_uv) {
        printf("Input undervoltage fault\n");
    }
    if (status_error.iout_oc) {
        printf("Output overcurrent fault\n");
    }
    if (status_error.vout_ov) {
        printf("Input vout overvoltage fault\n");
    }
    if (status_error.off) {
        printf("Convert fault\n");
    }
    if (status_error.busy) {
        printf("Busy and non respone\n");
    }
    if (status_error.other) {
        printf("Other fault\n");
    }
    if (status_error.manufacturer) {
        printf("Manufacturer defined faults\n");
    }
    if (status_error.input) {
        printf("Input faults\n");
    }
    if (status_error.iout) {
        printf("Out current fault\n");
    }
    if (status_error.vout) {
        printf("Out voltage fault\n");
    }
    else if(status_error.data ==0){
		printf("No fault\n");
	}
	return error;
    }
static TPS546D24A_Error  Tps546d24a_QueryVoutError(TPS546d24A_t* dev){
	TPS546D24A_Error error;
	error.data = 0;
	 //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
	
	STATUSVOUT_Error  statusvout_error ={0} ;
	uint8_t data[1]= {0x11};
	Tps546d24a_Smbus_Read(dev,dev->pcontext,&dev->smbus_command->status_vout,data,2);
	statusvout_error.data=data[0];

// 检查每个故障位
     // 检查每个故障位
    if (statusvout_error.bits.VOUT_OVF) {
        printf("Output overvoltage fault\n");
    }
    if (statusvout_error.bits.VOUT_OVW) {
        printf("Output overvoltage warning\n");
    }
    if (statusvout_error.bits.VOUT_UVW) {
        printf("Output undervoltage warning\n");
    }
    if (statusvout_error.bits.VOUT_UVF) {
        printf("Output undervoltage fault\n");
    }
    if (statusvout_error.bits.VOUT_MIN_MAX) {
        printf("Output voltage minimum/maximum fault\n");
    }
    if (statusvout_error.bits.TON_MAX) {
        printf("Maximum on-time fault\n");
    }
    if (statusvout_error.data == 0) {
        printf("No fault\n");
    }
	return error;
    }

//QuerySettingserror

static void TPS546D24A_QuerySettingserror(TPS546D24A_Error  error){
	if (error.dev) {
        printf("Device error\n");
    }
    if (error.malloc) {
        printf("Memory allocation error\n");
    }
    if (error.Pcontext) {
        printf("Pcontext error\n");
    }
    if (error.null_data) {
        printf("Null data error\n");
    }
    if (error.size_data) {
        printf("Size data error\n");
    }
    if (error.set_vout) {
        printf("Set Vout error\n");
    }
    if (error.get_vout) {
        printf("Get Vout error\n");
    }
    if (error.set_iout) {
        printf("Set Iout error\n");
    }
    if (error.get_iout) {
        printf("Get Iout error\n");
    }
    if (error.vout_max) {
        printf("Vout max error\n");
    }
    if (error.vout_min) {
        printf("Vout min error\n");
    } 
    if (error.init) {
        printf("Init error\n");
    }
	else {
		printf("No error\n");
	}
}


/*
	(4) 给初始化TPS546D24A函数接口
*/
TPS546D24A_Error TPS546D24A_API_INIT(TPS546d24A_t* dev)
{
    TPS546D24A_Error error;
    error.data = 0;
    
    //若设备不存在，直接返回
    CHECK_PTR(dev, error, dev);
    
	
    //绑定函数接口
	dev->Init           =  TPS546D24A_Init;
    dev->DeInit         =  TPS546D24A_DeInit;
	dev->SetVout        =  TPS546D24A_SetVout;
	dev->GetVout        =  TPS546D24A_GetVout;
	dev->GetIout        =  TPS546D24A_GetIout;
	dev->GetTemperature =  TPS546D24A_GetTemperature;
	dev->SoftOn         =  TPS546D24A_SoftOn;
	dev->SoftOff        =  TPS546D24A_SoftOff;
	dev->SetVoutMax     =  TPS546D24A_SetVoutMax;
	dev->SetVoutMin     =  TPS546D24A_SetVoutMin;
	dev->SetFrequancySwitch=TPS546D24A_SetFrequancySwitch;
//    dev->SetIoutLimit      =TPS546D24A_Set_IoutLimit;
	dev->QuerySettingserror=TPS546D24A_QuerySettingserror;
	dev->SetIoutGain    =  TPS546D24A_SetIoutGain;
	dev->SetIoutOffset  =  TPS546D24A_SetIoutOffset;
	dev->QueryDevserror =  Tps546d24a_QueryDevserror;
    dev->QueryVoutError = Tps546d24a_QueryVoutError;
    return error;
}	
