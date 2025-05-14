/*
@filename	:	uart_dev.c
@brief		:	�����ض���HAL���й����첽���ڵ��жϷ��������� uart_dev_isr.c �����ʹ��
      
@time		:	2024/12/03
@author		: 	�轡

@version    :   3.0  2024/12/03
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "uart_dev.h"	
#include "uart_dev_conf.h"	

//���º������û����ɼ�, ��Uart_dev���͵Ķ���ȥ����

/*
	д���ڣ����ڷ�����Ϣ��
    data : ��д�봮�ڵ������׵�ַ
	byte : д��������, ���ֽ�Ϊ��λ
*/
static void Uart_dev_Write(Uart_dev_t *dev, void* data, const size_t byte)
{
	if((dev == NULL) || (data == NULL) || (byte == 0))
		return ;
    
    //��Ϊ���ջ���������ֱ���˳�
    if((uint8_t*)data == dev->rxbuf.data)
       return ;
   
    //��Ϊ���ͻ���������ȴ����ͻ���������
    if((uint8_t*)data == dev->txbuf.data)
    {
        while(dev->txbuf.ctrl.lock){};
            
        //����
        dev->txbuf.ctrl.lock = 1;
    }  
    
    //�ȴ�����׼���÷�������
	while(dev->uart_handle->gState != HAL_UART_STATE_READY){};
        
    //��������
    if(dev->use_dma & 0x1)
        HAL_UART_Transmit_DMA(dev->uart_handle, (uint8_t*)data, byte); 
    else
        HAL_UART_Transmit_IT(dev->uart_handle, (uint8_t*)data, byte);    
}
	
/*
�����ڣ����ڽ�����Ϣ��, ����ʵ�ʶ������ֽ���
	data : ��������ڻ��������ݵ��׵�ַ
	byte : ������������, ���ֽ�Ϊ��λ
*/
static size_t Uart_dev_Read(Uart_dev_t *dev, void* data, const size_t byte)
{
	if(dev == NULL)
		return 0;
    
    //����ʱ�����������ݣ�ֱ������wait_read
    if(!dev->rb_rxbuf.m_cur_num)
        dev->rb_rxbuf.m_wait_read = 0;
    
    //������ȡ���ֽ���Ϊ0�������ڴ洢������ָ��Ϊ�գ���ֱ���˳�
    if((data == NULL) || (byte == 0))
        return 0;
    
    //��Ϊ���ջ���������ֱ���˳�
    if((uint8_t*)data == dev->rxbuf.data)
       return 0;
   
    //��Ϊ���ͻ���������ȴ����ͻ���������
    if((uint8_t*)data == dev->txbuf.data)
    {
        while(dev->txbuf.ctrl.lock){};
            
        //����
        dev->txbuf.ctrl.lock = 1;
    }
    
	uint8_t *p = (uint8_t*)data;
	size_t read_byte = 0;
	
	while((read_byte < byte) && (dev->rb_rxbuf.m_cur_num > 0))
	{
		dev->rb_rxbuf.Pop(&dev->rb_rxbuf, p + read_byte);
		read_byte++;
	}
    
    //��Ϊ���ͻ���������������ͻ�����
    if((uint8_t*)data == dev->txbuf.data)
        dev->txbuf.ctrl.lock = 0;
	
	return read_byte;
}

/*
    �Ը�ʽ���ַ���д���ڲ���������д����ֽ���
*/
static int  Uart_dev_Printf(Uart_dev_t *dev, const char* format, ...)
{
    if(dev == NULL)
        return 0;       
    
    //�ȴ����ͻ���������
    while(dev->txbuf.ctrl.lock){};
    
    int ret;
    va_list aptr;

    //ʹ��vsnprintf����ȫ��д��
    va_start(aptr, format);
    ret = vsnprintf((char*)dev->txbuf.data, dev->txbuf.size, format, aptr);
    va_end(aptr);
    
    //���С��0����������ֱ�ӷ���
    if(ret < 0)
        return 0; 
    
    
    //�������ֵ���ڻ���ڻ�������С����ô�ַ������ض�
    if(ret >= (int)dev->txbuf.size)
        dev->Write(dev, dev->txbuf.data, dev->txbuf.size);
    else 
        dev->Write(dev, dev->txbuf.data, ret);
  
    return ret;
}
    
   
/*
��ʼ������
	uart_handle : ����������
       use_dma     : DMAʹ�ñ�־λ�����λ��Ч�����λΪ1�����ʹ��DMA���շ�����
       txbuf_size  : ���ڷ��ͻ�����������ֽ���,Ϊ0�����������ڷ��͹���
	rxbuf_size 	: ���ڽ��ջ�����������ֽ���, Ϊ0�����������ڽ��չ��� 
   ע�⣬�����ڲ��߱����͹��ܣ�����ͨ��������������д������صĺ��������������Printf������Write������ʱ��ֱ�Ӵ���Ӳ�����жϣ������ڲ��߱����չ���ʱ���ơ�
   ע�⣬����ͬʱ�رմ��ڵ��շ����ܣ����շ�����������ͬʱΪ0��
*/
static void Uart_dev_Init(Uart_dev_t *dev, UART_HandleTypeDef* uart_handle, const uint8_t use_dma, const size_t txbuf_size, const size_t rxbuf_size)
{	
    if(dev == NULL)
		return ;
    
    dev->use_dma = use_dma & 0x1;
    
    //����շ�������ȫΪ0�ֽڣ������
    if(!(txbuf_size | rxbuf_size))
    {
        memset(dev, 0, sizeof(Uart_dev_t));
        return ;
    }
    
	//�󶨴���������
    dev->uart_handle = uart_handle;
    
    //������ͻ�����Ϊ0�ֽڣ��򲻿������͹���
    if(!txbuf_size)
    {
        //���ͻ���������
        memset(&dev->txbuf, 0, sizeof(dev->txbuf));
        
        //�����д����������صĺ���ָ��
        dev->Write      = NULL;
        dev->Printf     = NULL;
        dev->TxCallBack = NULL;
    }
    else
    {
        //��ʼ�����ͻ�����
        dev->txbuf.data = UART_DEV_MALLOC(uint8_t, txbuf_size);
        dev->txbuf.size = txbuf_size;
        dev->txbuf.ctrl.lock = 0;
    }
    
    //������ջ�����Ϊ0�ֽڣ��򲻿������չ���
    if(!rxbuf_size)
    {
        //���ջ��λ���������ջ�����ȫ������
        memset(&dev->rb_rxbuf, 0 , sizeof(dev->rb_rxbuf));
        memset(&dev->rxbuf, 0, sizeof(dev->rxbuf));
        
        //����������������صĺ���ָ��
        dev->Read       = NULL;
        dev->RxCallBack = NULL;
    }
    else
    {
        //��ʼ�����ڻ��ν��ջ�����
        RingBuffer_API_INIT(&dev->rb_rxbuf);
        dev->rb_rxbuf.Init(&dev->rb_rxbuf, 1, rxbuf_size);
    
        //��ʼ�����ջ�����
        dev->rxbuf.data = UART_DEV_MALLOC(uint8_t, rxbuf_size);
        dev->rxbuf.size = rxbuf_size;
        dev->rxbuf.ctrl.dma_size = 0;
    
        //����һ�ν���
        if(dev->use_dma & 0x1)
            HAL_UARTEx_ReceiveToIdle_DMA(uart_handle, dev->rxbuf.data, dev->rxbuf.size);
        else
            HAL_UARTEx_ReceiveToIdle_IT(uart_handle, dev->rxbuf.data, dev->rxbuf.size);
    }
}
	
//����ʼ������
static void Uart_dev_DeInit(Uart_dev_t *dev)
{
	if(dev == NULL)
		return ;
    
    dev->use_dma = 0;
	
	//����ʼ������
	HAL_UART_DeInit(dev->uart_handle);
	
	//����ʼ�����ڽ��ջ�����
	dev->rb_rxbuf.DeInit(&dev->rb_rxbuf);
    
    //����ʼ�������շ�������
    UART_DEV_FREE(dev->txbuf.data);
    UART_DEV_FREE(dev->rxbuf.data);
	
	//�������ָ��
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
    Ĭ�ϴ��ڿ����жϻص�����
*/
static void Uart_dev_RxCallBack(Uart_dev_t *dev, UART_HandleTypeDef *huart, uint16_t Size)
{
    if(dev == NULL)
        return ;
    
    if(huart != dev->uart_handle)
        return ;
    
    //���ջ��������ȡ�Ŀ�ʼ�±�
    uint16_t rxbuf_pos = 0;
    uint16_t rxbuf_num = Size;

   //���ʹ��DMA,����Ҫ�ȼ����ȡ���ջ�����������������
   if(dev->use_dma & 0x1)
   {
       //���ջ��������ȡ�Ŀ�ʼ�±�
        rxbuf_pos = dev->rxbuf.ctrl.dma_size % dev->rxbuf.size;   
    
        //���ջ��������ȡ��������
        rxbuf_num = (dev->rxbuf.size + Size - dev->rxbuf.ctrl.dma_size)%dev->rxbuf.size;  
    
        //�������һ��DMA�յ���������
        dev->rxbuf.ctrl.dma_size = Size;
    
        //�������������뻺����
        for(uint16_t i=0; i<rxbuf_num; i++)
        {   
            //��������������ջ�����յ�������
            if(dev->rb_rxbuf.IsFull(&dev->rb_rxbuf).FULL)
                dev->rb_rxbuf.Pop(&dev->rb_rxbuf, NULL);
     
            dev->rb_rxbuf.Push(&dev->rb_rxbuf, &dev->rxbuf.data[rxbuf_pos]);
            rxbuf_pos = (rxbuf_pos + 1)%dev->rxbuf.size;
        }
    
        // ��Ϊ�����жϣ����¿�ʼһ�ν���
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE)
        {
            dev->rb_rxbuf.m_wait_read++;
            HAL_UARTEx_ReceiveToIdle_DMA(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
        }
    }
    
    //��ʹ��DMA����ֱ�Ӷ�ȡ���ջ���������������    
    else
    {
        //�������������뻺����
        for(uint16_t i=0; i<rxbuf_num; i++)
        {   
            //��������������ջ�����յ�������
            if(dev->rb_rxbuf.IsFull(&dev->rb_rxbuf).FULL)
                dev->rb_rxbuf.Pop(&dev->rb_rxbuf, NULL);
     
            dev->rb_rxbuf.Push(&dev->rb_rxbuf, &dev->rxbuf.data[rxbuf_pos+i]);
        }
    
        dev->rb_rxbuf.m_wait_read++;
        //���¿�ʼһ�ν���
        HAL_UARTEx_ReceiveToIdle_IT(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
    }
}

static void Uart_dev_TxCallBack(Uart_dev_t *dev, UART_HandleTypeDef *huart)
{
    if(dev == NULL)
        return ;
    
    if(huart != dev->uart_handle)
        return ;
    
    //�������ͻ�����
    dev->txbuf.ctrl.lock = 0;
}

/*
���ڴ����жϻص����������� HAL_UART_ErrorCallback �ص������е��ô˺���
*/

static void Uart_dev_ErrorCallBack(Uart_dev_t *dev, UART_HandleTypeDef *huart)
{
    if(dev == NULL)
        return ;
    
    if(huart != dev->uart_handle)
        return ;
    
    //������ڲ��߱����չ��ܣ�ֱ���˳�
    if(!dev->rxbuf.data || !dev->rxbuf.size)
        return ;
    
    //���¿�ʼһ�ν���
    if(dev->use_dma & 0x1)
        HAL_UARTEx_ReceiveToIdle_DMA(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
    else
        HAL_UARTEx_ReceiveToIdle_IT(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
}

/*
    ���Ĵ��ڲ�����
*/
static void Uart_dev_SetBaudRate(Uart_dev_t *dev, const uint32_t baudrate)
{
    if(dev == NULL)
        return ;
    
    //�ȴ�����׼���÷�������
	while(dev->uart_handle->gState != HAL_UART_STATE_READY){};
        
    // �رմ���
    __HAL_UART_DISABLE(dev->uart_handle);
    
    // �޸Ĳ�����
    dev->uart_handle->Init.BaudRate = baudrate;
    
    // ���³�ʼ������
    HAL_UART_Init(dev->uart_handle);
    
    // �������ô���
    __HAL_UART_ENABLE(dev->uart_handle);
    
    //���¿�ʼһ�ν���
    if(dev->use_dma & 0x1)
        HAL_UARTEx_ReceiveToIdle_DMA(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
    else
        HAL_UARTEx_ReceiveToIdle_IT(dev->uart_handle, dev->rxbuf.data, dev->rxbuf.size);
}

//�������û���ʼ�����ں����ӿ�
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
  



