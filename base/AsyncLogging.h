#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H

#include "CountDownLatch.h"
#include "Mutex.h"
#include "Thread.h"
#include "LogStream.h"

#include <atomic>
#include <vector>

class AsyncLogging : noncopyable
{
public:
	AsyncLogging(const string& basename, int flushInterval = 2);

	~AsyncLogging()
	{
		if(running_) stop();
	}

	//供前端的生产者线程调用，将日志数据写道缓冲区
	void append(const char* logline, int len);

	void start()
	{
		running_ = true;
		thread_.start(); //日志线程启动
		latch_.wait(); //等待线程函数启动
	}

	void stop()
	{
		running_ = false;
		cond_.notify();
		thread_.join();
	}

private:
	//供后端消费者线程调用，将数据写到日志文件
	void threadFunc();

	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef vector<shared_ptr<Buffer>> BufferVector;
	typedef BufferVector::value_type BufferPtr;  //可以理解为Buffer的智能指针，可以管理Buffer的生存期，具备移动语义

	const int flushInterval_;  //超时时间，如果超时时间内缓冲区没有写满，仍将缓冲区中数据写入日志文件中

	bool running_;
	const string basename_;
	Thread thread_;
	CountDownLatch latch_;
	MutexLock mutex_;
	Condition cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
};


#endif
