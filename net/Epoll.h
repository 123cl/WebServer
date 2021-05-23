#ifndef EPOLL_H
#define EPOLL_H

#include "Poller.h"
#include <vector>

struct epoll_event;

class Epoll : public Poller
{
public:
	Epoll(EventLoop* loop);
	~Epoll();

	void poll(int timeoutMs, ChannelList* activeChannels) override;

	void updateChannel(Channel* channel) override;
	void removeChannel(Channel* channel) override;

private:
	static const int kInitEventListSize = 16;

	void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

	void update(int operation, Channel* channel);

	typedef std::vector<struct epoll_event> EventList;

	int epollfd_;
	EventList events_;
};


#endif
