#ifndef CHANNEL_H
#define CHANNEL_H

#include "../base/noncopyable.h"
#include <functional>
#include <memory>

class EventLoop;

class Channel :private noncopyable
{
public:
	typedef std::function<void()> EventCallback;
 	typedef std::function<void()> ReadEventCallback;

 	Channel(EventLoop* loop, int fd);
 	~Channel();

	void handleEvent();
  	void setReadCallback(ReadEventCallback cb)
  	{ 
		readCallback_ = std::move(cb); 
	}

  	void setWriteCallback(EventCallback cb)
  	{ 
		writeCallback_ = std::move(cb); 
	}

  	void setCloseCallback(EventCallback cb)
  	{ 
		closeCallback_ = std::move(cb); 
	}

  	void setErrorCallback(EventCallback cb)
  	{ 
		errorCallback_ = std::move(cb); 
	}
	
	//控制TcpConnection对象的生存周期
  	void tie(const std::shared_ptr<void>&);

  	int fd() const { return fd_; }
  	
	int events() const { return events_; }
  	void set_revents(int revt) 
	{ 
		revents_ = revt; 
	}
  
  	bool isNoneEvent() const { return events_ == kNoneEvent; }

  	void enableReading() { events_ |= kReadEvent; update(); }
  	void disableReading() { events_ &= ~kReadEvent; update(); }
  	void enableWriting() { events_ |= kWriteEvent; update(); }
  	void disableWriting() { events_ &= ~kWriteEvent; update(); }
  	void disableAll() { events_ = kNoneEvent; update(); }
  	bool isWriting() const { return events_ & kWriteEvent; }
  	bool isReading() const { return events_ & kReadEvent; }
	
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }	

  	EventLoop* ownerLoop() { return loop_; }
  	void remove();

 private:
  	void update();
  	void handleEventWithGuard();

  	static const int kNoneEvent;
  	static const int kReadEvent;
  	static const int kWriteEvent;

  	EventLoop* loop_;  //所属EventLoop
  	const int  fd_;
  	int        events_;  //注册了哪些事件
  	int        revents_; // poll返回的事件
	int index_ = -1;

  	std::weak_ptr<void> tie_;
  	bool tied_;
  	bool eventHandling_;
  	bool addedToLoop_;
  	ReadEventCallback readCallback_;
  	EventCallback writeCallback_;
  	EventCallback closeCallback_;
  	EventCallback errorCallback_;
};

#endif
