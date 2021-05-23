#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <set>
#include <vector>

#include "WebServer/base/Mutex.h"
#include "WebServer/base/Timestamp.h"
#include "WebServer/net/Callbacks.h"
#include "WebServer/net/Channel.h"


class EventLoop;
class Timer;
class TimerId;

class TimerQueue : noncopyable
{
public:
	explicit TimerQueue(EventLoop* loop);
	~TimerQueue();

	TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
	void cancel(TimerId timerId);


private:
	typedef std::pair<Timestamp, Timer*> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;


	void addTimerInLoop(Timer* timer);
	void cancelInLoop(TimerId timerId);

	void handleRead();

	//返回超时的定时器列表
	std::vector<Entry> getExpired(Timestamp now);

	//对超时的定时器重置
	void reset(const std::vector<Entry>& expired, Timestamp now);

	//插入定时器
	bool insert(Timer* timer);

	EventLoop* loop_;
	const int timerfd_;
	Channel timerfdChannel_;
	TimerList timers_;

	ActiveTimerSet activeTimers_;
	bool callingExpiredTimers_;
	ActiveTimerSet cancelingTimers_;  //保存的是被取消的定时器
};


#endif
