/*
@filename	:	uart_dev_isr.c
@brief		:	用于重定义HAL库中关于异步串口的中断服务函数，与usart_dev函数库相配合使用
      
@time		:	2024/12/03
@author		: 	黎健

@version    :   3.0  2024/12/03
*/

#include <stdio.h>
#include <stdarg.h>
#include "Uart_dev.h"
#include "usart.h"
#include "main.h"


extern Uart_dev_t uart_dev1; 
extern Uart_dev_t uart_dev4; 
extern Uart_dev_t uart_dev5; 


/**
  * @brief  Reception Event Callback (Rx event notification called after use of advanced reception service).
  * @param  huart UART handle
  * @param  Size  Number of data available in application reception buffer (indicates a position in
  *               reception buffer until which, data are available)
  * @retval None
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{     
    if(huart == (&huart1))
    {
        uart_dev1.RxCallBack(&uart_dev1, huart, Size);
        
    }
      else if(huart == (&huart4))
    {
        uart_dev4.RxCallBack(&uart_dev4, huart, Size);
        process_uart4();
        
    }
      else if(huart == (&huart5))
    {
        uart_dev5.RxCallBack(&uart_dev5, huart, Size);
        process_uart5();

    }
    
}


/**
  * @brief  Tx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{   
    if(huart == (&huart1))
    {
    uart_dev1.TxCallBack(&uart_dev1, huart);
    }
    
    else if(huart == (&huart4)){
    uart_dev4.TxCallBack(&uart_dev4, huart);
    }
    
    else if(huart == (&huart5)){
        
    uart_dev5.TxCallBack(&uart_dev5, huart);
    }

}

/**
  * @brief  UART error callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{   
    if(huart == (&huart1))
    {
    uart_dev1.ErrorCallBack(&uart_dev1, huart);
    }
    
    else if(huart == (&huart4))
    {
        uart_dev4.ErrorCallBack(&uart_dev4, huart);
    }
    
    else if(huart == (&huart5))
    {
        uart_dev5.ErrorCallBack(&uart_dev5, huart);
    }
 
}