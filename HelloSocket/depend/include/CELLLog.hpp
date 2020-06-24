#ifndef _CELLLOG_HPP_
#define _CELLLOG_HPP_


#include "Cell.hpp"
#include <ctime>
class CELLLog
{

#ifdef _DEBUG
#ifndef CELLLog_Debug
#define CELLLog_Debug(...) CELLLog::Debug(__VA_ARGS__)
#endif // !CELLLog_Debug
#else
#ifndef CELLLog_Debug
#define CELLLog_Debug(...)
#endif // !CELLLog_Debug
#endif // !_DEBUG

#define CELLLog_Info(...)    CELLLog::Info(__VA_ARGS__)
#define CELLLog_Warring(...) CELLLog::Warring(__VA_ARGS__)
#define CELLLog_Error(...)	 CELLLog::Error(__VA_ARGS__)
private:
	CELLLog()
	{
		_CellTask.Start();
	}
	~CELLLog()
	{
		_CellTask.Close();
		if (_file)
		{
			Info("CELLLog::CELLLog  _fclose");
			fclose(_file);
			_file = nullptr;
		}
	}
public:

	void setLogPath(const char* logName,const char* mode)
	{
		if (_file)
		{
			Info("CELLLog::setLogPath _file != nullptr");
			fclose(_file);
			_file = nullptr;
		}
		static char logPath[256] = {};

		auto t = system_clock::now();
		auto now = system_clock::to_time_t(t);
		std::tm *tNow = std::localtime(&now);
		sprintf(logPath,"%s[%d-%02d-%02d_%02d-%02d-%02d].txt",logName, tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday, tNow->tm_hour, tNow->tm_min, tNow->tm_sec);
		_file = fopen(logPath, mode);
		if (_file)
		{
			Info("CELLLog::setLogPath success<%s,%s>", logPath, mode);
		}
		else
		{
			Info("CELLLog::setLogPath failed<%s,%s>", logPath, mode);
		}
	}

	static CELLLog& Instance()
	{
		static CELLLog sLog;
		return sLog;
	}

	static void Info(const char* pLog)
	{
		Info("%s", pLog);
	}
	template<typename ...Args>
	static void Info(const char* pFormat, Args ...arg)
	{
		Echo("Info", pFormat, arg...);
	}

	static void Error(const char* pLog)
	{
		Error("%s", pLog);
	}
	template<typename ...Args>
	static void Error(const char* pFormat, Args ...arg)
	{
		Echo("Error", pFormat, arg...);
	}

	static void Warring(const char* pLog)
	{
		Warring("%s", pLog);
	}

	template<typename ...Args>
	static void Warring(const char* pFormat, Args ...arg)
	{
		Echo("Warring", pFormat, arg...);
	}

	static void Debug(const char* pLog)
	{
		Debug("%s", pLog);
	}
	template<typename ...Args>
	static void Debug(const char* pFormat, Args ...arg)
	{
		Echo("Debug", pFormat, arg...);
	}
	template<typename ...Args>
	static void Echo(const char* temp,const char* pFormat,Args ...arg)
	{	
		CELLLog *sLog = &Instance();
		sLog->_CellTask.addTask([=]() {
			if (sLog->_file)
			{
				auto t = system_clock::now();
				auto now = system_clock::to_time_t(t);
				std::tm *tNow = std::localtime(&now);
				fprintf(sLog->_file, "[%s][%04d-%02d-%02d %02d:%02d:%02d]", temp, tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday, tNow->tm_hour, tNow->tm_min, tNow->tm_sec);
				fprintf(sLog->_file, pFormat, arg...);
				fprintf(sLog->_file, "%s", "\n");
				fflush(sLog->_file);
			}
			printf("%s",temp);
			printf(pFormat, arg...);
			printf("\n");
		});
		
	}
public:
	FILE *_file = nullptr;
	CellTaskServer _CellTask;
};


#endif // !_CELLLOG_HPP_
