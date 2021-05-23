#include "Logging.h"
#include "CurrentThread.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <time.h>  
#include <sys/time.h>

Logger::OutputFunc g_output;
Logger::FlushFunc g_flush;

Logger::Impl::Impl(const char* file, int line)
	: stream_(), line_(line), basename_(file)
{
	formatTime();
	CurrentThread::tid();  //缓存当前线程的id
	stream_ << CurrentThread::tidString();
}

void Logger::Impl::formatTime()
{
	struct timeval tv;
	time_t time;
	char str_t[26] = {0};
	gettimeofday(&tv, NULL);
	time = tv.tv_sec;
	struct tm* p_time = localtime(&time);
	strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
	stream_ << str_t ;
}


Logger::Logger(const char*file, int line, const char* func)
	: impl_(file, line)
{
	impl_.stream_ << func << ' ';
}

void Logger::Impl::finish()
{
	stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());
	g_output(buf.data(), buf.length());
}

void Logger::setOutput(OutputFunc out)
{
	g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
	g_flush = flush;
}
