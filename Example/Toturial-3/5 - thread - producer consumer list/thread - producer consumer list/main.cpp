
#include <windows.h>
#include <stdio.h>
#include "MTLibrary/Thread.h"
#include "MTLibrary/Lock.h"
#include "MTLibrary/Logger.h"
#include "MTLibrary/MemoryCounter.h"
#include "MTLibrary/LockList.hpp"

struct TaskInfo
{
	UINT Counter;
};

class TestThread
{
public:
	TMPSinglyLockList< TaskInfo > TaskList;
	bool RunFlag;
	UINT TaskCounter;
	volatile LONG EndCounter;

	CThread       ProducerThread;
	CThread       ConsumerThread;

	static UINT WINAPI sProducer_ThreadProc(LPVOID param);
	static UINT WINAPI sConsumer_ThreadProc(LPVOID param);

	void Create()
	{
		TaskList.Create(2000);
		RunFlag = true;
		TaskCounter = 0;
		EndCounter = 0;

		ProducerThread.Create(sProducer_ThreadProc, this);
		ConsumerThread.Create(sConsumer_ThreadProc, this);
		ProducerThread.Start();
	}
	
	void Producer_InThread()
	{
		TSNode< TaskInfo >* task_node = NULL;

		while (RunFlag)
		{
			ProducerThread.Wait();
			while (RunFlag)
			{
				if(TaskCounter < 2000)
				{ 
					task_node = TaskList.LockGetNode();
					task_node->Counter = ++TaskCounter;
					TaskList.LockLink(task_node);
					printf("Producer_InThread   Produce, TaskCounter= %d\n", TaskCounter);
				}
				ConsumerThread.Start();
				break;
			}
		}
		ProducerThread.Release();

		InterlockedIncrement(&EndCounter);
		printf("Producer_InThread   EndCounter= %d \n", EndCounter);
	}

	void Consumer_InThread()
	{
		TSNode< TaskInfo >* task_node = NULL;

		while (RunFlag)
		{
			ConsumerThread.Wait();
			while (task_node = TaskList.LockGetHeadNode())
			{
				printf("Consumer_InThread   Consume, task_node->Counter= %d\n", task_node->Counter);
				Sleep(10);
				TaskList.LockUnlinkFreeNode(task_node);
			}
			if (TaskCounter == 2000)
				RunFlag = false;
			ProducerThread.Start();
		}
		ConsumerThread.Release();

		InterlockedIncrement(&EndCounter);
		printf("Consumer_InThread   EndCounter= %d \n", EndCounter);
	}

	void Wait()
	{
		while (1)
		{
			if (EndCounter == 2)
				break;
			Sleep(10);
		}
	}

	void Release()
	{
		ProducerThread.Release();
		ConsumerThread.Release();
	}
};

UINT WINAPI TestThread::sProducer_ThreadProc(LPVOID param)
{
	TestThread* thread = (TestThread*)param;
	thread->Producer_InThread();
	return 0;
}

UINT WINAPI TestThread::sConsumer_ThreadProc(LPVOID param)
{
	TestThread* thread = (TestThread*)param;
	thread->Consumer_InThread();
	return 0;
}

int main()
{
	Logger::Create();
	const UINT memory_pool_bytes_max = 100 * MB;
	MainMemoryPool::sMemoryPoolCreate(memory_pool_bytes_max);

	TestThread thread;
	thread.Create();
	thread.Wait();

	printf("\n\n\n");
	printf("thread.TaskCounter= %d \n", thread.TaskCounter);
	printf("thread.EndCounter= %d \n", thread.EndCounter);
	printf("\nEND.");

	thread.Release();
	MainMemoryPool::sMemoryPoolRelease();
	MemoryCounter::sShow_MemoryUseCount();
	getc(stdin);
	return 0;
}