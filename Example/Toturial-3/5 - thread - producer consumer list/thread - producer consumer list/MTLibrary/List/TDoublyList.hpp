#pragma once
#include <Windows.h>
#include "MTLibrary/MemoryCounter.h"
#include "MTLibrary/Logger.h"

#include <typeinfo>

template < class DATA_TYPE >
struct TDNode : DATA_TYPE
{
	UINT                 NodeNumber;
	TDNode< DATA_TYPE > *pPrev;
	TDNode< DATA_TYPE > *pNext;
};

template < class DATA_TYPE >
class TDoublyList
{
public :
	TDNode< DATA_TYPE > *pHeadNode;
	TDNode< DATA_TYPE > *pTailNode;
	UINT                 NodeUseCount;
	UINT                 NodeLinkCount;

	TDNode< DATA_TYPE > *pFreeList;
	UINT                 FreeListCount;

	TDoublyList()
	{
		Create();
	}
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
		TDNode< DATA_TYPE > *node = pHeadNode, *del_node = NULL;
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

	inline TDNode< DATA_TYPE> * GetNode()
	{
		TDNode< DATA_TYPE > *node = NULL;
		if( FreeListCount > 0 )
		{
			node = pFreeList;
			pFreeList = pFreeList->pNext;
			--FreeListCount;
		}
		else
		{
			MY_NEW( node, TDNode< DATA_TYPE > );
			ZeroMemory( node, sizeof( TDNode< DATA_TYPE > ) );
		}

		node->NodeNumber = 0;
		node->pPrev = NULL;
		node->pNext = NULL;

		++NodeUseCount;
		return node;
	}

	inline void  ReleaseNode( TDNode< DATA_TYPE > *&release_node )
	{
		if( !release_node )
		{
			Logger::Log( "TDoublyList< %s >::ReleaseNodeEx   error : release_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}
		if( NodeUseCount == 0 )
		{
			Logger::Log( "TDoublyList< %s >::ReleaseNodeEx   error : NodeUseCount = 0", typeid( DATA_TYPE ).name() );
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
	inline void  FreeNode( TDNode< DATA_TYPE > *&release_node )
	{
		if( !release_node )
		{
			Logger::Log( "TDoublyList< %s >::ReleaseNode   error : release_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}
		if( NodeUseCount == 0 )
		{
			Logger::Log( "TDoublyList< %s >::ReleaseNode   error : NodeUseCount = 0", typeid( DATA_TYPE ).name() );
			return;
		}

		ZeroMemory( release_node, sizeof( TDNode< DATA_TYPE > ) );

		if( FreeListCount > 0 )
			release_node->pNext = pFreeList;
		pFreeList = release_node;
		release_node = NULL;
		++FreeListCount;
		--NodeUseCount;
	}

	inline void Link( TDNode< DATA_TYPE > *node )
	{
		if( !node )
		{
			Logger::Log( "TDoublyList< %s >::Link   link node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}

		node->pPrev = NULL;
		node->pNext = NULL;

		if( !pHeadNode )
		{
			pTailNode = pHeadNode = node;
		}
		else
		{
			pTailNode->pNext = node;
			node->pPrev = pTailNode;
			pTailNode = node;
		}
		++NodeLinkCount;
	}
	inline void LinkToHead( TDNode< DATA_TYPE > *node )
	{
		if( !node )
		{
			Logger::Log( "TDoublyList< %s >::LinkToHead   link node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}

		node->pPrev = NULL;
		node->pNext = NULL;

		if( !pHeadNode )
		{
			pTailNode = pHeadNode = node;
		}
		else
		{
			pHeadNode->pPrev = node;
			node->pNext = pHeadNode;
			pHeadNode = node;
		}
		++NodeLinkCount;
	}

	inline BOOL Unlink( TDNode< DATA_TYPE > *node )
	{
		if( !node )
		{
			Logger::Log( "TDoublyList< %s >::Unlink   error : unlink node = NULL", typeid( DATA_TYPE ).name() );
			return FALSE;
		}

		if( node->pPrev )
		{
			if( node->pNext ) // middle node
			{
				node->pPrev->pNext = node->pNext;
				node->pNext->pPrev = node->pPrev;
			}
			else // tail node
			{
				if( pTailNode == node )
				{
					pTailNode = pTailNode->pPrev;
					pTailNode->pNext = NULL;
				}
				else
				{
					Logger::Log( "TDoublyList< %s >::Unlink   if( pTailNode != node ) it will lose the node.", typeid( DATA_TYPE ).name() );
					return FALSE;
				}
			}
		}
		else // head node
		{
			if( pHeadNode == node )
			{
				if( pHeadNode->pNext )
				{
					pHeadNode = pHeadNode->pNext;
					pHeadNode->pPrev = NULL;
				}
				else
				{
					pHeadNode = NULL;
					pTailNode = NULL;
				}
			}
			else
			{
				Logger::Log( "TDoublyList< %s >::Unlink   if( pHeadNode != node ) it will lose the node.", typeid( DATA_TYPE ).name() );
				return FALSE;
			}
		}

		node->pPrev = NULL;
		node->pNext = NULL;

		if( NodeLinkCount > 0 )
			--NodeLinkCount;
		else
			Logger::Log( "TDoublyList< %s >::Unlink   error : NodeLinkCount = 0", typeid( DATA_TYPE ).name() );
		return TRUE;
	}

	inline void LinkInsertPrev( TDNode< DATA_TYPE > *dest_node, TDNode< DATA_TYPE > *new_node )
	{
		if( !new_node )
		{
			Logger::Log( "TDoublyList< %s >::LinkInsertPrev   new_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}

		TDNode< DATA_TYPE > *prev_node = NULL;
		if( dest_node )
			prev_node = dest_node->pPrev;

		if( dest_node == NULL )
		{
			new_node->pPrev = NULL;
			if( !pHeadNode )
			{
				new_node->pNext = NULL;
				pHeadNode = pTailNode = new_node;
			}
			else
			{
				new_node->pNext  = pHeadNode;
				pHeadNode->pPrev = new_node;

				pHeadNode = new_node;
			}
		}
		else
		{
			new_node->pPrev  = prev_node;
			new_node->pNext  = dest_node;
			if( prev_node )
				prev_node->pNext = new_node;
			dest_node->pPrev = new_node;

			if( !prev_node )
				pHeadNode = new_node;
		}
		++NodeLinkCount;
	}
	inline void LinkInsertNext( TDNode< DATA_TYPE > *dest_node, TDNode< DATA_TYPE > *new_node )
	{
		if( !new_node )
		{
			Logger::Log( "TDoublyList< %s >::LinkInsertNext   new_node = NULL", typeid( DATA_TYPE ).name() );
			return;
		}
		TDNode< DATA_TYPE > *next_node = NULL;
		if( dest_node )
			next_node = dest_node->pNext;

		if( dest_node == NULL )
		{
			new_node->pPrev = NULL;
			if( !pHeadNode )
			{
				new_node->pNext = NULL;
				pHeadNode = pTailNode = new_node;
			}
			else
			{
				new_node->pNext  = pHeadNode;
				pHeadNode->pPrev = new_node;

				pHeadNode = new_node;
			}
		}
		else
		{
			new_node->pPrev = dest_node;
			new_node->pNext = next_node;
			if( next_node )
				next_node->pPrev = new_node;
			dest_node->pNext = new_node;

			if( !next_node )
				pTailNode = new_node;
		}
		++NodeLinkCount;
	}
	inline BOOL LinkInsertOrderWithNumber( TDNode< DATA_TYPE > *new_node )
	{
		if( !pHeadNode )
		{
			Link( new_node );
			return TRUE;
		}
		BOOL flag = FALSE;
		TDNode< DATA_TYPE > *prev_node = NULL;
		TDNode< DATA_TYPE > *node = pHeadNode;
		while( node )
		{
			if( new_node->NodeNumber >= node->NodeNumber )
			{
				if( node->pNext == NULL )
				{
					flag = TRUE;
					Link( new_node );
					break;
				}
			}
			else
			{
				flag = TRUE;
				LinkInsertPrev( node, new_node );
				break;
			}
			node = node->pNext;
		}
		return flag;
	}

	inline DATA_TYPE* SearchNodeWithNumber( UINT node_number )
	{
		TDNode< DATA_TYPE > *node = pHeadNode;
		while( node )
		{
			if( node->NodeNumber == node_number )
				return node;
			node = node->pNext;
		}
		return NULL;
	}
};