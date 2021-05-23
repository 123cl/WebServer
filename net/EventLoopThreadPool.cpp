#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include<stdio.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg)
  : baseLoop_(baseLoop),
    started_(false),
    numThreads_(0),
    next_(0),
    name_(nameArg ? nameArg : "EventLoop")
{
}


EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start()
{
	assert(!started_);
	baseLoop_->assertInLoopThread();

	started_ = true;

	//创建线程
	for(int i = 0; i < numThreads_; i ++ )
	{
		char buf[name_.size() + 32];
		snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
		EventLoopThread* t = new EventLoopThread(cb, buf);
		threads_.push_back(std::unique_ptr<EventLoopThread>(t));
		loops_.push_back(t->startLoop());  //启动EventLoopThread线程，在进入事件循环之前，会调用cb
	}
}


//当新的连接到来的时候，需要选择新的eventloop对象来处理
EventLoop* EventLoopThreadPool::getNextLoop()
{
	baseLoop_->assertInLoopThread();
	assert(started_);

	EventLoop* loop = baseLoop_;  //baseLoop_为主reactor


	//如果不为空，按照轮叫的方式选择
	if(!loops_.empty())
	{
		loop = loops_[next_];
		++ next_;

		if(static_cast<size_t>(next_) >= loops_.size()) next_ = 0;
	}
	return loop;
}

























