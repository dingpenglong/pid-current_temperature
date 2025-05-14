/*
@filename	:	uart_dev.c
@brief		:	用于重定义HAL库中关于异步串口的中断服务函数，与 uart_dev_isr.c 相配合使用
      
@time		:	2024/12/03
@author		: 	黎健

@version    :   3.0  2024/12/03
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "uart_dev.h"	
#include "uart_dev_conf.h"	

//以下函数对用户不可见, 由Uart_dev类型的对象去调用

/*
	写串口（串口发送消息）
    data : 需写入串口的数据首地址
	byte : 写入数据量, 以字节为单位
*/
static void Uart_dev_Write(Uart_dev_t *dev, void* data, const size_t byte)
{
	if((dev == NULL) || (data == NULL) || (byte == 0))
		return ;
    
    //若为接收缓冲区，则直接退出
    if((uint8_t*)data == dev->rxbuf.data)
       return ;
   
    //若为发送缓冲区，则等待发送缓冲区解锁
    if((uint8_t*)data == dev->txbuf.data)
    {
        while(dev->txbuf.ctrl.lock){};
            
        //上锁
        dev->txbuf.ctrl.lock = 1;
    }  
    
    //等待串口准备好发送数据
	while(dev->uart_handle->gState != HAL_UART_STATE_READY){};
        
    //发送数据
    if(dev->use_dma & 0x1)
        HAL_UART_Transmit_DMA(dev->uart_handle, (uint8_t*)data, byte); 
    else
        HAL_UART_Transmit_IT(dev->uart_handle, (uint8_t*)data, byte);    
}
	
/*
读串口（串口接收消息）, 返回实际读出的字节数
	data : 需读出串口缓冲区数据的首地址
	byte : 最大读出数据量, 以字节为单位
*/
static size_t Uart_dev_Read(Uart_dev_t *dev, void* data, const size_t byte)
{
	if(dev == NULL)
		return 0;
    
    //若此时缓冲区无数据，直接清零wait_read
    if(!dev->rb_rxbuf.m_cur_num)
        dev->rb_rxbuf.m_wait_read = 0;
    
    //如果需读取的字节数为0或者用于存储的数组指针为空，则直接退出
    if((data == NULL) || (byte == 0))
        return 0;
    
    //若为接收缓冲区，则直接退出
    if((uint8_t*)data == dev->rxbuf.data)
       return 0;
   
    //若为发送缓冲区，则等待发送缓冲区解锁
    if((uint8_t*)data == dev->txbuf.data)
    {
        while(dev->txbuf.ctrl.lock){};
            
        //上锁
        dev->txbuf.ctrl.lock = 1;
    }
    
	uint8_t *p = (uint8_t*)data;
	size_t read_byte = 0;
	
	while((read_byte < byte) && (dev->rb_rxbuf.m_cur_num > 0))
	{
		dev->rb_rxbuf.Pop(&dev->rb_rxbuf, p + read_byte);
		read_byte++;
	}
    
    //若为发送缓冲区，则解锁发送缓冲区
    if((uint8_t*)data == dev->txbuf.data)
        dev->txbuf.ctrl.lock = 0;
	
	return read_byte;
}

/*
    以格式化字符串写串口操作，返回写入的字节数
*/
static int  Uart_dev_Printf(Uart_dev_t *dev, const char* format, ...)
{
    if(dev == NULL)
        return 0;       
    
    //等待发送缓冲区解锁
    while(dev->txbuf.ctrl.lock){};
    
    int ret;
    va_list aptr;

    //使用vsnprintf来安全的写入
    va_start(aptr, format);
    ret = vsnprintf((char*)dev->txbuf.data, dev->txbuf.size, format, aptr);
    va_end(aptr);
    
    //如果小于0，则发生错误，直接返回
    if(ret < 0)
        return 0; 
    
    
    //如果返回值大于或等于缓冲区大小，那么字符串被截断
    if(ret >= (int)dev->txbuf.size)
        dev->Write(dev, dev->txbuf.data, dev->txbuf.size);
    else 
        dev->Write(dev, dev->txbuf.data, ret);
  
    return ret;
}
    
   
/*
初始化串口
	uart_handle : 串口描述符
       use_dma     : DMA使用标志位，最低位有效。最低位为1则代表使用DMA来收发数据
       txbuf_size  : 串口发送缓冲区的最大字节数,为0代表不开启串口发送功能
	rxbuf_size 	: 串口接收缓冲区的最大字节数, 为0代表不开启串口接收功能 
   注意，若串口不具备发送功能，尝试通过本驱动调用与写串口相关的函数或变量（例如Printf函数或Write函数）时将直接触发硬错误中断，若串口不具备接收功能时类似。
   注意，不能同时关闭串口的收发功能，即收发缓冲区不能同时为0。
*/
static void Uart_dev_Init(Uart_dev_t *dev, UART_HandleTypeDef* uart_handle, const uint8_t use_dma, const size_t txbuf_size, const size_t rxbuf_size)
{	
    if(dev == NULL)
		return ;
    
    dev->use_dma = use_dma & 0x1;
    
    //如果收发缓存区全为0字节，则出错
    if(!(txbuf_size | rxbuf_size))
    {
        memset(dev, 0, sizeof(Uart_dev_t));
        return ;
    }
    
	//绑定串口描述符
    dev->uart_handle = uart_handle;
    
    //如果发送缓冲区为0字节，则不开启发送功能
    if(!txbuf_size)
    {
        //发送缓冲区置零
        memset(&dev->txbuf, 0, sizeof(dev->txbuf));
        
        //解绑与写串口数据相关的函数指针
        dev->Write      = NULL;
        dev->Printf     = NULL;
        dev->TxCallBack = NULL;
    }
    else
    {
        //初始化发送缓冲区
        dev->txbuf.data = UART_DEV_MALLOC(uint8_t, txbuf_size);
        dev->txbuf.size = txbuf_size;
        dev->txbuf.ctrl.lock = 0;
    }
    
    //如果接收缓冲区为0字节，则不开启接收功能
    if(!rxbuf_size)
    {
        //接收环形缓冲区与接收缓冲区全部置零
        memset(&dev->rb_rxbuf, 0 , sizeof(dev->rb_rxbuf));
        memset(&dev->rxbuf, 0, sizeof(dev->rxbuf));
        
        //解绑与读串口数据相关的函数指针
        dev->Read       = NULL;
        dev->RxCallBack = NULL;
    }
    else
    {
        //初始化串口环形接收缓冲区
        RingBuffer_API_INIT(&dev->rb_rxbuf);
        dev->rb_rxbuf.Init(&dev->rb_rxbuf, 1, rxbuf_size);
    
        //初始化接收缓冲区
        dev->rxbuf.data = UART_DEV_MALLOC(uint8_t, rxbuf_size);
        dev->rxbuf.size = rxbuf_size;
        dev->rxbuf.ctrl.dma_size = 0;
    
        //开启一次接收
        if(dev->use_dma & 0x1)
            HAL_UARTEx_ReceiveToIdle_DMA(uart_handle, dev->rxbuf.data, dev->rxbuf.size);
        else
            HAL_UARTEx_ReceiveToIdle_IT(uart_handle, dev->rxbuf.data, dev->rxbuf.size);
    }
}
	
//反初始化串口
static void Uart_dev_DeInit(Uart_dev_t *dev)
{
	if(dev == NULL)
		return ;
    
    dev->use_dma = 0;
	
	//反初始化串口
	HAL_UART_DeInit(dev->uart_handle);
	
	//反初始化串口接收缓冲区
	dev->rb_rxbuf.DeInit(&dev->rb_rxbuf);
    
    //反初始化串口收发缓冲区
    UART_DEV_FREE(dev->txbuf.data);
    UART_DEV_FREE(dev->rxbuf.data);
	
	//解除函数指针
	dev->Init	        = NULL;
	dev->DeInit	        = NULL;
	dev->Write	        = NULL;
	dev->Read	        = NULL;
    dev->TxCallBack     = NULL;
    dev->RxCallBack     = NULL;
    dev->ErrorCallBack  = NULL;
    dev->Printf         = NULL;
    dev->SetBaudRate    = NULL;
}

/*
    默认串口空闲中断回调函数
*/
static void Uart_dev_RxCallBack(Uart_dev_t *dev, UART_HandleTypeDef *huart, uint16_t Size)
{
    if(dev == NULL)
        return ;
    
    if(huart != dev->uart_handle)
        return ;
    
    //接收缓冲区需读取的开始下标
    uint16_t rxbuf_pos = 0;
    uint16_t rxbuf_num = Size;

   //如果使用DMA,则需要先计算读取接收缓冲区的起点和数据量
   if(dev->use_dma & 0x1)
   {
       //接收缓冲区需读取的开始下标
        rxbuf_pos = dev->rxbuf.ctrl.dma_size % dev->rxbuf.size;   
    
        //接收缓冲区需读取的数据量
        rxbuf_num = (dev->rxbuf.size + Size - dev->rxbuf.ctrl.dma_size)%dev->rxbuf.size;  
    
        //更新最后一次DMA收到的数据量
        dev->rxbuf.ctrl.dma_size = Size;
    
        //将数据依次移入缓冲区
        for(uint16_t i=0; i<rxbuf_num; i++)
        {   
            //若缓冲区满，出栈最先收到的数据
            if(dev->rb_rxbuf.IsFull(&dev->rb_rxbuf).FULL)
                dev->rb_rxbuf.Pop(&dev->rb_rxbuf, NULL);
     
            dev->rb_rxbuf.Push(&dev->rb_rxbuf, &dev->rxbuf.data[rxbuf_pos]);
            rxbuf_pos = (rxbuf_pos + 1)%dev->rxbuf.size;
        }
    
        // 若为空闲中断，重新开始一次接收
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE)
        {
            dev->rb_rxbuf.m_wait_read++;
            HAL_UARTEx_ReceiveToIdle_DMA(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
        }
    }
    
    //不使用DMA，则直接读取接收缓冲区的所有数据    
    else
    {
        //将数据依次移入缓冲区
        for(uint16_t i=0; i<rxbuf_num; i++)
        {   
            //若缓冲区满，出栈最先收到的数据
            if(dev->rb_rxbuf.IsFull(&dev->rb_rxbuf).FULL)
                dev->rb_rxbuf.Pop(&dev->rb_rxbuf, NULL);
     
            dev->rb_rxbuf.Push(&dev->rb_rxbuf, &dev->rxbuf.data[rxbuf_pos+i]);
        }
    
        dev->rb_rxbuf.m_wait_read++;
        //重新开始一次接收
        HAL_UARTEx_ReceiveToIdle_IT(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
    }
}

static void Uart_dev_TxCallBack(Uart_dev_t *dev, UART_HandleTypeDef *huart)
{
    if(dev == NULL)
        return ;
    
    if(huart != dev->uart_handle)
        return ;
    
    //解锁发送缓冲区
    dev->txbuf.ctrl.lock = 0;
}

/*
串口错误中断回调函数，请在 HAL_UART_ErrorCallback 回调函数中调用此函数
*/

static void Uart_dev_ErrorCallBack(Uart_dev_t *dev, UART_HandleTypeDef *huart)
{
    if(dev == NULL)
        return ;
    
    if(huart != dev->uart_handle)
        return ;
    
    //如果串口不具备接收功能，直接退出
    if(!dev->rxbuf.data || !dev->rxbuf.size)
        return ;
    
    //重新开始一次接收
    if(dev->use_dma & 0x1)
        HAL_UARTEx_ReceiveToIdle_DMA(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
    else
        HAL_UARTEx_ReceiveToIdle_IT(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
}

/*
    更改串口波特率
*/
static void Uart_dev_SetBaudRate(Uart_dev_t *dev, const uint32_t baudrate)
{
    if(dev == NULL)
        return ;
    
    //等待串口准备好发送数据
	while(dev->uart_handle->gState != HAL_UART_STATE_READY){};
        
    // 关闭串口
    __HAL_UART_DISABLE(dev->uart_handle);
    
    // 修改波特率
    dev->uart_handle->Init.BaudRate = baudrate;
    
    // 重新初始化串口
    HAL_UART_Init(dev->uart_handle);
    
    // 重新启用串口
    __HAL_UART_ENABLE(dev->uart_handle);
    
    //重新开始一次接收
    if(dev->use_dma & 0x1)
        HAL_UARTEx_ReceiveToIdle_DMA(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
    else
        HAL_UARTEx_ReceiveToIdle_IT(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
}

//公开给用户初始化串口函数接口
void UART_DEV_API_INIT(Uart_dev_t *dev)
{
	if(dev == NULL)
		return ;
	
	dev->Init	        = Uart_dev_Init;
	dev->DeInit	        = Uart_dev_DeInit;
	dev->Write	        = Uart_dev_Write;
	dev->Read	        = Uart_dev_Read;
    dev->RxCallBack     = Uart_dev_RxCallBack;
    dev->TxCallBack     = Uart_dev_TxCallBack;
    dev->ErrorCallBack  = Uart_dev_ErrorCallBack;
    dev->Printf         = Uart_dev_Printf;
    dev->SetBaudRate    = Uart_dev_SetBaudRate;
}
  



