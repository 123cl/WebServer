#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "../base/noncopyable.h"
#include "Buffer.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include <memory>
#include <boost/any.hpp>
#include<string.h>
#include<string>
#include "Buffer.h"
using std::string;

class Channel;
class EventLoop;
class Socket;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(EventLoop* loop, const string& name, int sockfd, const InetAddress& peerAddr);

	~TcpConnection();

	EventLoop* getLoop() const { return loop_; }
	const string& name() const { return name_; }
	const InetAddress& peerAddress() const { return peerAddr_; }
	bool connected() const { return state_ == kConnected; }
	bool disconnected() const { return state_ == kDisconnected; }
	
	void send(const char* data, size_t len);
	void send(Buffer* message);

	void shutdown();
	void setTcpNoDelay(bool on);

	void settimerContext(const boost::any& context)
	{
		timercontext_ = context;
	}
	
	void sethttpContext(const boost::any& context)
	{
		httpcontext_ = context;
	}
	
	void setfdContext(const boost::any& context)
	{
		fdcontext_ = context;
	}

	const boost::any& gettimerContext() const { return timercontext_; }
	const boost::any& getfdContext() const { return fdcontext_; }
	const boost::any& gethttpContext() const { return httpcontext_; }

	boost::any* getMutabletimerContext() { return &timercontext_; }
	boost::any* getMutablefdContext() { return &fdcontext_; }
	boost::any* getMutablehttpContext() { return &httpcontext_; }

	void setConnectionCallback(const ConnectionCallback& cb)
	{
		connectionCallback_ = cb;
	}

	void setMessageCallback(const MessageCallback& cb)
  	{ messageCallback_ = cb; }

	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{ writeCompleteCallback_ = cb; }

	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
  	{ highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }


	void setCloseCallback(const CloseCallback& cb)
  	{ closeCallback_ = cb; }

	Buffer* outputBuffer() 
	{
		return &outputBuffer_; 
	}
	Buffer* inputBuffer()
	{
		return &inputBuffer_;	
	}
	
	void connectEstablished();
	void connectDestroyed();

private:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();
	
	void sendInLoop(const string& message);
	void sendInLoop(const void* message, size_t len);

	void shutdownInLoop();
	void setState(StateE s) { state_ = s; }

	EventLoop* loop_;
	const string name_;
	StateE state_;
	bool reading_;
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;
	const InetAddress peerAddr_;


	ConnectionCallback connectionCallback_;  //???????????????????????????
	MessageCallback messageCallback_;       //???????????????????????????
	WriteCompleteCallback writeCompleteCallback_; //?????????????????????????????????????????????????????????????????????????????????????????????????????????
	HighWaterMarkCallback highWaterMarkCallback_;

	CloseCallback closeCallback_;  //???????????????????????????????????????
	size_t highWaterMark_;     //????????????
	Buffer inputBuffer_;  //????????????????????????
	Buffer outputBuffer_;
	boost::any timercontext_;    //??????????????????????????????????????????
	boost::any httpcontext_;
	boost::any fdcontext_;	
};



#endif
