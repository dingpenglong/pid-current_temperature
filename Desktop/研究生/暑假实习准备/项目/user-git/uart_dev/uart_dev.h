#ifndef __UART_DEV_H__
#define __UART_DEV_H__

/*
@filename	:	uart_dev.h
@brief		:	�ṩ�������ڵĺ����ӿ�, ���ڲ����ʿɵ�, 8λ����λ, 1λֹͣλ, ��У��λ
                �����˹������ʵ����ã��趨Ϊ16��
      
@time		:	2024/12/03
@author		: 	�轡

@version    :   2.0  2024/08/31
                ��1��������Ĭ�ϵĴ����жϷ���������ȥ���û��ظ���д�����жϷ������Ĺ������û�Ҳ�����Զ����жϷ�����
                
                ��2������Printf�������ṩ������д���ڲ�����
                
                2.1 2024/11/30
                ��1��ɾ��Ĭ�ϴ����жϺ����ж�HAL_UART_IRQHandler�������ظ����ã�

                ��2��������������ring_buffer������m_wait_read��־��Ϊ����֡�������ƣ�����ͬ�����Ĺ���m_wait_read��־����ش���
                
                3.0 2024/12/03
                ��1���ع������������룬����HAL��ʵ�ֶ�����ST MCU�ļ��ݡ����� uart_dev_isr.c �������Զ���HAL��ĸ��ֻص�������
                    ���ڣ������ֶ��޸� stm32fxxx_it.c �еĴ��롣��Ϊ���²�����
                    ��1.1���� uart_dev_isr.c ��������Ӧ���ⲿ������
                    ��1.2���� uart_dev_isr.c �е� HAL_UARTEx_RxEventCallback �ص���������Ӷ�Ӧ���Դ��� RxCallBack ������
                    ��1.3���� uart_dev_isr.c �е� HAL_UART_TxCpltCallback �ص���������Ӷ�Ӧ���Դ��� TxCallBack ������
                    ��1.4���� uart_dev_isr.c �е� HAL_UART_ErrorCallback �ص���������Ӷ�Ӧ���Դ��� ErrorCallBack ������
                
                ��2���޸���2.1�����°汾�������ͺ��жϽ��յ�ͨ�ŷ�ʽ����Ϊ�ж��շ�����DMA�շ���ע�⣬�շ���ʽ������ͬ����ͬʱʹ��DMA�շ����ݻ�ͬʱʹ���ж��շ����ݡ�
                    �������ṩ�ĺ����ӿڶ��޷��Խ��ջ�������д���û��κ�ʱ�򶼲�Ӧ�ó��Զ�д���ջ����������򽫿��ܶ�д�����쳣��
                    ���ڷ��ͻ������ṩ�˶�д�����ơ�ע�⣬���ջ�������ר���ڽ��մ��ڵĲ��������ݡ�
                    ���ڷ��ͻ������Ķ�д�����ƣ�������Ǳ�Ҫ����±���ʹ�÷��ͻ���������Ϊ�������������Ľӿڵ��βΡ�
                
                ��3������˶����ķ��ͻ������ͽ��ջ��������շ�������������HAL��ĺ�����ϣ��û�����Read����ʱʵ��ֻ�ӻ��ν��ջ������ж�ȡ���ݡ�
                
                ��4��������̬���Ĳ����ʵĹ��ܣ��û�����ʵʱ�޸Ĵ��ڲ����ʡ�����˵�ͨ��˫�����õĲ����ʲ�һ��ʱ����ͨ�Ŵ�����봮�ڴ����жϻص���������ѭ�������⡣
                    ���ڣ�����֮ǰͨ�Ų������Ƿ�һ�£���ͨ��˫��������һ��ʱ�Ϳ�������ͨ�š�
                
                ��5��֧�ֵ���Ĵ���ͨ�ţ���ֻ���ͻ�ֻ���գ������ܽ����ڵĶ�д���ܶ��رա�
                
                ��6��
                
                
                
                

*/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>	
#include "main.h" 
#include "ring_buffer.h"


//�������ݻ���ṹ�������շ�����
typedef struct _Uart_dev_buf_t
{
    uint8_t *data;//������ָ�� 
    size_t size;  //��������С����λΪ�ֽ�
    
    union _ctrl
    {
        size_t lock;    //��������
        size_t dma_size;//DMA�����жϵ����һ���յ����ֽ�
    }ctrl;
    
}Uart_dev_buf_t;

typedef struct _Uart_dev_t 	Uart_dev_t;

struct _Uart_dev_t
{ 
	UART_HandleTypeDef*	uart_handle; //����1���  
    uint8_t use_dma;       //DMAʹ�ñ�־λ�����λ��Ч�����λΪ1�����ʹ��DMA���շ�����
	RingBuffer  rb_rxbuf;  //���ν��ջ�����
	
    Uart_dev_buf_t txbuf;   //���ͻ�����
    Uart_dev_buf_t rxbuf;   //���ջ�����
    
	//�������˼���װ���ڲ���	
	/*
	��ʼ������
		uart_handle : ����������
        use_dma     : DMAʹ�ñ�־λ�����λ��Ч�����λΪ1�����ʹ��DMA���շ�����
        txbuf_size  : ���ڷ��ͻ�����������ֽ���,Ϊ0�����������ڷ��͹���
		rxbuf_size 	: ���ڽ��ջ�����������ֽ���, Ϊ0�����������ڽ��չ��� 
    ע�⣬�����ڲ��߱����͹��ܣ�����ͨ��������������д������صĺ��������������Printf������Write������ʱ��ֱ�Ӵ���Ӳ�����жϣ������ڲ��߱����չ���ʱ���ơ�
    ע�⣬����ͬʱ�رմ��ڵ��շ����ܣ����շ�����������ͬʱΪ0��
	*/
	void (*Init) (Uart_dev_t *dev, UART_HandleTypeDef* uart_handle, const uint8_t use_dma, const size_t txbuf_size, const size_t rxbuf_size);  
	
	//����ʼ������
	void (*DeInit)(Uart_dev_t *dev);
	
	/*
	д���ڣ����ڷ�����Ϣ��
		data : ��д�봮�ڵ������׵�ַ���������ǽ��ջ�����
		byte : д��������, ���ֽ�Ϊ��λ
	*/
	void (*Write) (Uart_dev_t *dev, void* data, const size_t byte);
	
	/*
	�����ڣ����ڽ�����Ϣ��, ����ʵ�ʶ������ֽ���
		data : ��������ڻ��������ݵ��׵�ַ���������ǽ��ջ�����
		byte : ������������, ���ֽ�Ϊ��λ
	*/
	size_t (*Read) (Uart_dev_t *dev, void* data, const size_t byte);
    
    /*
    �Ը�ʽ���ַ���д���ڲ���������ʵ��д����ֽ�����
    һ����������ַ������ó������ͻ������Ĵ�С�����򴮿ڷ��͵����ݽ����ضϡ�
    ע�⣬��ӡ���β��б��ð������ͻ������������ͻ������������ǣ�ʵ�ʷ������ݿ��ܲ�����Ԥ�ڡ�
    */
    int  (*Printf)(Uart_dev_t *dev, const char* format, ...);
    
    /*
     ���ڽ�����ɻص�����������HAL_UARTEx_RxEventCallback�ص������е��ô˺���
    */
    void (*RxCallBack)(Uart_dev_t *dev, UART_HandleTypeDef *huart, uint16_t Size);
    
    /*
    ���ڷ�����ɻص����������� HAL_UART_TxCpltCallback�ص������е��ô˺���
    */
    void (*TxCallBack)(Uart_dev_t *dev, UART_HandleTypeDef *huart);
    
    /*
    ���ڴ����жϻص����������� HAL_UART_ErrorCallback �ص������е��ô˺���
    */
    
    void (*ErrorCallBack)(Uart_dev_t *dev, UART_HandleTypeDef *huart);
    
    
    /*
        ���Ĵ��ڲ�����
    */
    void (*SetBaudRate)(Uart_dev_t *dev, const uint32_t baudrate);
};
	
//�������û���ʼ�����ں����ӿ�
void UART_DEV_API_INIT(Uart_dev_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */



