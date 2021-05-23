#ifndef LOGGING_H
#define LOGGING_H

#include "LogStream.h"
#include <string>

class Logger
{
public:
        Logger(const char* file, int line, const char* func);
	~Logger();
	LogStream& stream() { return impl_.stream_; }
	
	typedef void(*OutputFunc)(const char*mag, int len);
	typedef void(*FlushFunc)();
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);

private:
	class Impl
	{
	public:
		Impl(const char* file, int line);
		void formatTime();
		void finish();

		LogStream stream_;
		int line_;
		std::string basename_;
	};

	Impl impl_;
};

#define LOG logger(__FILE__, __LINE__, __func__).stream()

#endif

