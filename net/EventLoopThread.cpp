#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const string& name)
	: loop_(NULL),
    	  exiting_(false),
    	  thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    	  mutex_(),
   	  cond_(mutex_),
  	  callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
	exiting_ = true;
	if(loop_ != NULL)
	{
		loop_->quit();
		thread_.join();
	}
}

EventLoop* EventLoopThread::startLoop()
{
	assert(!thread_.started());
	thread_.start();//启动线程，这个线程成为了IO线程

	EventLoop* loop = NULL;

	
	{
		MutexLockGuard lock(mutex_);
		while(loop_ == NULL)
		{
			cond_.wait();
		}
		loop = loop_;
	}

	return loop;
}

//这个线程结束了，整个程序就结束了
void EventLoopThread::threadFunc()
{
	EventLoop loop;

	if(callback_) callback_(&loop);

	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop; //loop指针指向一个栈上的对象。threadFunc函数退出之后，这个指针就失效
		cond_.notify();
	}

	loop.loop();

	MutexLockGuard lock(mutex_);
	loop_ = NULL;
}














