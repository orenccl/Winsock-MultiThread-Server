#include "mtpch.h"
#include "MTLibrary/Thread.h"
#include "MTLibrary/MemoryCounter.h"
#include <process.h> // _beginthread   _beginthreadex

CThread::CThread()
{
	ThreadID = 0;
	ThreadHandle = NULL;
}

void CThread::Create( UINT WINAPI thread_work_func( LPVOID param ), void *param_data )
{
	WaitEvent::Create();

	/*!
	* @details
	* Initialize a thread, and execute immediately.
	* @param1 Secure Level, no need for normal situation
	* @param2 stack size of thread, 0 = default = 1MB
	* @param3 State : False = Initial state
	* @param4 Pass user data to thread_work_func
	* @param5 state after initial : 0 = execute immediately, CREATE_SUSPEND = sleep
	* @param6 Get thread ID
	*/

	ThreadHandle = (HANDLE)::_beginthreadex( NULL, 0, thread_work_func, param_data, 0, &ThreadID );
	MemoryCounter::sAdd_MemoryUseCount();
}

void CThread::Release()
{
	if( ThreadHandle )
	{
		::CloseHandle( ThreadHandle ); // Close one thread and release it's memory
		ThreadHandle = NULL;
		MemoryCounter::sDel_MemoryUseCount();
	}
	WaitEvent::Release();
}

void CThread::ReleaseWait()
{
	while( ThreadHandle )
	{
		::Sleep( 100 ); // sleep 100ms 
	}
}

CThreadEx::CThreadEx()
{
	SwitchFlag = false;
}

void CThreadEx::SetSwitchFlag(bool flag)
{
	::InterlockedExchange(&SwitchFlag, flag);
}