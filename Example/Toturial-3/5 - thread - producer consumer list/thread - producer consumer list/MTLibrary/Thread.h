#pragma once
#include "WaitEvent.h"

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

class CThreadEx : public CThread
{
	volatile LONG SwitchFlag;
public:
	CThreadEx();

	void SetSwitchFlag(bool flag);
	inline LONG GetSwitchFlag() { return SwitchFlag; }
};