
#include <windows.h>
#include <stdio.h>
#include "MTLibrary/Thread.h"
#include "MTLibrary/Logger.h"
#include "MTLibrary/MemoryCounter.h"

int gCounter = 0;

class TestThread
{
public :
	int           Counter;
	volatile LONG EndCounter;

	#define  THREAD_POOL_MAX  1000
	CThread  ThreadPool[ THREAD_POOL_MAX ];

	static UINT WINAPI sTestThreadProc( LPVOID param );

	void Create()
	{
		Counter = 0;
		EndCounter = 0;

		for( int i = 0; i < THREAD_POOL_MAX; ++i )
			ThreadPool[ i ].Create( sTestThreadProc, this );
	}
	void Wait()
	{
		while( 1 )
		{
			if( EndCounter == THREAD_POOL_MAX )
				break;
			Sleep( 10 );
		}
	}
	void Release()
	{
		for( int i = 0; i < THREAD_POOL_MAX; ++i )
			ThreadPool[ i ].Release();
	}
};

UINT WINAPI TestThread::sTestThreadProc( LPVOID param )
{
	TestThread *thread = (TestThread*)param;

	Sleep( 1000 );
	++gCounter;
	++thread->Counter;
	InterlockedIncrement( &thread->EndCounter );
	printf( "sTestThreadProc   thread->Counter= %d \n", thread->Counter );
	Sleep( 1000 );

	return 0;
}

int main()
{
	Logger::Create();

	TestThread thread;
	thread.Create();
	thread.Wait();

	printf( "\n\n\n" );
	printf( "gCounter= %d \n", gCounter );
	printf( "thread.Counter= %d \n", thread.Counter );
	printf( "thread.EndCounter= %d \n", thread.EndCounter );
	printf( "\nEND." );

	thread.Release();
	MemoryCounter::sShow_MemoryUseCount();
	getc( stdin );
	return 0;
}