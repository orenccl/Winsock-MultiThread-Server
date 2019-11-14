#pragma once
#include <Windows.h>
#include "MTLibrary/MemoryCounter.h"
#include "MTLibrary/Logger.h"

struct SNode
{
	UINT   NodeNumber;
	void  *pData;
	SNode *pNext;
};

class SinglyList
{
public :
	SNode *pHeadNode;
	SNode *pTailNode;
	UINT   NodeUseCount;
	UINT   NodeLinkCount;

	SNode *pFreeList;
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
		SNode *node = pHeadNode, *del_node = NULL;
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
	
	inline SNode* GetNode( void *user_data )
	{
		SNode *node = NULL;
		if( FreeListCount > 0 )
		{
			node = pFreeList;
			pFreeList = pFreeList->pNext;
			--FreeListCount;
		}
		else
		{
			MY_NEW( node, SNode );
		}
		node->NodeNumber = 0;
		node->pData = user_data;
		node->pNext = NULL;
		++NodeUseCount;
		return node;
	}
	
	inline void ReleaseNode(SNode*& release_node)
	{
		if (!release_node)
		{
			Logger::Log("SinglyList::ReleaseNode   error : release_node = NULL");
			return;
		}
		if (NodeUseCount == 0)
		{
			Logger::Log("SinglyList::ReleaseNode   error : NodeUseCount= 0");
			return;
		}

		MY_DELETE(release_node);
		--NodeUseCount;
	}

	/*!
	* @brief Free node to freelist, store for later use.
	* @details 
	* Prevent repeatedly New and Delete to cause memory broken.
	*/
	inline void FreeNode( SNode *&release_node )
	{
		if( !release_node )
		{
			Logger::Log( "SinglyList::ReleaseNode   error : release_node = NULL" );
			return;
		}
		if( NodeUseCount == 0 )
		{
			Logger::Log( "SinglyList::ReleaseNode   error : NodeUseCount= 0" );
			return;
		}
		
		release_node->NodeNumber = 0;
		release_node->pData = NULL;

		if( FreeListCount > 0 )
			release_node->pNext = pFreeList;
		pFreeList = release_node;
		release_node = NULL;
		++FreeListCount;
		--NodeUseCount;
	}
	
	inline void  Link( SNode *link_node )
	{
		if( !link_node )
		{
			Logger::Log( "SinglyList::Link   error : link_node = NULL" );
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

	inline void  LinkToHead( SNode *link_node )
	{
		if( !link_node )
		{
			Logger::Log( "SinglyList::LinkToHead   error : link_node = NULL" );
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
	
	inline void Unlink( SNode *unlink_node )
	{
		if( !unlink_node )
		{
			Logger::Log( "SinglyList::Unlink   error : unlink_node = NULL" );
			return;
		}

		SNode	*node = pHeadNode;
		SNode	*prev_node = NULL;
		BOOL	get_flag = FALSE;

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
			Logger::Log( "SinglyList::Unlink   error : node get_flag = FALSE." );
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
			Logger::Log( "SinglyList::Unlink   error : NodeLinkCount= 0" );
	}
	
	inline SNode* SearchNodeWithNumber( UINT node_number )
	{
		SNode *node = pHeadNode;
		while( node )
		{
			if( node->NodeNumber == node_number )
				return node;
			node = node->pNext;
		}
		return NULL;
	}
};