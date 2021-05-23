#include "LogFile.h"
#include "FileUtil.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <unistd.h>
using std::string;

LogFile::LogFile(const string& basename,  int flushInterval, int checkEveryN)
	: basename_(basename),
	  flushInterval_(flushInterval),
	  checkEveryN_(checkEveryN),
	  count_(0),
	  mutex_(new MutexLock)
{
	assert(basename.find('/') == string::npos); //断言basename不包含/
	rollFile(); //第一次滚动日志，就是产生一个文件
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len)
{
	MutexLockGuard lock(*mutex_);
	append_unlocked(logline, len);
}

void LogFile::append_unlocked(const char* logline, int len)
{
	file_->append(logline, len);

	if(file_->writtenBytes() > rollSize_)
		rollFile();
	else
	{
		++count_;
		if(count_ >= checkEveryN_)
		{
			count_ = 0;
			file_->flush();
		}
	}
}

void LogFile::rollFile()
{
	time_t now = 0;
	string filename = getLogFileName(basename_, &now);
	file_.reset(new AppendFile(filename));
}

//获取日志文件的名称
string LogFile::getLogFileName(const string& basename, time_t* now)
{
	string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;

	char timebuf[32];
	struct tm tm;
	*now = time(NULL);   //获取当前时间
	gmtime_r(now, &tm); // FIXME: localtime_r ?
	//将时间格式化
	strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
	filename += timebuf;

	char pidbuf[32];
	snprintf(pidbuf, sizeof pidbuf, ".%d", ::getpid());
	filename += pidbuf;

	filename += ".log";

	return filename;
}
