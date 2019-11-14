#pragma once
#include <windows.h>

class CThread
{
	UINT   ThreadID;
	HANDLE ThreadHandle;
public :
	CThread();

	void  Create( UINT WINAPI thread_work_func( LPVOID param ), void *param_data = NULL );
	void  Release();
	void  ReleaseWait();
};