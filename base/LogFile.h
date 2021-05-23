#ifndef LOGFILE_H
#define LOGFILE_H

#include "Mutex.h"
#include <memory>
#include "FileUtil.h"
#include <string>
using std::string;

class LogFile : noncopyable
{
public:
	LogFile(const string& basename, int flushInterval = 2, int checkEveryN = 1024);
	~LogFile();

	void append(const char* logline, int len);
	void flush();
	void rollFile();

private:
	void append_unlocked(const char* logline, int len);

	static string getLogFileName(const string& basename, time_t* now); //获取日志文件的名称

	const string basename_;
	const off_t rollSize_ = 1024 * 1024 * 100;  //日志文件写满rollsize就换个文件
	const int flushInterval_;  //日志写入的间隔时间
	const int checkEveryN_;

	int count_;

	std::unique_ptr<MutexLock> mutex_;
	std::unique_ptr<AppendFile> file_;
};


#endif
