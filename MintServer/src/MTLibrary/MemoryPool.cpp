#include "mtpch.h"
#include "MTLibrary/MemoryPool.h"
#include "MTLibrary/Logger.h"
#include "MTLibrary/MemoryCounter.h"

struct ExtraBufferNode // Use it when main memory pool is not enough
{
	BYTE             *pBuffer;
	ExtraBufferNode  *pNext;
};

class PrivateMMP : public MainMemoryPool
{
public :
	static BYTE  *spBuffer;						// MemoryPool start
	static UINT   sBufferByteMax;				// MemoryPool maximum byte
	static BYTE  *spNowBufferPositionLink;		// MemoryPool current address
	static UINT   sNowBufferUseLength;			// MemoryPool current length

	static ExtraBufferNode *spExtraBufferHead;	// Extra buffer head link
	static UINT             sExtraBufferSize;	// Extra buffer size
	
	static void sMyBeep() {::Beep(1000, 150);}  // Beep!
};

BYTE  *PrivateMMP::spBuffer;
UINT   PrivateMMP::sBufferByteMax;
BYTE  *PrivateMMP::spNowBufferPositionLink;
UINT   PrivateMMP::sNowBufferUseLength;
ExtraBufferNode *PrivateMMP::spExtraBufferHead;
UINT             PrivateMMP::sExtraBufferSize;

BOOL MainMemoryPool::sMemoryPoolCreate( UINT byte_max )
{
	// Initialize Memory Pool

	PrivateMMP::sBufferByteMax = byte_max;
	MY_NEW( PrivateMMP::spBuffer, BYTE[ byte_max ] );
	if( !PrivateMMP::spBuffer )
	{
		PrivateMMP::sMyBeep();
		Logger::Log( "MainMemoryPool::sMemoryPoolCreate   new failed. memory= %d (%d MB)", byte_max, byte_max / 1024 / 1024 );
		return FALSE;
	}
	::memset( PrivateMMP::spBuffer, 0, byte_max );
	PrivateMMP::spNowBufferPositionLink = PrivateMMP::spBuffer;
	PrivateMMP::sNowBufferUseLength = 0;
	
	return TRUE;
}

void MainMemoryPool::sMemoryPoolRelease()
{
	if( PrivateMMP::spBuffer )
	{
		MY_ARRAY_DELETE( PrivateMMP::spBuffer );
		PrivateMMP::spBuffer = NULL;
		PrivateMMP::spNowBufferPositionLink = NULL;
		PrivateMMP::sBufferByteMax = 0;
		PrivateMMP::sNowBufferUseLength = 0;
	}

	if( PrivateMMP::spExtraBufferHead )
	{
		ExtraBufferNode *node = PrivateMMP::spExtraBufferHead, *del_node = NULL;
		while( node )
		{
			del_node = node;
			node = node->pNext;
			MY_ARRAY_DELETE( del_node->pBuffer );
			MY_DELETE( del_node );
		}
		PrivateMMP::spExtraBufferHead = NULL;
	}
}


void* MainMemoryPool::sMemoryAllocate( UINT byte_max )
{
	// Ask Main Memory Pool for space

	// If memory pool is not enough
	if( (PrivateMMP::sNowBufferUseLength + byte_max) > PrivateMMP::sBufferByteMax )
	{
		// New extra space
		BYTE *buffer = NULL;
		MY_NEW( buffer, BYTE[ byte_max ] );
		if( !buffer )
		{
			Logger::Log( "MainMemoryPool::sMemoryAllocate   #1 new the memory(%d) failed. (%d MB)", byte_max, byte_max / 1024 / 1024 );
			PrivateMMP::sMyBeep();
			return NULL;
		}
		::memset( buffer, 0, byte_max );
		
		// Link every extra space pointer
		const UINT node_size = sizeof( ExtraBufferNode );
		ExtraBufferNode *node = NULL;
		MY_NEW( node, ExtraBufferNode() );
		if( node )
		{
			::memset( node, 0, node_size );
			node->pBuffer = buffer;
			PrivateMMP::sExtraBufferSize += node_size + byte_max;
			
			if( PrivateMMP::spExtraBufferHead == NULL )
			{
				PrivateMMP::spExtraBufferHead = node;
			}
			else
			{
				node->pNext = PrivateMMP::spExtraBufferHead;
				PrivateMMP::spExtraBufferHead = node;
			}

			return node->pBuffer;
		}

		Logger::Log( "MainMemoryPool::sMemoryAllocate   #2 new the memory(%d) failed. (%d MB)", node_size, node_size / 1024 / 1024 );
		PrivateMMP::sMyBeep();
		return NULL;
	}

	// Devide a block memory and return
	BYTE *block = PrivateMMP::spNowBufferPositionLink;
	PrivateMMP::spNowBufferPositionLink += byte_max;
	PrivateMMP::sNowBufferUseLength += byte_max;
	return block;
}

BOOL MainMemoryPool::sCheckMemoryPoolCreate()
{
	if( PrivateMMP::spBuffer )
		return TRUE;
	return FALSE;
}

UINT MainMemoryPool::sGetMaxMemoryPoolBytes()
{
	return PrivateMMP::sBufferByteMax;
}

UINT MainMemoryPool::sGetUseMemoryPoolBytes()
{
	return PrivateMMP::sNowBufferUseLength;
}

UINT MainMemoryPool::sGetExtraUseMemoryPoolBytes()
{
	return PrivateMMP::sExtraBufferSize;
}