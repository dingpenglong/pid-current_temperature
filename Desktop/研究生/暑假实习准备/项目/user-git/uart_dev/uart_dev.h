#ifndef __UART_DEV_H__
#define __UART_DEV_H__

/*
@filename	:	uart_dev.h
@brief		:	提供操作串口的函数接口, 串口波特率可调, 8位数据位, 1位停止位, 无校验位
                增加了过采样率的设置，设定为16倍
      
@time		:	2024/12/03
@author		: 	黎健

@version    :   2.0  2024/08/31
                （1）增加了默认的串口中断服务函数，免去了用户重复编写串口中断服务函数的工作；用户也可以自定义中断服务函数
                
                （2）增加Printf函数以提供更灵活的写串口操作。
                
                2.1 2024/11/30
                （1）删除默认串口中断函数中对HAL_UART_IRQHandler函数的重复调用；

                （2）由于其依赖的ring_buffer驱动中m_wait_read标志改为接收帧计数机制，这里同步更改关于m_wait_read标志的相关代码
                
                3.0 2024/12/03
                （1）重构整体驱动代码，基于HAL库实现对所有ST MCU的兼容。增加 uart_dev_isr.c 来用于自定义HAL库的各种回调函数。
                    现在，无需手动修改 stm32fxxx_it.c 中的代码。改为如下操作：
                    （1.1）在 uart_dev_isr.c 中声明对应的外部变量。
                    （1.2）在 uart_dev_isr.c 中的 HAL_UARTEx_RxEventCallback 回调函数中添加对应的自带的 RxCallBack 函数。
                    （1.3）在 uart_dev_isr.c 中的 HAL_UART_TxCpltCallback 回调函数中添加对应的自带的 TxCallBack 函数。
                    （1.4）在 uart_dev_isr.c 中的 HAL_UART_ErrorCallback 回调函数中添加对应的自带的 ErrorCallBack 函数。
                
                （2）修改了2.1及以下版本阻塞发送和中断接收的通信方式，改为中断收发或者DMA收发。注意，收发方式必须相同，即同时使用DMA收发数据或同时使用中断收发数据。
                    本驱动提供的函数接口都无法对接收缓冲区读写，用户任何时候都不应该尝试读写接收缓冲区，否则将可能读写数据异常！
                    对于发送缓冲区提供了读写锁机制。注意，接收缓冲区被专用于接收串口的不定长数据。
                    由于发送缓冲区的读写锁机制，建议如非必要情况下避免使用发送缓冲区来作为本驱动所公开的接口的形参。
                
                （3）添加了独立的发送缓冲区和接收缓冲区。收发缓冲区用于与HAL库的函数配合，用户调用Read函数时实际只从环形接收缓冲区中读取数据。
                
                （4）新增动态更改波特率的功能，用户可以实时修改串口波特率。解决了当通信双方设置的波特率不一致时由于通信错误进入串口错误中断回调函数后死循环的问题。
                    现在，无论之前通信波特率是否一致，当通信双方波特率一致时就可以正常通信。
                
                （5）支持单向的串口通信，即只发送或只接收，但不能将串口的读写功能都关闭。
                
                （6）
                
                
                
                

*/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>	
#include "main.h" 
#include "ring_buffer.h"


//定义数据缓冲结构体用于收发数据
typedef struct _Uart_dev_buf_t
{
    uint8_t *data;//缓冲区指针 
    size_t size;  //缓冲区大小，单位为字节
    
    union _ctrl
    {
        size_t lock;    //缓冲区锁
        size_t dma_size;//DMA接收中断的最后一次收到的字节
    }ctrl;
    
}Uart_dev_buf_t;

typedef struct _Uart_dev_t 	Uart_dev_t;

struct _Uart_dev_t
{ 
	UART_HandleTypeDef*	uart_handle; //串口1句柄  
    uint8_t use_dma;       //DMA使用标志位，最低位有效。最低位为1则代表使用DMA来收发数据
	RingBuffer  rb_rxbuf;  //环形接收缓冲区
	
    Uart_dev_buf_t txbuf;   //发送缓冲区
    Uart_dev_buf_t rxbuf;   //接收缓冲区
    
	//面向对象思想封装串口操作	
	/*
	初始化串口
		uart_handle : 串口描述符
        use_dma     : DMA使用标志位，最低位有效。最低位为1则代表使用DMA来收发数据
        txbuf_size  : 串口发送缓冲区的最大字节数,为0代表不开启串口发送功能
		rxbuf_size 	: 串口接收缓冲区的最大字节数, 为0代表不开启串口接收功能 
    注意，若串口不具备发送功能，尝试通过本驱动调用与写串口相关的函数或变量（例如Printf函数或Write函数）时将直接触发硬错误中断，若串口不具备接收功能时类似。
    注意，不能同时关闭串口的收发功能，即收发缓冲区不能同时为0。
	*/
	void (*Init) (Uart_dev_t *dev, UART_HandleTypeDef* uart_handle, const uint8_t use_dma, const size_t txbuf_size, const size_t rxbuf_size);  
	
	//反初始化串口
	void (*DeInit)(Uart_dev_t *dev);
	
	/*
	写串口（串口发送消息）
		data : 需写入串口的数据首地址，不可以是接收缓冲区
		byte : 写入数据量, 以字节为单位
	*/
	void (*Write) (Uart_dev_t *dev, void* data, const size_t byte);
	
	/*
	读串口（串口接收消息）, 返回实际读出的字节数
		data : 需读出串口缓冲区数据的首地址，不可以是接收缓冲区
		byte : 最大读出数据量, 以字节为单位
	*/
	size_t (*Read) (Uart_dev_t *dev, void* data, const size_t byte);
    
    /*
    以格式化字符串写串口操作，返回实际写入的字节数。
    一次性输出的字符串不得超过发送缓冲区的大小，否则串口发送的数据将被截断。
    注意，打印的形参列表不得包含发送缓冲区，否则发送缓冲区将被覆盖，实际发送数据可能不符合预期。
    */
    int  (*Printf)(Uart_dev_t *dev, const char* format, ...);
    
    /*
     串口接收完成回调函数，请在HAL_UARTEx_RxEventCallback回调函数中调用此函数
    */
    void (*RxCallBack)(Uart_dev_t *dev, UART_HandleTypeDef *huart, uint16_t Size);
    
    /*
    串口发送完成回调函数，请在 HAL_UART_TxCpltCallback回调函数中调用此函数
    */
    void (*TxCallBack)(Uart_dev_t *dev, UART_HandleTypeDef *huart);
    
    /*
    串口错误中断回调函数，请在 HAL_UART_ErrorCallback 回调函数中调用此函数
    */
    
    void (*ErrorCallBack)(Uart_dev_t *dev, UART_HandleTypeDef *huart);
    
    
    /*
        更改串口波特率
    */
    void (*SetBaudRate)(Uart_dev_t *dev, const uint32_t baudrate);
};
	
//公开给用户初始化串口函数接口
void UART_DEV_API_INIT(Uart_dev_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */



