

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "string.h"
#include "buffer.h"


void ring_buffer_init(RingBuffer *st, void *buffer, int size)
{
   st->head = 0;
   st->rear = 0;
   st->data = buffer;
   st->size = size;
}

void ring_buffer_destroy(RingBuffer *st)
{

}

int ring_buffer_write(RingBuffer *st, void *_data, int len)
{
    int ret = 0;
    while(len --)
    {
        ring_buffer_writeByte(st, _data++);
        ret ++;
    }

    return ret;

}

void ring_buffer_writeByte(RingBuffer *st, void *_data)
{
	st->data[st->head] = *((char*)_data);
	st->head ++;
	if(st->head >= st->size)
		st->head = 0;
	if(st->head == st->rear)
	{
		st->rear++;
		if(st->rear >= st->size)
			st->rear = 0;
	}
}

int ring_buffer_writezeros(RingBuffer *st, int len)
{
	return 0;
}

int ring_buffer_readByte(RingBuffer *st, void *_data)
{
    if( st->rear == st->head )
        return 0;

    *((char*)_data) = st->data[st->rear];
    //memcpy(_data, &st->data[st->rear], len);
    st->rear++;
    if(st->rear >= st->size)
        st->rear = 0;

    return 1;
}

int ring_buffer_read(RingBuffer *st, void *_data, int len)
{
    int ret = 0;
    while(len --)
    {
        if(!ring_buffer_readByte(st, _data++))
            break;
        ret ++;
    }

	return ret;
}

int ring_buffer_get_available(RingBuffer *st)
{
	int  count = 0;
	int  result = st->head - st->rear;

	if( result >= 0)
		count =  st->head - st->rear;
	else
		count =  (st->head - st->rear) + st->size;

	return count;
}

int ring_buffer_get_free(RingBuffer *st)
{
	int count = st->size - ring_buffer_get_available(st);

	return count;
}

int ring_buffer_resize(RingBuffer *st, int len)
{
   return 0;
}

void ring_buffer_flush(RingBuffer *st)
{
   st->head = 0;
   st->rear = 0;
}
