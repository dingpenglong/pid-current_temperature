#include <string.h>
#include <stdlib.h>

#include "ring_buffer.h"
#include "ring_buffer_conf.h"


//打印调试信息
#ifdef RING_BUFFER_PRINT_DEBUG_INFO 
#define RING_BUFFER_PRINT_DEBUG(fmt,args...) do{printf("file:%s(%d) func %s:\n", __FILE__,__LINE__,  __FUNCTION__);printf(fmt, ##args);}while(0)
#else
#define RING_BUFFER_PRINT_DEBUG(fmt,args...) 
#endif

//检查指针非空
#define CHECK_PTR(ptr, param, field) do{\
                                    if(ptr == NULL) \
                                    { \
                                        param.field = 1;\
										RING_BUFFER_PRINT_DEBUG("pointer %s is NULL.\n", #ptr);\
                                        return param;\
                                    } \
                                }while(0)

//检查参数合法
#define CHECK_PARAM(cond, param, field) do{\
                                    if(cond) \
                                    { \
                                        param.field = 1;\
										RING_BUFFER_PRINT_DEBUG("%s is false.\n", #cond);\
                                        return param;\
                                    } \
                                }while(0)

                                

/*
 * 循环数据块队列操作函数
 *
 * */
 
 

/*
以下函数对用户不可见
*/

//初始化环形缓冲区
static RingBuffer_Error_t RingBuffer_Init(RingBuffer * buffer, const uint32_t data_size, const uint32_t max_num);
                                
//销毁队列
static RingBuffer_Error_t RingBuffer_DeInit(RingBuffer * buffer);
                                
//判断队列空
static RingBuffer_Error_t RingBuffer_IsEmpty(RingBuffer * buffer);
    
//判断队列满
static RingBuffer_Error_t RingBuffer_IsFull(RingBuffer * buffer);
    
//获取当前队列的空余元素数量
static RingBuffer_Error_t RingBuffer_FreeSpace(RingBuffer * buffer, uint32_t *num);                                

//入队,更新队尾
static RingBuffer_Error_t RingBuffer_Push(RingBuffer * buffer, const uint8_t* data);

//出队，更新队首
static RingBuffer_Error_t RingBuffer_Pop(RingBuffer * buffer, uint8_t* data);

//销毁队列
static RingBuffer_Error_t RingBuffer_DeInit(RingBuffer * buffer);

//清空队列
static RingBuffer_Error_t RingBuffer_Clear(RingBuffer * buffer);

//查看队首数据
static RingBuffer_Error_t RingBuffer_PeekHead(RingBuffer * buffer, uint8_t* data);

//查看队尾数据
static RingBuffer_Error_t RingBuffer_PeekTail(RingBuffer * buffer, uint8_t* data);
 

 
 

//初始化数据队列
/*
buffer      : 队列指针
data_size   ：一个数据所占的字节
max_num     : 队列可以存储的最大数据量
*/

static RingBuffer_Error_t RingBuffer_Init(RingBuffer * buffer, const uint32_t data_size, const uint32_t max_num)
{
	RingBuffer_Error_t err = {0};
    CHECK_PTR(buffer, err, MALLOC);
    
    //若最大数据量或者元素的大小为0，则报错
    CHECK_PARAM(!(max_num|data_size), err, SIZE);
    
    buffer->m_cur_num   = 0;
    buffer->m_first     = 0;
    buffer->m_rear      = 0;
	buffer->m_wait_read = 0;
    buffer->m_data_size = data_size;
    buffer->m_max_num   = max_num;
	buffer->m_max_index = max_num * data_size;
    buffer->m_data = RING_BUFFER_MALLOC(uint8_t, max_num * data_size);
	
	//若队列最大存储数量不为0, 则说明单个数据所占用的字节太大了 
    CHECK_PTR(buffer->m_data, err, SIZE);
	
	return err;
}

//销毁队列
static RingBuffer_Error_t RingBuffer_DeInit(RingBuffer * buffer)
{
    RingBuffer_Error_t err = {0};;
	CHECK_PTR(buffer, err, MALLOC);
    
	RING_BUFFER_FREE(buffer->m_data);
	
	buffer->m_wait_read	= 0;
	buffer->m_cur_num   = 0;
    buffer->m_first     = 0;
    buffer->m_rear      = 0;
    buffer->m_data_size = 0;
    buffer->m_max_num   = 0;
    buffer->m_max_index = 0;
	
	buffer->Init 	    = NULL;
	buffer->DeInit 	    = NULL;
    buffer->IsEmpty     = NULL;
    buffer->IsFull      = NULL;
    buffer->FreeSpace   = NULL;
	buffer->Clear 	    = NULL;
	buffer->Pop 	    = NULL;
	buffer->Push	    = NULL;
	buffer->PeekHead    = NULL;
	buffer->PeekTail    = NULL;
    
    return err;
}

//判断队列空
static RingBuffer_Error_t RingBuffer_IsEmpty(RingBuffer * buffer)
{
    RingBuffer_Error_t err = {0};
	CHECK_PTR(buffer, err, MALLOC);
    
    //检查队空
    if(!buffer->m_cur_num)
        err.EMPTY = 1;
    
    return err;
}
    
//判断队列满
static RingBuffer_Error_t RingBuffer_IsFull(RingBuffer * buffer)
{
    RingBuffer_Error_t err = {0};
	CHECK_PTR(buffer, err, MALLOC);
    
    //检查队满
    if(buffer->m_max_num == buffer->m_cur_num)
        err.FULL = 1;
    
    return err;
}   
    
//获取当前队列的空余元素数量
static RingBuffer_Error_t RingBuffer_FreeSpace(RingBuffer * buffer, uint32_t *num)
{
    RingBuffer_Error_t err = {0};;
	CHECK_PTR(buffer, err, MALLOC);
    
    *num = buffer->m_max_num - buffer->m_cur_num;
    
    return err;
}

//入队,更新队尾
static RingBuffer_Error_t RingBuffer_Push(RingBuffer * buffer, const uint8_t* data)
{
	RingBuffer_Error_t err = {0};;
	CHECK_PTR(buffer, err, MALLOC);
    CHECK_PTR(data, err, MALLOC);
	
	//如果队满, 则无法入队
    CHECK_PARAM(buffer->m_max_num == buffer->m_cur_num, err, PUSH);
	
	memmove(&buffer->m_data[buffer->m_rear], data, buffer->m_data_size);
	buffer->m_rear = (buffer->m_rear + buffer->m_data_size) % buffer->m_max_index;
    buffer->m_cur_num++;

    return err;
}

//出队，更新队首
static RingBuffer_Error_t RingBuffer_Pop(RingBuffer * buffer, uint8_t* data) 
{
	RingBuffer_Error_t err = {0};;
	CHECK_PTR(buffer, err, MALLOC);
	
	//如果队空, 则无法出队
    CHECK_PARAM(!buffer->m_cur_num, err, POP);
	
    //如果用于接收队首的指针为空，则默认丢弃队首数据
	if(data != NULL)
		memmove(data, &buffer->m_data[buffer->m_first], buffer->m_data_size);
	
	buffer->m_first = (buffer->m_first + buffer->m_data_size) % buffer->m_max_index;
    buffer->m_cur_num--;
    
	return err;
}


//清空队列
static RingBuffer_Error_t RingBuffer_Clear(RingBuffer * buffer)
{
	RingBuffer_Error_t err = {0};;
	CHECK_PTR(buffer, err, MALLOC);
	
	buffer->m_wait_read	= 0;
    buffer->m_cur_num	= 0;
    buffer->m_first		= 0;
    buffer->m_rear		= 0;
	
	return err;
}

//查看队首数据
static RingBuffer_Error_t RingBuffer_PeekHead(RingBuffer * buffer, uint8_t* data)
{
    RingBuffer_Error_t err = {0};;
	CHECK_PTR(buffer, err, MALLOC);
    CHECK_PARAM(data == NULL, err, MALLOC);
    
	memmove(data, &buffer->m_data[buffer->m_first], buffer->m_data_size);
    
	return err;    
}

//查看队尾数据
static RingBuffer_Error_t RingBuffer_PeekTail(RingBuffer * buffer, uint8_t* data)
{
    RingBuffer_Error_t err = {0};;
    CHECK_PTR(buffer, err, MALLOC);
    CHECK_PARAM(data == NULL, err, MALLOC);
    
    memmove(&buffer->m_data[buffer->m_rear], data, buffer->m_data_size);
    
	return err;    
}

 //公开给用户初始化环形数据缓冲区对象的函数接口
RingBuffer_Error_t RingBuffer_API_INIT(RingBuffer *buffer)
{
	RingBuffer_Error_t err = {0};;
    CHECK_PTR(buffer, err, MALLOC);
    
	buffer->Init        = RingBuffer_Init;
	buffer->DeInit      = RingBuffer_DeInit;
    buffer->IsEmpty     = RingBuffer_IsEmpty;
    buffer->IsFull      = RingBuffer_IsFull;
    buffer->FreeSpace   = RingBuffer_FreeSpace;
	buffer->Clear       = RingBuffer_Clear;
	buffer->Pop         = RingBuffer_Pop;
	buffer->Push        = RingBuffer_Push;
    buffer->PeekHead    = RingBuffer_PeekHead;
    buffer->PeekTail    = RingBuffer_PeekTail;
	
	return err;
}
