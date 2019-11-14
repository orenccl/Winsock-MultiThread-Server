#pragma once

#include "List/TMPSinglyList.hpp"
#include "List/TMPDoublyList.hpp"
#include "Lock.h"

template < class DATA_TYPE >
class TMPSinglyLockList : public TMPSinglyList< DATA_TYPE >
{
public:
	CLock Locker;

	inline void Create(UINT object_max)
	{
		TMPSinglyList< DATA_TYPE >::Create(object_max);
		Locker.Create();
	}
	inline TSNode< DATA_TYPE >* LockGetNode()
	{
		Locker.Lock();
		TSNode< DATA_TYPE >* node = TMPSinglyList< DATA_TYPE >::GetNode();
		Locker.Unlock();
		return node;
	}
	inline void LockLink(TSNode< DATA_TYPE >* link_node)
	{
		Locker.Lock();
		TMPSinglyList< DATA_TYPE >::Link(link_node);
		Locker.Unlock();
	}
	inline void LockUnlink(TSNode< DATA_TYPE >* unlink_node)
	{
		Locker.Lock();
		TMPSinglyList< DATA_TYPE >::Unlink(unlink_node);
		Locker.Unlock();
	}
	inline void LockFreeNode(TSNode< DATA_TYPE >*& release_node)
	{
		Locker.Lock();
		TMPSinglyList< DATA_TYPE >::FreeNode(release_node);
		Locker.Unlock();
	}
	inline TSNode< DATA_TYPE >* LockGetHeadNode()
	{
		Locker.Lock();
		TSNode< DATA_TYPE >* node = TMPSinglyList< DATA_TYPE >::pHeadNode;
		Locker.Unlock();
		return node;
	}
	inline void LockUnlinkFreeNode(TSNode< DATA_TYPE >*& node)
	{
		Locker.Lock();
		TMPSinglyList< DATA_TYPE >::Unlink(node);
		TMPSinglyList< DATA_TYPE >::FreeNode(node);
		Locker.Unlock();
	}
};

template < class DATA_TYPE >
class TMPDoublyLockList : public TMPDoublyList< DATA_TYPE >
{
public:
	CLock Locker;

	inline void Create(UINT object_max)
	{
		TMPDoublyList< DATA_TYPE >::Create(object_max);
		Locker.Create();
	}
	inline TDNode< DATA_TYPE >* LockGetNode()
	{
		Locker.Lock();
		TDNode< DATA_TYPE >* node = TMPDoublyList< DATA_TYPE >::GetNode();
		Locker.Unlock();
		return node;
	}
	inline void LockLink(TDNode< DATA_TYPE >* link_node)
	{
		Locker.Lock();
		TMPDoublyList< DATA_TYPE >::Link(link_node);
		Locker.Unlock();
	}
	inline void LockUnlink(TDNode< DATA_TYPE >* unlink_node)
	{
		Locker.Lock();
		TMPDoublyList< DATA_TYPE >::Unlink(unlink_node);
		Locker.Unlock();
	}
	inline void LockFreeNode(TDNode< DATA_TYPE >*& release_node)
	{
		Locker.Lock();
		TMPDoublyList< DATA_TYPE >::FreeNode(release_node);
		Locker.Unlock();
	}
	inline TDNode< DATA_TYPE >* LockGetHeadNode()
	{
		Locker.Lock();
		TDNode< DATA_TYPE >* node = TMPDoublyList< DATA_TYPE >::pHeadNode;
		Locker.Unlock();
		return node;
	}
	inline void LockUnlinkFreeNode(TDNode< DATA_TYPE >*& node)
	{
		Locker.Lock();
		TMPSinglyList< DATA_TYPE >::Unlink(node);
		TMPSinglyList< DATA_TYPE >::FreeNode(node);
		Locker.Unlock();
	}
};