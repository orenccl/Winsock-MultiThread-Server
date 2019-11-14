#include "WaitEvent.h"
#include "MemoryCounter.h"

WaitEvent::WaitEvent()
{
	WaitHandle = NULL;
}

void WaitEvent::Create()
{
	/*!
	* @details
	* Initialize a event, use to block thread on WaitForSingleObject function.
	* Wait until SetEvent is called.
	* @param1 Secure Level, no need for normal situation
	* @param2 Treu = stop manual, False = stop automatically
	* @param3 State : False = Initial state
	* @param4 Event Name
	*/
	WaitHandle = ::CreateEvent( NULL, TRUE, FALSE, NULL );
	MemoryCounter::sAdd_MemoryUseCount();
}

void WaitEvent::Release()
{
	if( WaitHandle )
	{
		::CloseHandle( WaitHandle ); // Release Event
		WaitHandle = NULL;
		MemoryCounter::sDel_MemoryUseCount();
	}
}

void WaitEvent::Wait()
{
	::WaitForSingleObject(WaitHandle, INFINITE); // Set event into blosk state! INFINITE waiting time.
	::ResetEvent(WaitHandle); // end event - Clear last event signal
}

void WaitEvent::Start()
{
	::SetEvent( WaitHandle ); // start event - SerEvent signal! object can jump out of block state.
}
