#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "Channel.h"
#include "Socket.h"

class Acceptor : noncopyable
{
public:
	typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

	Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport = false);
	~Acceptor();

	void setNewConnectionCallback(const NewConnectionCallback& cb)
 	{ newConnectionCallback_ = cb; }

	void listen();
	bool listening() { return listening_;}

private:
	void handleRead();

	EventLoop* loop_;
	Socket acceptSocket_;
	Channel acceptChannel_;  //观察套接字的可读事件
	NewConnectionCallback newConnectionCallback_;
	bool listening_;
	int idleFd_;
};


#endif
