#include "mtpch.h"
#include "MTLibrary/Tool.h"

void Tool::sMemcpy(void* dest_buff, UINT dest_buff_length, void* src_buff, UINT src_buff_length)
{
	if (!dest_buff || !src_buff)
		return;

	if (src_buff_length > dest_buff_length)
	{
		memcpy(dest_buff, src_buff, dest_buff_length);
	}
	else
	{
		memcpy(dest_buff, src_buff, src_buff_length);
	}
}
