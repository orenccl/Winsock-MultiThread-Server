#pragma once
#include "List/DoublyList.hpp"

class IComponent
{
public :
	UINT        Type;
	UINT        ID;
	DoublyList  ComponentList;

	static IComponent *spRootComponent;

	IComponent();
	virtual ~IComponent();

	void AddComponent( IComponent *component, UINT id );
	void DelComponent( IComponent *component );

	IComponent* SearchComponent( UINT search_id );

	bool SendMessage( UINT dest_id, UINT message_type, void *data );

	virtual void vOnCreate();
	virtual void vOnRelease();

	virtual void vOnUpdate();
	virtual void vOnLateUpdate();

	virtual void vOnShow();

	virtual void vOnMessage( UINT message_type, void *data ){}
};

class IControlComponent : public IComponent
{
public :
	virtual void vOnMouseLeftDown();
	virtual void vOnMouseRightDown();
	virtual void vOnMouseLeftDownDouble();
	virtual void vOnMouseLeftUp();
	virtual void vOnMouseRightUp();

	virtual void vOnKeyboardDown( long wparam, long lparam );
	virtual void vOnKeyboardUp( long wparam, long lparam );
};
