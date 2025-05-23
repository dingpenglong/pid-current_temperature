#include <stdio.h>
#include "main.h"

//////////////////////////////////////////////////////////////////////////////////
//重定义标准输出至串口

//#ifdef  USE_FULL_ASSERT
//当编译提示出错的时候此函数用来报告错误的文件和所在行
//file：指向源文件
//line：指向在文件中的行数
//void assert_failed(uint8_t* file, uint32_t line)
//{ 
//	while (1)
//	{
//	}
//}
//#endif


//定义用于支持printf函数的串口, 默认使用串口1
#ifndef PRINTF_UART_HANDLE
#define PRINTF_UART_HANDLE	huart1 
#endif

extern UART_HandleTypeDef PRINTF_UART_HANDLE;


//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
//#pragma import(__use_no_semihosting)   

//AC 6 编译器需要替换为下面这句
__asm(".global __use_no_semihosting");

//标准库需要的支持函数                 
struct FILE 
{ 
	int handle; 
}; 

FILE __stdout;     

//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 


//fix the error : __use_no_semihosting was requested, but _ttywrch was referenced
void _ttywrch(int ch)
{
	ch = ch;
}

//fix the error : __use_no_semihosting was requested, but _sys_command_string was referenced
char* _sys_command_string(char *cmd, int len)
{
    return NULL;
} 

//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	//等待串口准备好发送数据
	while(PRINTF_UART_HANDLE.gState != HAL_UART_STATE_READY){};
	HAL_UART_Transmit(&PRINTF_UART_HANDLE, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

