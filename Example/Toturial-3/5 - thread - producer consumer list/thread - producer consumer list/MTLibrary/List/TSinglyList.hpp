#pragma once
#include <Windows.h>
#include "MTLibrary/MemoryCounter.h"
#include "MTLibrary/Logger.h"

#include <typeinfo>

template < class DATA_TYPE >
struct TSNode : DATA_TYPE
{
	UINT                 NodeNumber;
	TSNode< DATA_TYPE > *pNext;
};

template < class DATA_TYPE >
class TSinglyList
{
public :
	TSNode< DATA_TYPE > *pHeadNode;
	TSNode< DATA_TYPE > *pTailNode;
	UINT                 NodeUseCount;
	UINT                 NodeLinkCount;

	TSNode< DATA_TYPE > *pFreeList;
	UINT                 FreeListCount;

	inline void Create()
	{
		pHeadNode = NULL;
		pTailNode = NULL;
		NodeUseCount = 0;
		NodeLinkCount = 0;

		pFreeList = NULL;
		FreeListCount = 0;
	}

	inline void Release()
	{
		TSNode< DATA_TYPE > *node = pHeadNode, *del_node = NULL;
		while( node )
		{
			del_node = node;
			node = node->pNext;
			MY_DELETE( del_node );
		}

		node = pFreeList;
		while( node )
		{
			del_node = node;
			node = node->pNext;
			MY_DELETE( del_node );
		}

		Create(); // clear to default value.
	}
	
	inline TSNode< DATA_TYPE >* GetNode()
	{
		TSNode< DATA_TYPE > *node = NULL;
		if( FreeListCount > 0 )
		{
			node = pFreeList;
			pFreeList = pFreeList->pNext;
			--FreeListCount;

			node->pNext = NULL;
		}
		else
		{
			MY_NEW( node, TSNode< DATA_TYPE > );
			ZeroMemory( node, sizeof( TSNode< DATA_TYPE > ) );
		}
		++NodeUseCount;
		return node;
	}
	
	inline void ReleaseNode( TSNode< DATA_TYPE > *&release_node )
	{
		if( !release_node )
		{
			Logger::Log( "TSinglyList< %s >::ReleaseNodeEx   error : release_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}
		if( NodeUseCount == 0 )
		{
			Logger::Log( "TSinglyList< %s >::ReleaseNodeEx   error : NodeUseCount = 0", typeid( DATA_TYPE ).name() );
			return;
		}
		
		MY_DELETE( release_node );
		--NodeUseCount;
	}

	/*!
	* @brief Free node to freelist, store for later use.
	* @details
	* Prevent repeatedly New and Delete to cause memory broken.
	*/
	inline void FreeNode( TSNode< DATA_TYPE > *&release_node )
	{
		if( !release_node )
		{
			Logger::Log( "TSinglyList< %s >::ReleaseNode   error : release_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}
		if( NodeUseCount == 0 )
		{
			Logger::Log( "TSinglyList< %s >::ReleaseNode   error : NodeUseCount = 0", typeid( DATA_TYPE ).name() );
			return;
		}
		
		ZeroMemory( release_node, sizeof( TSNode< DATA_TYPE > ) );

		if( FreeListCount > 0 )
			release_node->pNext = pFreeList;
		pFreeList = release_node;
		release_node = NULL;
		++FreeListCount;
		--NodeUseCount;
	}

	inline void  Link( TSNode< DATA_TYPE > *node )
	{
		if( !node )
		{
			Logger::Log( "TSinglyList< %s >::Link   error : link node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}

		node->pNext = NULL;

		if( !pHeadNode )
		{
			pTailNode = pHeadNode = node;
		}
		else
		{
			pTailNode->pNext = node;
			pTailNode = node;
		}
		++NodeLinkCount;
	}

	inline void  LinkToHead( TSNode< DATA_TYPE > *node )
	{
		if( !node )
		{
			Logger::Log( "TSinglyList< %s >::LinkToHead   error : link node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}

		node->pNext = NULL;

		if( !pHeadNode )
		{
			pTailNode = pHeadNode = node;
		}
		else
		{
			node->pNext = pHeadNode;
			pHeadNode = node;
		}
		++NodeLinkCount;
	}

	inline void Unlink( TSNode< DATA_TYPE > *unlink_node )
	{
		if( !unlink_node )
		{
			Logger::Log( "TSinglyList< %s >::Unlink   error : unlink_node = NULL", typeid( DATA_TYPE ).name() );
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
			Logger::Log( "TSinglyList< %s >::Unlink   error : node get_flag = FALSE", typeid( DATA_TYPE ).name() );
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
				pTailNode = pHeadNode = NULL;
			}
			else
				pHeadNode = pHeadNode->pNext;
		}

		unlink_node->pNext = NULL;

		if( NodeLinkCount > 0 )
			--NodeLinkCount;
		else
			Logger::Log( "TSinglyList< %s >::Unlink   error : NodeLinkCount = 0", typeid( DATA_TYPE ).name() );
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