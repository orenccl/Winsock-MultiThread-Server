#include "mtpch.h"
#include "EDComponentFramework.h"

EDComponentFramework::EDComponentFramework()
{
	UpdateFlag = TRUE;
	ShowFlag = TRUE;
	MouseStateType = MOUSE_STATE_TYPE::NONE;
	IsRunFlag = TRUE;
}

EDComponentFramework::~EDComponentFramework()
{

}

void EDComponentFramework::vOnCreate()
{
	IComponent::vOnCreate();
}

void EDComponentFramework::vOnRelease()
{
	IComponent::vOnRelease();
}

void EDComponentFramework::vOnUpdate()
{
	IComponent::vOnUpdate();
}

void EDComponentFramework::vOnLateUpdate()
{
	IComponent::vOnLateUpdate();
}

void EDComponentFramework::vOnShow()
{
	IComponent::vOnShow();
}

void EDComponentFramework::vOnMouseLeftDown()
{
	IControlComponent::vOnMouseLeftDown();
}

void EDComponentFramework::vOnMouseRightDown()
{
	IControlComponent::vOnMouseRightDown();
}

void EDComponentFramework::vOnMouseLeftDownDouble()
{
	IControlComponent::vOnMouseLeftDownDouble();
}

void EDComponentFramework::vOnMouseLeftUp()
{
	IControlComponent::vOnMouseLeftUp();
}

void EDComponentFramework::vOnMouseRightUp()
{
	IControlComponent::vOnMouseRightUp();
}

void EDComponentFramework::vOnKeyboardDown( long wparam, long lparam )
{
	IControlComponent::vOnKeyboardDown( wparam, lparam );
}

void EDComponentFramework::vOnKeyboardUp( long wparam, long lparam )
{
	IControlComponent::vOnKeyboardUp( wparam, lparam );
}

void EDComponentFramework::vRun()
{
	vOnCreate();

//	while( IsRunFlag )
	{
		MainEventRun();
	}

	vOnRelease();
}

void EDComponentFramework::MainEventRun()
{
//	BackgroundClear();

	if( UpdateFlag )
	{
		vOnUpdate();

		switch ( MouseStateType )
		{
			case MOUSE_STATE_TYPE::LEFT_DOWN :
			{
				vOnMouseLeftDown();
				break;
			}
			case MOUSE_STATE_TYPE::RIGHT_DOWN :
			{
				vOnMouseRightDown();
				break;
			}
			case MOUSE_STATE_TYPE::LEFT_DOWN_DOUBLE :
			{
				vOnMouseLeftDownDouble();
				break;
			}
			case MOUSE_STATE_TYPE::LEFT_UP :
			{
				vOnMouseLeftUp();
				break;
			}
			case MOUSE_STATE_TYPE::RIGHT_UP :
			{
				vOnMouseRightUp();
				break;
			}
		};

		vOnLateUpdate();
	}

	if( ShowFlag )
	{
		vOnShow();

//		ImageRender();
	}
}