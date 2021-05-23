#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include "../base/noncopyable.h"

#include <functional>
#include <memory>
#include <vector>
#include<string>
using std::string;

class EventLoop;
class EventLoopThread;


//IO线程池类，开启若干个线程，并让这些IO线程处于事件循环状态
class EventLoopThreadPool : private noncopyable
{
public:
	EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
	~EventLoopThreadPool();

	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	void start();

	EventLoop* getNextLoop();

private:
	EventLoop* baseLoop_; //与Acceptor所属的EventLoop相同
	string name_;
	bool started_;
	int numThreads_; //线程数
	int next_;   //新线程到来，所选择的EventLoop对象下标
	std::vector<std::unique_ptr<EventLoopThread>> threads_;  //IO线程列表
	std::vector<EventLoop*> loops_;

};

#endif
