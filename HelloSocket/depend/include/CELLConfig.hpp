#ifndef _CELL_CONFIG_HPP_
#define _CELL_CONFIG_HPP_
/*
	专门用于读取配置数据
	目前我们的配置数据主要来源于main函数args传入
*/

#include <string>
#include <map>
#include "CELLLog.hpp"
class CELLConfig
{
private:
	CELLConfig()
	{

	}
	~CELLConfig()
	{


	}
public:
	static CELLConfig& Instance()
	{
		static  CELLConfig obj;
		return obj;
	}

	void Init(int argc, char *args[])
	{
		_exepath = args[0];
		for (int  i = 0; i < argc; i++)
		{
			//CELLLog_Info(args[i]);
			madeCmd(args[i]);
		}
	}

	void madeCmd(char* cmd)
	{
		char* val = strchr(cmd,'=');
		if (val)
		{
			val[0] = '\0';
			val++;
			_kv[cmd] = val;
			CELLLog_Info("key = %s,val = %s",cmd,val);
		}
		else
		{
			_kv[cmd] = "";
			CELLLog_Info("key = %s no val ", cmd);
		}
	}

	const char* getStr(const char* key, const char* def)
	{
		auto iter = _kv.find(key);
		if (iter == _kv.end())
		{
			CELLLog_Error("CELLConfig::getStr,key=%s not find",key);
		}
		else
		{	
			def = iter->second.c_str();
			CELLLog_Info("%s=%s", key, def);
		}
		return def; 
	}

	int getInt(const char* key, int def)
	{
		auto iter = _kv.find(key);
		if (iter == _kv.end())
		{
			CELLLog_Error("CELLConfig::getStr,key=%s not find", key);
		}
		else
		{
			def = atoi(iter->second.c_str());
			CELLLog_Info("%s=%d", key, def);
		}
		return def;
	}

	bool hasKey(const char* key)
	{
		auto iter = _kv.find(key);
		return iter != _kv.end();
	}
private:
	//当前程序路径
	std::string _exepath;

	std::map<std::string, std::string> _kv;
};


#endif // !_CELL_CONFIG_HPP_ 
