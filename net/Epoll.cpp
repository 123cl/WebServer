#include"Epoll.h"
#include"Channel.h"
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>


const int kNew = -1, kAdded = 1, kDeleted = 2;

Epoll::Epoll(EventLoop* loop)
	: Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
	  events_(kInitEventListSize)
{
}


Epoll::~Epoll()
{
	close(epollfd_);
}

void Epoll::poll(int timeoutMs, ChannelList* activeChannels)
{
	int nums = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), 
			      timeoutMs);

	int savedErrno = errno;

	if(nums > 0)
	{
		fillActiveChannels(nums, activeChannels);
		if (nums == events_.size())
		{
			events_.resize(events_.size()*2);
		}
	}
	else if(nums < 0)
	{
		if(savedErrno != EINTR)
		{
			errno = savedErrno;
		}
	}
	return;
}

void Epoll::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
	for (int i = 0; i < numEvents; ++i)
	{
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		channel->set_revents(events_[i].events);
		activeChannels->push_back(channel);
	}
}

void Epoll::updateChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	const int index = channel->index();

	if(index == kNew || index == kDeleted)
	{
		int fd = channel->fd();
		if(index == kNew)
		{
			assert(channels_.find(fd) == channels_.end());
			channels_[fd] = channel;
		}
		else
		{
			assert(channels_.find(fd) != channels_.end());
			assert(channels_[fd] == channel);
		}

		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
	}
	else
	{
		int fd = channel->fd();
		assert(channels_.find(fd) != channels_.end());
		assert(channels_[fd] == channel);
		assert(index == kAdded);

		if (channel->isNoneEvent()) //剔除关注
		{
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted); //kDeleted仅仅表示没有用EPOLL关注，不代表从channel中移除了
		}
		else
		{
			update(EPOLL_CTL_MOD, channel);
		}
	}
}


void Epoll::removeChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	int fd = channel->fd();
	assert(channels_.find(fd) != channels_.end());
	assert(channels_[fd] == channel);
	assert(channel->isNoneEvent());
	int index = channel->index();
	
	size_t n = channels_.erase(fd);

	if(index == kAdded)
	{
		update(EPOLL_CTL_DEL, channel);
	}

	channel->set_index(kNew);
}


void Epoll::update(int operation, Channel* channel)
{
	struct epoll_event event;
	memset(&event, 0, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();

	epoll_ctl(epollfd_, operation, fd, &event);
}






















