#pragma once

#include "Logger.h"

static long gMemoryUseCount = 0;
class MemoryCounter
{
public:
	static void sAdd_MemoryUseCount() { ++gMemoryUseCount; }
	static void sDel_MemoryUseCount() { --gMemoryUseCount; }
	static long sGet_MemoryUseCount() { return gMemoryUseCount; }
	static void sShow_MemoryUseCount() { Logger::Log("sShow_MemoryUseCount : %d", gMemoryUseCount); }
};

#define MY_NEW( object, new_type ) \
{ \
	object = new new_type; \
	MemoryCounter::sAdd_MemoryUseCount(); \
}

#define MY_DELETE( object ) \
{ \
	if( object ) \
		delete object; \
	object = NULL; \
	MemoryCounter::sDel_MemoryUseCount(); \
}

#define MY_ARRAY_DELETE( object ) \
{ \
	if( object ) \
		delete[] object; \
	object = NULL; \
	MemoryCounter::sDel_MemoryUseCount(); \
}

#define MY_RELEASE( object ) \
{ \
	if( object ) \
		object->vRelease(); \
	object = NULL; \
}