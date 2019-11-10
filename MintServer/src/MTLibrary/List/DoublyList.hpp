#pragma once
#include "mtpch.h"
#include "MTLibrary/MemoryCounter.h"
#include "MTLibrary/Logger.h"

struct DNode
{
	UINT   NodeNumber;
	void  *pData;
	DNode *pPrev;
	DNode *pNext;
};

class DoublyList
{
public :
	DNode *pHeadNode;
	DNode *pTailNode;
	UINT   NodeUseCount;
	UINT   NodeLinkCount;
	
	DNode *pFreeList;
	UINT   FreeListCount;

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
		DNode *node = pHeadNode, *del_node = NULL;
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
	
	inline DNode* GetNode( void *user_data )
	{
		DNode *node = NULL;
		if( FreeListCount > 0 )
		{
			node = pFreeList;
			pFreeList = pFreeList->pNext;
			--FreeListCount;
		}
		else
		{
			MY_NEW( node, DNode );
		}
		node->NodeNumber = 0;
		node->pData = user_data;
		node->pPrev = NULL;
		node->pNext = NULL;
		++NodeUseCount;
		return node;
	}
	
	inline void ReleaseNode( DNode *&release_node )
	{
		if( !release_node )
		{
			Logger::Log( "DoublyList::ReleaseNodeEx   error : release_node = NULL" );
			return;
		}
		if( NodeUseCount == 0 )
		{
			Logger::Log( "DoublyList::ReleaseNodeEx   error : NodeUseCount = 0" );
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
	inline void FreeNode( DNode *&release_node )
	{
		if( !release_node )
		{
			Logger::Log( "DoublyList::ReleaseNode   error : release_node = NULL" );
			return;
		}
		if( NodeUseCount == 0 )
		{
			Logger::Log( "DoublyList::ReleaseNode   error : NodeUseCount = 0" );
			return;
		}
		
		ZeroMemory( release_node, sizeof( DNode ) );

		if( FreeListCount > 0 )
			release_node->pNext = pFreeList;
		pFreeList = release_node;
		release_node = NULL;
		++FreeListCount;
		--NodeUseCount;
	}
	
	inline void Link( DNode *node )
	{
		if( !node )
		{
			Logger::Log( "DoublyList::Link   link node = NULL" );
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

	inline void LinkToHead( DNode *node )
	{
		if( !node )
		{
			Logger::Log( "TDoublyList::LinkToHead   link node = NULL" );
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
	
	inline BOOL Unlink( DNode *node )
	{
		if( !node )
		{
			Logger::Log( "DoublyList::Unlink   error : unlink node = NULL" );
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
					Logger::Log( "DoublyList::Unlink   if( pTailNode != node ) it will lose the node." );
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
				Logger::Log( "DoublyList::Unlink   if( pHeadNode != node ) it will lose the node." );
				return FALSE;
			}
		}

		node->pPrev = NULL;
		node->pNext = NULL;

		if( NodeLinkCount > 0 )
			--NodeLinkCount;
		else
			Logger::Log( "DoublyList::Unlink   error : NodeLinkCount = 0" );
		return TRUE;
	}
	
	inline DNode* SearchNodeWithNumber( UINT node_number )
	{
		DNode *node = pHeadNode;
		while( node )
		{
			if( node->NodeNumber == node_number )
				return node;
			node = node->pNext;
		}
		return NULL;
	}
};