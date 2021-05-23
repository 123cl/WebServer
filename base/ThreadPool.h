#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include<string>
#include<deque>
#include<vector>
using namespace std;

class ThreadPool : noncopyable
{
public:
	typedef std::function<void()> Task;

	explicit ThreadPool(const string& nameArg = string("ThreadPool"));
	~ThreadPool();

	void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
	void setThreadInitCallback(const Task& cb)
	{
		threadInitCallback_ = cb;
	}

	void start(int numThreads);
	void stop();

	size_t queueSize() const;

	void run(Task f);


private:
	bool isFull() const;
	void runInThread(); // 线程池中的线程所执行的任务
	Task take();

	mutable MutexLock mutex_;
	Condition notEmpty_;
	Condition notFull_;

	string name_;
	Task threadInitCallback_;
	std::vector<std::unique_ptr<Thread>> threads_;
	std::deque<Task> queue_;
	size_t maxQueueSize_;
	bool running_;
};

#endif

