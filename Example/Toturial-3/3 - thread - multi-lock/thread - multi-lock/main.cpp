
#include <windows.h>
#include <stdio.h>
#include "MTLibrary/Thread.h"
#include "MTLibrary/Lock.h"
#include "MTLibrary/Logger.h"
#include "MTLibrary/MemoryCounter.h"


class PlayerInfo
{
public:
	int    Value;
	CLock  Locker;

	void Create(int v)
	{
		Value = v;
		Locker.Create();
	}
	void Release()
	{
		Locker.Release();
	}
};

class TestThread
{
public:
#define       PLAYER_MAX  2
	PlayerInfo    PlayerTable[PLAYER_MAX];
	volatile LONG EndCounter;

#define       THREAD_POOL_MAX  1000 // Thread should not set too large, it's only for test.
	CThread       ThreadPool[THREAD_POOL_MAX];
	CLock         BigLocker;

	static UINT WINAPI sTest_A_ThreadProc(LPVOID param);
	static UINT WINAPI sTest_B_ThreadProc(LPVOID param);
	static UINT WINAPI sTest_C_ThreadProc(LPVOID param);
	static UINT WINAPI sTest_D_ThreadProc(LPVOID param);

	void Create()
	{
		EndCounter = 0;

		BigLocker.Create();

		for (int i = 0; i < PLAYER_MAX; ++i)
			PlayerTable[i].Create(i + 1);

#if 1
		for (int i = 0; i < THREAD_POOL_MAX; i += 2)
		{
			ThreadPool[i].Create(sTest_A_ThreadProc, this);
			ThreadPool[i + 1].Create(sTest_B_ThreadProc, this);
		}
#else
		for (int i = 0; i < THREAD_POOL_MAX; ++i)
		{
			ThreadPool[i].Create(sTest_C_ThreadProc, this);
			Sleep( 10 ); // Reduce down CPU loading
		}
#endif
	}
	PlayerInfo* GetPlayer(UINT index)
	{
		if (index < PLAYER_MAX)
			return &PlayerTable[index];
		return NULL;
	}

	void SwapValue_Safe_Slow_1(PlayerInfo* player_a, PlayerInfo* player_b)
	{
		BigLocker.Lock();   // BigLocker, Low effiency

		int a_value = player_a->Value;
		player_a->Value = player_b->Value;
		player_b->Value = a_value;
		Sleep(10);

		BigLocker.Unlock();
	}

	void SwapValue_Safe_Slow_2(PlayerInfo* player_a, PlayerInfo* player_b)
	{
		BigLocker.Lock();   // BigLocker, Low effiency

		player_a->Locker.Lock();
		player_b->Locker.Lock();

		int a_value = player_a->Value;
		player_a->Value = player_b->Value;
		player_b->Value = a_value;
		Sleep(10);

		player_b->Locker.Unlock();
		player_a->Locker.Unlock();

		BigLocker.Unlock();
	}

	void SwapValue_Safe_Fast(PlayerInfo* player_a, PlayerInfo* player_b)
	{
		// Lock a variable at a time, normal efficiency
		// Maybe cause some problem after seprate execution.

		Sleep(10);

		player_a->Locker.Lock();
		int backup_a_value = player_a->Value;
		player_a->Locker.Unlock();

		player_b->Locker.Lock();
		int backup_b_value = player_b->Value;
		player_b->Value = backup_a_value;
		player_b->Locker.Unlock();

		player_a->Locker.Lock();
		player_a->Value = backup_b_value;
		player_a->Locker.Unlock();
	}
	
	void SwapValue_DeadLock_1(PlayerInfo* player_a, PlayerInfo* player_b)
	{
		// If multi-thread execute SwapValue_DeadLock_1, SwapValue_DeadLock_2 at same time
		// May cause dead lock

		player_a->Locker.Lock();
		player_b->Locker.Lock();

		int a_value = player_a->Value;
		player_a->Value = player_b->Value;
		player_b->Value = a_value;
		Sleep(10);

		player_b->Locker.Unlock();
		player_a->Locker.Unlock();
	}

	void SwapValue_DeadLock_2(PlayerInfo* player_a, PlayerInfo* player_b)
	{
		// If multi-thread execute SwapValue_DeadLock_1, SwapValue_DeadLock_2 at same time
		// May cause dead lock

		player_b->Locker.Lock();
		player_a->Locker.Lock();

		int a_value = player_a->Value;
		player_a->Value = player_b->Value;
		player_b->Value = a_value;
		Sleep(10);

		player_a->Locker.Unlock();
		player_b->Locker.Unlock();
	}

	void TestSwap_A_InThread()
	{
		PlayerInfo* player_a = GetPlayer(0);
		PlayerInfo* player_b = GetPlayer(1);
		SwapValue_Safe_Slow_1(player_a, player_b);

		InterlockedIncrement(&EndCounter); // This function ensure thread secure. Don't need lock.
		printf("TestSwap_A_InThread   EndCounter= %d \n", EndCounter);
	}
	//------------------------------------------------------------
	void TestSwap_B_InThread()
	{
		PlayerInfo* player_a = GetPlayer(0);
		PlayerInfo* player_b = GetPlayer(1);
		SwapValue_Safe_Slow_1(player_b, player_a);

		InterlockedIncrement(&EndCounter); // This function ensure thread secure.
		printf("TestSwap_B_InThread   EndCounter= %d \n", EndCounter);
	}

	void TestSwap_C_InThread()
	{
		PlayerInfo* player_a = NULL;
		PlayerInfo* player_b = NULL;

		static int c = 0;
		if (++c % 2)
		{
			player_a = GetPlayer(0);
			player_b = GetPlayer(1);
			SwapValue_Safe_Slow_2(player_a, player_b);
		}
		else
		{
			player_a = GetPlayer(1);
			player_b = GetPlayer(0);
			SwapValue_Safe_Slow_2(player_a, player_b);
		}

		InterlockedIncrement(&EndCounter); // This function ensure thread secure.
		printf("TestSwap_C_InThread   EndCounter= %d \n", EndCounter);
	}

	void TestSwap_D_InThread()
	{
		PlayerInfo* player_a = NULL;
		PlayerInfo* player_b = NULL;

		for (int i = 1; i <= 100; ++i)
		{
			static int c = 0;
			if (++c % 2)
			{
				player_a = GetPlayer(0);
				player_b = GetPlayer(1);
				SwapValue_Safe_Fast(player_a, player_b);
			}
			else
			{
				player_a = GetPlayer(1);
				player_b = GetPlayer(0);
				SwapValue_Safe_Fast(player_a, player_b);
			}
			printf("TestSwap_D_InThread   i= %d \n", i);
		}

		InterlockedIncrement(&EndCounter); // This function ensure thread secure.
		printf("TestSwap_D_InThread   EndCounter= %d \n", EndCounter);
	}

	void Wait()
	{
		while (1)
		{
			if (EndCounter == THREAD_POOL_MAX)
				break;
			Sleep(10);
		}
	}
	void Release()
	{
		BigLocker.Release();

		for (int i = 0; i < PLAYER_MAX; ++i)
			PlayerTable[i].Release();

		for (int i = 0; i < THREAD_POOL_MAX; ++i)
			ThreadPool[i].Release();
	}
};

UINT WINAPI TestThread::sTest_A_ThreadProc(LPVOID param)
{
	TestThread* thread = (TestThread*)param;
	thread->TestSwap_A_InThread();
	return 0;
}

UINT WINAPI TestThread::sTest_B_ThreadProc(LPVOID param)
{
	TestThread* thread = (TestThread*)param;
	thread->TestSwap_B_InThread();
	return 0;
}

UINT WINAPI TestThread::sTest_C_ThreadProc(LPVOID param)
{
	TestThread* thread = (TestThread*)param;
	thread->TestSwap_C_InThread();
	return 0;
}

UINT WINAPI TestThread::sTest_D_ThreadProc(LPVOID param)
{
	TestThread* thread = (TestThread*)param;
	thread->TestSwap_D_InThread();
	return 0;
}


int main()
{
	Logger::Create();

	TestThread thread;
	thread.Create();
	thread.Wait();

	printf("\n\n\n");
	printf("thread.EndCounter= %d \n", thread.EndCounter);
	printf("\nEND.");

	thread.Release();
	MemoryCounter::sShow_MemoryUseCount();
	getc(stdin);
	return 0;
}