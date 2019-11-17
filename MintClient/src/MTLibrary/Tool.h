#pragma once

class Tool
{
public:
	static void sMemcpy(void* dest_buff, UINT dest_buff_length, void* src_buff, UINT src_buff_length);
	static bool sDelayTime(INT64& start_time, INT64& end_time, UINT delay_time);
	static void sBeep(){ ::Beep(1000, 150); }
};

#define D_WARNING() \
{ \
	Tool::sBeep(); \
	Logger::Log( "D_WARNING   error : file= %s, func= %s, code_line= %d", __FILE__, __FUNCTION__, (DWORD)__LINE__ ); \
}

#define D_CHECK( pointer ) \
{ \
	if( !pointer ) \
	{ \
		Tool::sBeep(); \
		Logger::Log( "D_CHECK   error : obj_name= %s, file= %s, func= %s, code_line= %d", typeid( pointer ).name(), __FILE__, __FUNCTION__, (DWORD)__LINE__ ); \
		return; \
	} \
}

#define D_CHECK_BREAK( pointer ) \
{ \
	if( !pointer ) \
	{ \
		Tool::sBeep(); \
		Logger::Log( "D_CHECK_BREAK   error : obj_name= %s, file= %s, func= %s, code_line= %d", typeid( pointer ).name(), __FILE__, __FUNCTION__, (DWORD)__LINE__ ); \
		break; \
	} \
}