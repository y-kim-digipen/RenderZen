#pragma once
#include <iostream>
#include <string>


enum LogSeverityLevel
{
	NONE,
	VERB,
	WARN,
	ERR,
	COUNT,
};

inline void Log(LogSeverityLevel lvl, std::string msg)
{
	std::cout << "[" << lvl << "]" << msg << std::endl;
}