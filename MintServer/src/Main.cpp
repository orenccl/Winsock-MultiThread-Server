#include "mtpch.h"
#include "MTLibrary/Type.h"
#include "MTLibrary/Logger.h"
#include "MTLibrary/List/SinglyList.hpp"
#include "MTLibrary/List/DoublyList.hpp"
#include "MTLibrary/List/TSinglyList.hpp"
#include "MTLibrary/List/TDoublyList.hpp"
#include "GameFramework.hpp"


struct TestInfo
{
	long a;
	long b;
	long c;
};
TestInfo info1 = { 0 };
TestInfo info2 = { 0 };
TestInfo info3 = { 0 };

void Test_SinglyList()
{
	SinglyList list;
	list.Create();

	SNode* node1 = list.GetNode(&info1);
	SNode* node2 = list.GetNode(&info2);
	SNode* node3 = list.GetNode(&info3);
	list.Link(node1);
	list.Link(node2);
	list.Link(node3);
	list.Unlink(node2);
	list.Unlink(node1);
	list.Unlink(node3);

	list.FreeNode(node1);
	list.FreeNode(node2);
	list.FreeNode(node3);

	list.Release();
}

void Test_DoublyList()
{
	DoublyList list;
	list.Create();

	DNode* node1 = list.GetNode(&info1);
	DNode* node2 = list.GetNode(&info2);
	DNode* node3 = list.GetNode(&info3);
	list.Link(node1);
	list.Link(node2);
	list.Link(node3);
	list.Unlink(node3);
	list.Unlink(node2);
	list.Unlink(node1);

	list.FreeNode(node1);
	list.FreeNode(node2);
	list.FreeNode(node3);

	list.Release();
}

void Test_TSinglyList()
{
	TSinglyList< TestInfo > list;
	list.Create();

	TSNode< TestInfo >* node1 = list.GetNode();
	TSNode< TestInfo >* node2 = list.GetNode();
	TSNode< TestInfo >* node3 = list.GetNode();
	list.Link(node1);
	list.Link(node2);
	list.Link(node3);
	list.Unlink(node2);
	list.Unlink(node1);
	list.Unlink(node3);

	list.FreeNode(node1);
	list.FreeNode(node2);
	list.FreeNode(node3);

	node1 = list.GetNode();
	node2 = list.GetNode();
	node3 = list.GetNode();
	list.Link(node1);
	list.Link(node3);
	list.Link(node2);
	list.Unlink(node3);
	list.Unlink(node1);
	list.Unlink(node2);

	list.FreeNode(node2);
	list.FreeNode(node1);
	list.ReleaseNode(node3);

	list.Release();
}

void Test_TDoublyList()
{
	TDoublyList< TestInfo > dblist;
	dblist.Create();

	TDNode< TestInfo >* node1 = dblist.GetNode();
	TDNode< TestInfo >* node2 = dblist.GetNode();
	TDNode< TestInfo >* node3 = dblist.GetNode();
	dblist.Link(node1);
	dblist.Link(node3);
	dblist.Link(node2);
	dblist.Unlink(node1);
	dblist.Unlink(node3);
	dblist.Unlink(node2);

	dblist.FreeNode(node2);
	dblist.ReleaseNode(node1);
	dblist.ReleaseNode(node3);
	dblist.Release();
}


void Test_GameFramework()
{
	GameFramework framework;
	framework.vRun();
}

int main()
{
	Logger::Create();
	Logger::Log("main");

	Test_SinglyList();
	Test_DoublyList();

	Test_TSinglyList();
	Test_TDoublyList();

	Test_GameFramework();

	MemoryCounter::sShow_MemoryUseCount();
	getc(stdin);
	return 0;
}