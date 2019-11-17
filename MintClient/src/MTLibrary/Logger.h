#pragma once

/*!
* @Details
* Logger only use static function because we don't need it to be an Object.
*/
class Logger
{
public:
	static void Create();
	static void Log(const char* format, ...);

private:
	static char m_LogFileFolder[LENGTH_128];
	static char m_LogFileName[LENGTH_128];
};