#pragma once
#include "mtpch.h"
#include "MTLibrary/EDComponentFramework.h"

class TestComponentA : public IComponent
{
public :
	virtual void vOnCreate() override
	{
		printf( "TestComponentA::vOnCreate\n" );
	}
	virtual void vOnUpdate() override
	{
		printf( "TestComponentA::vOnUpdate\n" );
	}
	virtual void vOnLateUpdate() override
	{
		printf( "TestComponentA::vOnLateUpdate\n" );
	}
	virtual void vOnShow() override
	{
		printf( "TestComponentA::vOnShow\n" );
	}
	virtual void vOnRelease() override
	{
		printf( "TestComponentA::vOnRelease\n" );
	}
};

class TestComponentB : public IComponent
{
public:
	virtual void vOnCreate() override
	{
		printf( "TestComponentB::vOnCreate\n" );
	}
	virtual void vOnUpdate() override
	{
		printf( "TestComponentB::vOnUpdate\n" );
	}
	virtual void vOnLateUpdate() override
	{
		printf( "TestComponentB::vOnLateUpdate\n" );
	}
	virtual void vOnShow() override
	{
		printf( "TestComponentB::vOnShow\n" );
	}
	virtual void vOnRelease() override
	{
		printf( "TestComponentB::vOnRelease\n" );
	}
};

class TestComponentC : public IComponent
{
public:
	virtual void vOnCreate() override
	{
		printf( "TestComponentC::vOnCreate\n" );
	}
	virtual void vOnUpdate() override
	{
		printf( "TestComponentC::vOnUpdate\n" );
	}
	virtual void vOnLateUpdate() override
	{
		printf( "TestComponentC::vOnLateUpdate\n" );
		
	}
	virtual void vOnShow() override
	{
		printf( "TestComponentC::vOnShow\n" );
	}
	virtual void vOnRelease() override
	{
		printf( "TestComponentC::vOnRelease\n" );
	}
	virtual void vOnMessage( UINT message_type, void *data ) override
	{
		IComponent *node = (IComponent*)data;
		printf( "TestComponentC::vOnMessage   my_ID= %d, message_type= %d, source_node->ID= %d \n", ID, message_type, node->ID );
	}
};

class TestComponentD : public IComponent
{
public:
	virtual void vOnCreate() override
	{
		printf( "TestComponentD::vOnCreate\n" );
	}
	virtual void vOnUpdate() override
	{
		printf( "TestComponentD::vOnUpdate\n" );
	}
	virtual void vOnLateUpdate() override
	{
		printf( "TestComponentD::vOnLateUpdate\n" );
		SendMessage( 3, 100, this ); // Send message to Component C's vOnMessage function.
	}
	virtual void vOnShow() override
	{
		printf( "TestComponentD::vOnShow\n" );
	}
	virtual void vOnRelease() override
	{
		printf( "TestComponentD::vOnRelease\n" );
	}
};

class GameFramework : public EDComponentFramework
{
public :
	//------------------------------------------------------------
	void Create()
	{
		TestComponentA *component_a = NULL;
		MY_NEW( component_a, TestComponentA() );
		AddComponent( component_a, 1 );

		TestComponentB *component_b = NULL;
		MY_NEW( component_b, TestComponentB() );
		AddComponent( component_b, 2 );

		TestComponentC *component_c = NULL;
		MY_NEW( component_c, TestComponentC() );
		component_b->AddComponent( component_c, 3 );

		TestComponentD *component_d = NULL;
		MY_NEW( component_d, TestComponentD() );
		component_c->AddComponent( component_d, 4 );
	}

	static void sRecursiveRelease( DNode *node )
	{
		IComponent *component = NULL;
		while( node )
		{
			component = (IComponent*)node->pData;
			if( component->ComponentList.pHeadNode )
				sRecursiveRelease( component->ComponentList.pHeadNode );
			MY_DELETE( component );
			node = node->pNext;
		}
	}
	void Release()
	{
		sRecursiveRelease( ComponentList.pHeadNode );
	}
	
	virtual void vRun()
	{
		Create();
		EDComponentFramework::vRun();
		Release();
	}
};