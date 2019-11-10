#pragma once

class MainMemoryPool
{
public :

	static BOOL  sMemoryPoolCreate( UINT byte_max );
	static void  sMemoryPoolRelease();

	static void* sMemoryAllocate( UINT byte_max );

	static BOOL  sCheckMemoryPoolCreate();
	static UINT  sGetMaxMemoryPoolBytes();
	static UINT  sGetUseMemoryPoolBytes();
	static UINT  sGetExtraUseMemoryPoolBytes();
};