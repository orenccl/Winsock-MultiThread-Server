#include "mtpch.h"
#include "MTLibrary/Lock.h"

CLock::CLock()
{
	::memset( &LockInfo, 0, sizeof( CRITICAL_SECTION ) );
}

void CLock::Create()
{
	::InitializeCriticalSection( &LockInfo );
}

void CLock::Release()
{
	::DeleteCriticalSection( &LockInfo );
}

void CLock::Lock()
{
	::EnterCriticalSection( &LockInfo );
}

void CLock::Unlock()
{
	::LeaveCriticalSection( &LockInfo );
}

BOOL CLock::IsLock()
{
	return LockInfo.RecursionCount;
}