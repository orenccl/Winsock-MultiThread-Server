#pragma once
#include "mtpch.h"
#include "MTLibrary/List/TSinglyList.hpp"
#include "MTLibrary/MemoryPool.h"

template < class DATA_TYPE >
class TMPSinglyList
{
public :
	UINT                 ObjectCountMax;
	TSNode< DATA_TYPE > *pBufferList;
	UINT                 BufferListCount;

	TSNode< DATA_TYPE > *pHeadNode;
	TSNode< DATA_TYPE > *pTailNode;
	UINT                 NodeUseCount;
	UINT                 NodeLinkCount;

	TSNode< DATA_TYPE > *pFreeList;
	UINT                 FreeListCount;

	inline void Create( UINT object_max )
	{
		BufferListCount = ObjectCountMax = object_max;
		if( object_max > 0 )
		{
			const UINT object_size = sizeof( TSNode< DATA_TYPE > );
			pBufferList = (TSNode< DATA_TYPE >*)MainMemoryPool::sMemoryAllocate( object_size * object_max );
		}
		else
			pBufferList = NULL;

		pHeadNode = NULL;
		pTailNode = NULL;
		NodeUseCount = 0;
		NodeLinkCount = 0;

		pFreeList = NULL;
		FreeListCount = 0;
	}

	inline TSNode< DATA_TYPE >* GetNode()
	{
		TSNode< DATA_TYPE > *node = NULL;
		if( FreeListCount > 0 )
		{
			node = pFreeList;
			pFreeList = pFreeList->pNext;
			--FreeListCount;
		}
		else
		{
			if( BufferListCount > 0 )
			{
				node = pBufferList;
				++pBufferList;
				--BufferListCount;
			}
			else
			{
				Logger::Log( "TMPSinglyList< %s >::GetNode   warning: allocate a new node.", typeid( DATA_TYPE ).name() );
				node = (TSNode< DATA_TYPE >*)MainMemoryPool::sMemoryAllocate( sizeof( TSNode< DATA_TYPE > ) );
			}
		}
		node->pNext = NULL;
		++NodeUseCount;
		return node;
	}

	inline void FreeNode( TSNode< DATA_TYPE > *&release_node )
	{
		if( !release_node )
		{
			Logger::Log( "TMPSinglyList< %s >::ReleaseNode   error : release_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}
		if( NodeUseCount == 0 )
		{
			Logger::Log( "TMPSinglyList< %s >::ReleaseNode   error : NodeUseCount = 0", typeid( DATA_TYPE ).name() );
			return;
		}
		
		memset( release_node, 0, sizeof( TSNode< DATA_TYPE > ) );

		if( FreeListCount > 0 )
			release_node->pNext = pFreeList;
		pFreeList = release_node;
		release_node = NULL;
		++FreeListCount;
		--NodeUseCount;
	}

	inline void  Link( TSNode< DATA_TYPE > *link_node )
	{
		if( !link_node )
		{
			Logger::Log( "TMPSinglyList< %s >::Link   error : link_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}

		link_node->pNext = NULL;

		if( !pHeadNode )
		{
			pTailNode = pHeadNode = link_node;
		}
		else
		{
			pTailNode->pNext = link_node;
			pTailNode = link_node;
		}
		++NodeLinkCount;
	}
	inline void  LinkToHead( TSNode< DATA_TYPE > *link_node )
	{
		if( !link_node )
		{
			Logger::Log( "TMPSinglyList< %s >::LinkToHead   error : link_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}

		link_node->pNext = NULL;

		if( !pHeadNode )
		{
			pTailNode = pHeadNode = link_node;
		}
		else
		{
			link_node->pNext = pHeadNode;
			pHeadNode = link_node;
		}
		++NodeLinkCount;
	}

	inline void Unlink( TSNode< DATA_TYPE > *unlink_node )
	{
		if( !unlink_node )
		{
			Logger::Log( "TMPSinglyList< %s >::Unlink   error : unlink_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}

		TSNode< DATA_TYPE > *node = pHeadNode;
		TSNode< DATA_TYPE > *prev_node = NULL;
		BOOL get_flag = FALSE;

		while( node )
		{
			if( node == unlink_node )
			{
				get_flag = TRUE;
				break;
			}
			prev_node = node;
			node = node->pNext;
		}

		if( !get_flag )
		{
			Logger::Log( "TMPSinglyList< %s >::Unlink   error : node get_flag = FALSE", typeid( DATA_TYPE ).name() );
			return;
		}

		if( prev_node )
		{
			if( unlink_node == pTailNode )
			{
				pTailNode = prev_node;
				pTailNode->pNext = NULL;
			}
			else
				prev_node->pNext = unlink_node->pNext;
		}
		else
		{
			if( pHeadNode == pTailNode ) // only one node
			{
				pHeadNode = pHeadNode->pNext;
				pTailNode = pHeadNode;
			}
			else
				pHeadNode = pHeadNode->pNext;
		}

		unlink_node->pNext = NULL;

		if( NodeLinkCount > 0 )
			--NodeLinkCount;
		else
			Logger::Log( "TMPSinglyList< %s >::Unlink   error : NodeLinkCount = 0", typeid( DATA_TYPE ).name() );
	}

	inline TSNode< DATA_TYPE >* SearchNodeWithNumber( UINT node_number )
	{
		TSNode< DATA_TYPE > *node = pHeadNode;
		while( node )
		{
			if( node->NodeNumber == node_number )
				return node;
			node = node->pNext;
		}
		return NULL;
	}
};