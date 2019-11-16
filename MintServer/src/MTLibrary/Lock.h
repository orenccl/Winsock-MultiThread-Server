#pragma once

class CLock
{
	CRITICAL_SECTION LockInfo;
public :
	CLock();

	void  Create();
	void  Release();
	void  Lock();
	void  Unlock();
	BOOL  IsLock();
};