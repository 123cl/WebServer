#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketsOps.h"
#include <stdio.h>  // snprintf



TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
  // non-block
	: loop_(loop), ipPort_(listenAddr.toIpPort()), name_(nameArg), 
	  acceptor_(new Acceptor(loop, listenAddr)),
	  threadPool_(new EventLoopThreadPool(loop, name_)),
	  nextConnId_(1),
	  started_(false)
{
	//acceptor::handleread函数中会回调TcpServer::newConnection
	acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}


TcpServer::~TcpServer()
{
	loop_->assertInLoopThread();

	for(auto &p : connections_)
	{
		TcpConnectionPtr conn(p.second);
		p.second.reset();
		conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
	}
}


void TcpServer::setThreadNum(int numThreads)
{
	threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
	if(!started_) started_ = true;
	threadPool_->start();

	assert(!acceptor_->listening());
	loop_->runInLoop(std::bind(&Acceptor::listen, get_pointer(acceptor_)));
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
	loop_->assertInLoopThread();
	EventLoop* ioLoop = threadPool_->getNextLoop();
	char buf[64];
	snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);

	++nextConnId_;
	string connName = name_ + buf;

	TcpConnectionPtr conn(new TcpConnection(ioLoop,connName, sockfd, peerAddr));

	connections_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

	ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}



void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
	loop_->assertInLoopThread();
	size_t n = connections_.erase(conn->name()); //引用计数变为2

	assert(n == 1);
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}


