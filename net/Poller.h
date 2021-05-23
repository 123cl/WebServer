#ifndef POLLER_H
#define POLLER_H

#include <map>
#include <vector>
#include "EventLoop.h"


class Channel;

class Poller : private noncopyable
{
public:
	typedef std::vector<Channel*> ChannelList;

	Poller(EventLoop* loop);
	virtual ~Poller();

	virtual void poll(int timeoutMs, ChannelList* activeChannels) = 0;

	virtual void updateChannel(Channel* channel) = 0;

	virtual void removeChannel(Channel* channel) = 0;

	virtual bool hasChannel(Channel* channel) const;

	void assertInLoopThread() const
	{
		loop_->assertInLoopThread();
	}

protected:
	typedef std::map<int, Channel*> ChannelMap;
	ChannelMap channels_;

private:
	EventLoop* loop_;
};

#endif
