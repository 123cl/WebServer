#include "EventLoop.h"
#include "../base/Mutex.h"
#include "Channel.h"
#include "Poller.h"
#include "Epoll.h"
#include "SocketsOps.h"
#include<algorithm>
#include<signal.h>
#include<sys/eventfd.h>
#include <sys/timerfd.h>
#include<unistd.h>


//当前线程Eventloop对象指针
//线程局部存储
__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;  //超时时间 10s

int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    abort();
  }
  return evtfd;
}

int createTimerfd(double interval)
{
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
        if(timerfd <= 0)
        {
                std::cout << "Failed in create timerfd" << std::endl;
        }
	
	struct timespec time;
	time.tv_sec=interval / 1;
	time.tv_nsec=(interval-time.tv_sec)*1000*1000; 

        struct itimerspec its;
        its.it_value = time;
        its.it_interval = time;

        if (::timerfd_settime(timerfd, 0, &its, nullptr) != 0)
        {
                std::cout << "Fail to set timerfd!" << endl;
        }

        return timerfd;
}


class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
  }
};

IgnoreSigPipe initObj;


EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}



EventLoop::EventLoop()
	: looping_(false), quit_(false), eventHandling_(false),
          callingPendingFunctors_(false),
          threadId_(CurrentThread::tid()),
          poller_(new Epoll(this)),
          wakeupFd_(createEventfd()),
          wakeupChannel_(new Channel(this, wakeupFd_)),
          currentActiveChannel_(NULL)
{
 	 //如果已创建Eventloop对象，则终止
	if (t_loopInThisThread)
	{

	}	
	else
	{
	  t_loopInThisThread = this;
	}
  	//设定wakechannel的回调函数，并让poll管理
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));

	wakeupChannel_->enableReading();
}


EventLoop::~EventLoop()
{
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread = NULL;
}


//事件循环，该函数不能跨线程调用
void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;

	while(!quit_)
	{
		activeChannels_.clear();
		poller_->poll(kPollTimeMs, &activeChannels_);

		eventHandling_ = true;

		for(auto channel : activeChannels_)
		{
			currentActiveChannel_ = channel;
			currentActiveChannel_->handleEvent();
		}

		currentActiveChannel_ = NULL;
		eventHandling_ = false;

		doPendingFunctors();

	}

	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;

	if(!isInLoopThread())
	{
		wakeup();
	}
}


void EventLoop::startTimer(double interval, TimeCallback cb)
{
	setOnTimeCallback(cb);
	int timerfd = createTimerfd(interval);

	Channel* timerfdChannel_ = new Channel(this, timerfd);
        timerfdChannel_->setReadCallback(std::bind(&EventLoop::TimerhandleRead, this, timerfdChannel_));
        timerfdChannel_->enableReading();
}

void EventLoop::setOnTimeCallback(TimeCallback callback)
{
	timerCallback_ = std::move(callback);
}

void EventLoop::TimerhandleRead(Channel* timerChannel)
{
        uint64_t howmany;
        ssize_t n = ::read(timerChannel->fd(), &howmany, sizeof howmany);

        if(n != sizeof howmany)
        {
                std::cout << "handlRead() reads " << n << " bytes instead of 8" << std::endl;
        }
	
	timerCallback_();
}

void EventLoop::wakeup()
{
	u_int64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
}

void EventLoop::handleRead()
{
	u_int64_t one = 1;
	ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
}

//在IO线程中执行某个回调函数，该函数可以跨线程调用
void EventLoop::runInLoop(Functor cb)
{
	if(isInLoopThread())
	{
		//如果是在当前IO线程调用runinloop，则同步调用cb
		cb();
	}
	else
	{
		//如果是其他线程调用，则异步将cb添加入队列
		queueInLoop(std::move(cb));
	}
}


void EventLoop::queueInLoop(Functor cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));
	}

	//调用此函数的线程不是IO线程需要唤醒
	//调用此函数的线程是当前IO线程，并且此时正在调用pendingfuctor，需要唤醒
	
	if(!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}	
}


void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}

	for(auto functor : functors)
	{
		functor();
	}
	callingPendingFunctors_ = false;
}

size_t EventLoop::queueSize() const
{
	MutexLockGuard lock(mutex_);
	return pendingFunctors_.size();
}

void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if(eventHandling_)
	{
		assert(currentActiveChannel_ == channel || std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
	}
	poller_->removeChannel(channel);
}


bool EventLoop::hasChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	return poller_->hasChannel(channel);
}

























































