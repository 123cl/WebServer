#ifndef FILEUTIL_H
#define FILEUTIL_H

#include "noncopyable.h"
#include <string>
using std::string;

class AppendFile : noncopyable
{
public:
	explicit AppendFile(string filename);
	~AppendFile();

	void append(const char* logline, size_t len); //往文件写
	void flush();

	off_t writtenBytes() const { return writtenBytes_; }

private:
	size_t write(const char* logline, size_t len);

	FILE* fp_;
	char buffer_[64 * 1024];
	off_t writtenBytes_;  //已经写入的字节数
};

#endif

