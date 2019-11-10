#include "mtpch.h"
#include "IComponent.h"

IComponent *IComponent::spRootComponent = NULL;

IComponent::IComponent()
{
	if( IComponent::spRootComponent == NULL )
		IComponent::spRootComponent = this;

	Type = 0;
	ID = 0;
	ComponentList.Create();
}

IComponent::~IComponent()
{
	ComponentList.Release();
}

void IComponent::AddComponent( IComponent *component, UINT id )
{
	DNode *node = ComponentList.GetNode( component );
	node->NodeNumber = component->ID = id;
	ComponentList.Link( node );
}

void IComponent::DelComponent( IComponent *component )
{
	DNode *node = ComponentList.SearchNodeWithNumber( component->ID );
	if( node )
		ComponentList.Unlink( node );
}

DNode* gRecursiveSearchNodeWithID( DoublyList *list, UINT id )
{
	IComponent *component = NULL;
	DNode *node = list->pHeadNode, *result_node = NULL;
	while( node )
	{
		component = (IComponent*)node->pData;
		if( component->ID == id )
			return node;
		result_node = gRecursiveSearchNodeWithID( &component->ComponentList, id );
		if( result_node )
			return result_node;
		node = node->pNext;
	}
	return NULL;
}

IComponent* IComponent::SearchComponent( UINT search_id )
{
	DNode *node = gRecursiveSearchNodeWithID( &IComponent::spRootComponent->ComponentList, search_id );
	if( node )
		return (IComponent*)node->pData;
	return NULL;
}

bool IComponent::SendMessage( UINT dest_id, UINT message_type, void *data )
{
	DNode *node = gRecursiveSearchNodeWithID( &IComponent::spRootComponent->ComponentList, dest_id );
	if( node )
	{
		((IComponent*)node->pData)->vOnMessage( message_type, data );
		return true;
	}
	return false;
}

void gRecursiveCreate( DNode *node )
{
	IComponent *component = NULL;
	while( node )
	{
		component = (IComponent*)node->pData;
		component->vOnCreate();
		if( component->ComponentList.pHeadNode )
			gRecursiveCreate( component->ComponentList.pHeadNode );
		node = node->pNext;
	}
}

void IComponent::vOnCreate()
{
	gRecursiveCreate( ComponentList.pHeadNode );
}

void gRecursiveRelease( DNode *node )
{
	IComponent *component = NULL;
	while( node )
	{
		component = (IComponent*)node->pData;
		component->vOnRelease();
		if( component->ComponentList.pHeadNode )
			gRecursiveRelease( component->ComponentList.pHeadNode );
		node = node->pNext;
	}
}

void IComponent::vOnRelease()
{
	gRecursiveRelease( ComponentList.pHeadNode );
}

void gRecursiveUpdate( DNode *node )
{
	IComponent *component = NULL;
	while( node )
	{
		component = (IComponent*)node->pData;
		component->vOnUpdate();
		if( component->ComponentList.pHeadNode )
			gRecursiveUpdate( component->ComponentList.pHeadNode );
		node = node->pNext;
	}
}

void IComponent::vOnUpdate()
{
	gRecursiveUpdate( ComponentList.pHeadNode );
}

void gRecursiveLateUpdate( DNode *node )
{
	IComponent *component = NULL;
	while( node )
	{
		component = (IComponent*)node->pData;
		component->vOnLateUpdate();
		if( component->ComponentList.pHeadNode )
			gRecursiveLateUpdate( component->ComponentList.pHeadNode );
		node = node->pNext;
	}
}

void IComponent::vOnLateUpdate()
{
	gRecursiveLateUpdate( ComponentList.pHeadNode );
}

void gRecursiveShow( DNode *node )
{
	IComponent *component = NULL;
	while( node )
	{
		component = (IComponent*)node->pData;
		component->vOnShow();
		if( component->ComponentList.pHeadNode )
			gRecursiveShow( component->ComponentList.pHeadNode );
		node = node->pNext;
	}
}

void IComponent::vOnShow()
{
	gRecursiveShow( ComponentList.pHeadNode );
}

void IControlComponent::vOnMouseLeftDown()
{
}

void IControlComponent::vOnMouseRightDown()
{
}

void IControlComponent::vOnMouseLeftDownDouble()
{
}

void IControlComponent::vOnMouseLeftUp()
{
}

void IControlComponent::vOnMouseRightUp()
{
}

void IControlComponent::vOnKeyboardDown( long wparam, long lparam )
{
}

void IControlComponent::vOnKeyboardUp( long wparam, long lparam )
{
}