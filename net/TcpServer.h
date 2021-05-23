#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "TimeWheel.h"
#include "TcpConnection.h"
#include <map>
#include<string>
using std::string;

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);
	~TcpServer();

	const string& ipPort() const { return ipPort_; }
	const string& name() const { return name_; }
	EventLoop* getLoop() const { return loop_; }

	void setThreadNum(int numThreads);
	void setThreadInitCallback(const ThreadInitCallback& cb)
	{
		threadInitCallback_ = cb;
	}

	std::shared_ptr<EventLoopThreadPool> threadPool()
	{
		return threadPool_;
	}

	void start();

	//设置连接到来或连接关闭的回调函数
	void setConnectionCallback(const ConnectionCallback& cb)
	{
		connectionCallback_ = cb;
	}

	void setMessageCallback(const MessageCallback& cb)
	{
		messageCallback_ = cb;
	}

	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{
		writeCompleteCallback_ = cb;
	}

private:
	void newConnection(int sockfd, const InetAddress& peerAddr);  //连接到来回调的函数
	void removeConnection(const TcpConnectionPtr& conn);
	void removeConnectionInLoop(const TcpConnectionPtr& conn);

	typedef std::map<string, TcpConnectionPtr> ConnectionMap;

	EventLoop* loop_;
	const string ipPort_;
	const string name_;
	std::unique_ptr<Acceptor> acceptor_;
	std::shared_ptr<EventLoopThreadPool> threadPool_;

	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	ThreadInitCallback threadInitCallback_;
	
	int nextConnId_;
	ConnectionMap connections_;
	bool started_;
};

#endif
