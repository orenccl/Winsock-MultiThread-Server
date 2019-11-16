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

bool Tool::sDelayTime(INT64& start_time, INT64& end_time, UINT delay_time)
{
	// 延遲( 使用系統時間方式 )

	if (!start_time)
	{
		if (!delay_time)
			return true;
		start_time = ::timeGetTime();
		return false;
	}
	end_time = ::timeGetTime();

	if ((end_time - start_time) >= delay_time)
	{
		start_time = end_time;
		return true;
	}
	return false;
}