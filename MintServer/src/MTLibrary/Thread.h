#pragma once
#include "MTLibrary/WaitEvent.h"

class CThread : public WaitEvent
{
	UINT   ThreadID;
	HANDLE ThreadHandle;
public :
	CThread();

	void  Create( UINT WINAPI thread_work_func( LPVOID param ), void *param_data = NULL );
	void  Release();
	void  ReleaseWait();
};

class CAcceptThread : public CThread
{
	volatile LONG SelectTimeModeFlag;
public:
	CAcceptThread();

	void SetSelectTimeModeFlag(bool flag);
	inline bool GetSelectTimeModeFlag() { return SelectTimeModeFlag; }
};