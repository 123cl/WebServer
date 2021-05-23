#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include<iostream>
#include <atomic>
#include <functional>
#include <vector>
#include <boost/any.hpp>
#include "../base/Mutex.h"
#include "../base/CurrentThread.h"
#include "TimeWheel.h"

class Channel;
class Poller;
class TimeWheel;

class EventLoop : private noncopyable
{
public:
	typedef std::function<void()> Functor;
	typedef std::function<void()> TimeCallback;

	EventLoop();
	~EventLoop();

	void loop();
	void quit();

	void runInLoop(Functor cb);
	void queueInLoop(Functor cb);

	size_t queueSize() const;

	void wakeup();

	void updateChannel(Channel* channel); //往LOOP中添加或者更新通道
	void removeChannel(Channel* channel); //从POLL中移除通道

	bool hasChannel(Channel* channel);


	void assertInLoopThread()
	{
		if(!isInLoopThread())
		{
			std::cout << "EventLoop::abortNotInLoopThread - EventLoop" << std::endl;
			assert(isInLoopThread());
		}
	}
	
	void startTimer(double interval, TimeCallback callback);
	void setOnTimeCallback(TimeCallback callback);
	void TimerhandleRead(Channel* timerChannel);

	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

	bool eventHandling() const { return eventHandling_; }

	void setContext(const boost::any& context)
	{
		context_ = context;
	}

	const boost::any& getContext() const
	{
		return context_;
	}

	boost::any* getMutableContext()
	{
		return &context_;
	}

	static EventLoop* getEventLoopOfCurrentThread();

private:
	void handleRead();  // waked up
	void doPendingFunctors();

	typedef std::vector<Channel*> ChannelList;
	bool looping_;

	std::atomic<bool> quit_;

	bool eventHandling_;
	bool callingPendingFunctors_;

	const pid_t threadId_;  //当前对象所属线程ID

	std::unique_ptr<Poller> poller_;
	
	TimeCallback timerCallback_;
		
	int wakeupFd_;  //用于保存eventfd所创建的文件描述符
	std::unique_ptr<Channel> wakeupChannel_;  //wakefd所对应的通道
	
	boost::any context_;
	ChannelList activeChannels_;
	Channel* currentActiveChannel_;

	mutable MutexLock mutex_;
	std::vector<Functor> pendingFunctors_;
};

#endif
