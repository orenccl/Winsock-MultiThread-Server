#include "mtpch.h"
#include "Logger.h"
#include <filesystem>

char Logger::m_LogFileFolder[LENGTH_128] = "MintLog";
char Logger::m_LogFileName[LENGTH_128] = "Log";

void Logger::Create()
{
	std::filesystem::create_directories(m_LogFileFolder);
}

void Logger::Log(const char* format, ...)
{
	va_list	args;
	char	buff_format[LENGTH_1024];

	// Pass args into buffer format
	va_start(args, format);
	vsnprintf(buff_format, LENGTH_1024 - 1, format, args);
	va_end(args);

	SYSTEMTIME   time;
	GetLocalTime(&time);

	char buff_year_month_day[LENGTH_32];
	snprintf(buff_year_month_day, LENGTH_32 - 1, "%04d%02d%02d", time.wYear, time.wMonth, time.wDay);

	char buff_dateFloder[MAX_PATH];
	snprintf(buff_dateFloder, MAX_PATH - 1, "%s\\%s", m_LogFileFolder, buff_year_month_day);
	std::filesystem::create_directories(buff_dateFloder);

	char buff_fullPath[MAX_PATH];
	snprintf(buff_fullPath, MAX_PATH - 1, "%s\\%s\\%s-%s.log", m_LogFileFolder, buff_year_month_day, m_LogFileName, buff_year_month_day);

	FILE* fp = nullptr;
	errno_t err = fopen_s(&fp, buff_fullPath, "a");
	if (!err && fp != nullptr)
	{
		fprintf(fp, "%02d:%02d:%02d  %s\r\n", time.wHour, time.wMinute, time.wSecond, buff_format);
		fclose(fp);
	}

	printf("\n%02d:%02d:%02d  %s\n", time.wHour, time.wMinute, time.wSecond, buff_format);
}