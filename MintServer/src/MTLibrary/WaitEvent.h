#pragma once

class WaitEvent
{
	HANDLE WaitHandle;
public :
	WaitEvent();

	void Create();
	void Release();
	void Wait();
	void Start();
};