#ifndef _CELLLOG_HPP_
#define _CELLLOG_HPP_


#include "Cell.hpp"
#include <ctime>
class CELLLog
{
public:
	CELLLog()
	{
		_CellTask.Start();
	}
	~CELLLog()
	{
		_CellTask.Close();
		if (_file)
		{
			Info("CELLLog::CELLLog  _fclose\n");
			fclose(_file);
			_file = nullptr;
		}
	}
public:

	void setLogPath(const char* logPath,const char* mode)
	{
		if (_file)
		{
			Info("CELLLog::setLogPath _file != nullptr\n");
			fclose(_file);
			_file = nullptr;
		}
		_file = fopen(logPath, mode);
		if (_file)
		{
			Info("CELLLog::setLogPath success<%s,%s>\n", logPath, mode);
		}
		else
		{
			Info("CELLLog::setLogPath failed<%s,%s>\n", logPath, mode);
		}
	}

	static CELLLog& Instance()
	{
		static CELLLog sLog;
		return sLog;
	}

	static void Info(const char* pLog)
	{
		CELLLog *sLog = &Instance();
		sLog->_CellTask.addTask([=]() {
			if (sLog->_file)
			{
				auto t = system_clock::now();
				auto now = system_clock::to_time_t(t);
				std::tm *tNow = std::gmtime(&now);
				fprintf(sLog->_file, "[%04d-%02d-%02d %02d:%02d:%02d]", tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday, tNow->tm_hour + 8, tNow->tm_min, tNow->tm_sec);
				fprintf(sLog->_file, "%s", pLog);
				fflush(sLog->_file);
			}
			printf("%s\n", pLog);
		});

		
	}
	
	template<typename ...Args>
	static void Info(const char* pFormat,Args ...arg)
	{	
		CELLLog *sLog = &Instance();
		sLog->_CellTask.addTask([=]() {
			if (sLog->_file)
			{
				auto t = system_clock::now();
				auto now = system_clock::to_time_t(t);
				std::tm *tNow = std::gmtime(&now);
				fprintf(sLog->_file, "[%04d-%02d-%02d %02d:%02d:%02d]", tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday, tNow->tm_hour + 8, tNow->tm_min, tNow->tm_sec);
				fprintf(sLog->_file, pFormat, arg...);
				fflush(sLog->_file);
			}
			printf(pFormat, arg...);
		});
		
	}
public:
	FILE *_file = nullptr;
	CellTaskServer _CellTask;
};


#endif // !_CELLLOG_HPP_
