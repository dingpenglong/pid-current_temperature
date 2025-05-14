#include "time_period_fun.h"
#include "tim.h"
#define ADS1220_MAX_ADC_DATA  0X800000 //2^23 - 1


/*恒温定时器周期调用函数*/
void  Pump_Driver(TPS546d24A_t* tps546d24a,ADS1220_t* ads1220,DRV8412_t* drv8412,Pid_System_t* pid_system1,Pid_System_t* pid_system2,ntc_t* ntc){

// //恒流
    volatile double Vout1=0,Iout1=0;
    int32_t adc_data1=0;

//    //将adc读取换为通道1
    ads1220->regs.reg0.mux = AIN1_AVSS;
    ads1220->WriteReg(ads1220, REG0);
    ads1220->Start(ads1220);
    ads1220->GetAdcData(ads1220,&adc_data1,100);
    Iout1 = (adc_data1 * ads1220->monitor.vref[0]) / ADS1220_MAX_ADC_DATA;
    Iout1 *=4; //2.5m欧的采样电阻

    int temp1 = round(Iout1*1000);         // 上位机需要整数参数，转换一下  
    set_computer_value(&huart4,SEND_FACT_CMD, CURVES_CH1, &temp1, 1);
//    
    pid_system1->pid->actual_val=Iout1;
    double val1=pid_system1->Incremental_Realize(pid_system1,pid_system1->pid->actual_val);
    tps546d24a->SetVout(tps546d24a,val1);	

    
 //恒温
//    volatile double Vout2=0;
//    int32_t adc_data2=0;
//    //将adc读取换为通道0
//    ads1220->regs.reg0.mux = AIN0_AVSS;
//    ads1220->WriteReg(ads1220, REG0);
//    ads1220->Start(ads1220);
//    ads1220->GetAdcData(ads1220,&adc_data2,100);
//    Vout2 = (adc_data2 * ads1220->monitor.vref[0]) / ADS1220_MAX_ADC_DATA;
//    double NTCR =(Vout2*10)/(ads1220->monitor.vref[0]-Vout2);
//    double Temp =ntc->ConvToTemp(ntc,NTCR);
//    
//   int  temp2 = round(Temp*1000);            
//    // 上位机需要整数参数，转换一下  
//    set_computer_value(&huart5,SEND_FACT_CMD, CURVES_CH1, &temp2, 1);
//    
//    pid_system2->pid->actual_val = Temp;
//    volatile double val2=pid_system2->Incremental_Realize(pid_system2,pid_system2->pid->actual_val);
//    // 根据PID输出调整温度大小
//    drv8412->Set_Voltage(drv8412,val2);

    
}