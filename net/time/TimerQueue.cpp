#include "WebServer/net/TimerQueue.h"
#include "WebServer/net/EventLoop.h"
#include "WebServer/net/Timer.h"
#include "WebServer/net/TimerId.h"

#include <sys/timerfd.h>
#include <unistd.h>


int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	return timerfd;
}


void readTimerfd(int timerfd, Timestamp now)
{
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
}

//计算超时时刻与当前时间的时间差
struct timespec howMuchTimeFromNow(Timestamp when)
{
	int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
	if (microseconds < 100)
	{
		microseconds = 100;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
	return ts;
}

//重置定时器的超时时间
void resetTimerfd(int timerfd, Timestamp expiration)
{
	struct itimerspec newValue;
	struct itimerspec oldValue;
	memZero(&newValue, sizeof newValue);
	memZero(&oldValue, sizeof oldValue);
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
}


TimerQueue::TimerQueue(EventLoop* loop)
	    : loop_(loop),
    	      timerfd_(createTimerfd()),
              timerfdChannel_(loop, timerfd_),
    	      timers_(),
              callingExpiredTimers_(false)
{
	timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
	timerfdChannel_.enableReading();
}


TimerQueue::~TimerQueue()
{
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();
	::close(timerfd_);

	for(const Entry& timer : timers_)
	{
		delete timer.second;
	}
}

void TimerQueue::cancel(TimerId timerId)
{
	loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());
	ActiveTimer timer(timerId.timer_, timerId.sequence_);
	ActiveTimerSet::iterator it = activeTimers_.find(timer);
	if (it != activeTimers_.end())
	{
		size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
		delete it->first; 
		activeTimers_.erase(it);
	}
	else if (callingExpiredTimers_)
	{
    	//已经到期，并且正在调用回调函数的定时器
    		cancelingTimers_.insert(timer);
	}
  	assert(timers_.size() == activeTimers_.size());
}


TimerId addTimer(TimerCallback cb, Timestamp when, double interval)
{
	Timer* timer = new Timer(std::move(cb), when, interval);

	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));

	return TimerId(timer, timer->sequence());
}


void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assertInLoopThread();

	//插入一个定时器，有可能使得最早期的定时器发生改变
	bool earliestChanged = insert(timer);

	if(earliestChanged)
	{
		resetTimerfd(timerfd_, timer->expiration());
	}
}


void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();
	Timestamp now(Timestamp::now());
	readTimerfd(timerfd_, now);   //清除该事件，避免一直触发
	//获取该时刻之前的所有定时器列表
	std::vector<Entry> expired = getExpired(now);

	callingExpiredTimers_ = true;
	cancelingTimers_.clear();
	// safe to callback outside critical section
	for (const Entry& it : expired)
	{
		it.second->run();
	}
	callingExpiredTimers_ = false;

	//不是一次性定时器，需要重启
	reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
	assert(timers_.size() == activeTimers_.size());
	std::vector<Entry> expired;
	Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	//返回第一个未到期的迭代器
	//这里返回的是第一个大于的元素，因为重载了比较运算符，sentry的地址已经是最大的
	TimerList::iterator end = timers_.lower_bound(sentry);
	assert(end == timers_.end() || now < end->first);
	//插入到期的定时器
	std::copy(timers_.begin(), end, back_inserter(expired));
	timers_.erase(timers_.begin(), end);
	
	for (const Entry& it : expired)
	{
		ActiveTimer timer(it.second, it.second->sequence());
		size_t n = activeTimers_.erase(timer);
	}

	return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
	Timestamp nextExpire;

	for (const Entry& it : expired)
	{
		ActiveTimer timer(it.second, it.second->sequence());
    //如果是重复的定时器并且是未取消定时器，则重启该定时器
		if (it.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
		{
			it.second->restart(now);
			insert(it.second);
		}
		else
		{
	    		delete it.second; // FIXME: no delete please
		}
	}

	if (!timers_.empty())
	{
		nextExpire = timers_.begin()->second->expiration();
	}

	if (nextExpire.valid())
	{
		resetTimerfd(timerfd_, nextExpire);
	}
}

bool TimerQueue::insert(Timer* timer)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());
  //最早到期时间是否改变
	bool earliestChanged = false;
	Timestamp when = timer->expiration();
	TimerList::iterator it = timers_.begin();//第一个定时器
  //如果为空或者when小于timers中的最早到期时间
	if (it == timers_.end() || when < it->first)
	{
		earliestChanged = true;
	}
    //插入到timers中
	std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
    //插入到activetimers中
	std::pair<ActiveTimerSet::iterator, bool> result = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
	return earliestChanged;
}




















