#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include "../base/Condition.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"
#include<string>
using std::string;

class EventLoop;


//一个程序只要创建并运行了EvnetLoop，就是IO线程
//创建一个线程，在该线程函数中创建EventLoop对象并调用loop
class EventLoopThread : noncopyable
{
public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const string& name = string());

	~EventLoopThread();
	EventLoop* startLoop();  //启动线程，会调用thread中start函数，该线程成为IO线程

private:
	void threadFunc();    //线程函数

	EventLoop* loop_;  //指向一个EventLoop对象
	bool exiting_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
	ThreadInitCallback callback_;  //回调函数在loop事件循环之前调用
};


#endif
