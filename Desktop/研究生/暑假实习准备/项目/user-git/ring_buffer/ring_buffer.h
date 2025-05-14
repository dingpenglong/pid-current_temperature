/*
@filename	:	ring_buffer.h
@brief		:	定义了一个环形缓冲队列，并提供了相应的面向对象的操作函数接口

@time		:	2023/11/15
@author		: 	丁鹏龙

@version    :   
                1.1 2024/11/30
                （1）m_wait_read标志改为接收帧计数机制；
                （2）增加PeekHead函数用于查看队首数据，增加PeekTail函数用于查看队尾数据
*/



#ifndef _USER_RING_BUFFER_H_
#define _USER_RING_BUFFER_H_

#include "stdint.h"

//环形数据缓冲区错误定义
typedef union
{
	struct
	{
		uint8_t MALLOC	: 1; //申请内存出错或待操作的对象为空指针
		uint8_t PUSH 	: 1; //上溢出, 即缓冲区已满时入队
		uint8_t POP		: 1; //下溢出, 即缓冲区已空时出队
		uint8_t SIZE    : 1; //单个数据的空间过大或者为0
		uint8_t EMPTY   : 1; //队列空
        uint8_t FULL    : 1; //队列满
        uint8_t         : 2; //保留
	};
	
	uint8_t value;
}RingBuffer_Error_t;




//环形数据缓冲区结构体
typedef struct RingBuffer RingBuffer;

struct RingBuffer{
	uint8_t*	m_data;       //队列数据指针
	uint32_t    m_wait_read; //队列期望读数据操作的次数 
	int     	m_first;      //队列首指针
	int     	m_rear;       //队列尾指针
	int     	m_cur_num;    //队列当前存储的数据量个数
	uint32_t    m_data_size;  //一个数据所占的字节数
    uint32_t    m_max_num;    //队列中所能容纳的数据量  
	uint32_t    m_max_index;  //队列中m_data的最大下标
	
	//初始化环形数据缓冲区
	/*
	buffer      : 队列指针
	data_size   ：一个数据所占的字节
	max_num     : 队列可以存储的最大数据量
	*/
	RingBuffer_Error_t (*Init) (RingBuffer * buffer, const uint32_t data_size, const uint32_t max_num);
	
	//销毁环形数据缓冲区
	RingBuffer_Error_t (*DeInit) (RingBuffer * buffer);
    
    //判断队列空
    RingBuffer_Error_t (*IsEmpty)(RingBuffer * buffer);
    
    //判断队列满
    RingBuffer_Error_t (*IsFull)(RingBuffer * buffer);
    
    //获取当前队列的空余元素数量
    RingBuffer_Error_t (*FreeSpace)(RingBuffer * buffer, uint32_t *num);
	
	//入队,更新队尾
	RingBuffer_Error_t (*Push)(RingBuffer * buffer, const uint8_t* data);
	
	//出队，更新队首
	RingBuffer_Error_t (*Pop) (RingBuffer * buffer, uint8_t* data);
	
	//清空环形数据缓冲区
	RingBuffer_Error_t (*Clear) (RingBuffer * buffer);
    
    //查看队首数据
    RingBuffer_Error_t (*PeekHead)(RingBuffer * buffer, uint8_t* data);
    
    //查看队尾数据
    RingBuffer_Error_t (*PeekTail)(RingBuffer * buffer, uint8_t* data);
};

//公开给用户初始化环形数据缓冲区对象的函数接口
RingBuffer_Error_t RingBuffer_API_INIT(RingBuffer *buffer);

#endif /* _USER_RING_BUFFER_H_ */
