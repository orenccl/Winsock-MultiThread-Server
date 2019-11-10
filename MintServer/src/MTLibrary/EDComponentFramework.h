#pragma once
#include "IComponent.h"

enum class MOUSE_STATE_TYPE
{
	NONE,
	LEFT_DOWN,
	RIGHT_DOWN,
	LEFT_DOWN_DOUBLE,
	LEFT_UP,
	RIGHT_UP,
};

class EDComponentFramework : public IControlComponent // event-driven component-baesd framework
{
protected :
	BOOL				UpdateFlag;
	BOOL				ShowFlag;
	MOUSE_STATE_TYPE	MouseStateType;
	BOOL				IsRunFlag;

	void MainEventRun();

	EDComponentFramework();
	~EDComponentFramework();

	virtual void vOnCreate();
	virtual void vOnRelease();

	virtual void vOnUpdate();
	virtual void vOnLateUpdate();

	virtual void vOnShow();

	virtual void vOnMouseLeftDown();
	virtual void vOnMouseLeftDownDouble();
	virtual void vOnMouseRightDown();
	virtual void vOnMouseLeftUp();
	virtual void vOnMouseRightUp();

	virtual void vOnKeyboardDown( long wparam, long lparam );
	virtual void vOnKeyboardUp( long wparam, long lparam );

public :
	virtual void vRun();
};