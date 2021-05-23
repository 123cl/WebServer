#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"
#include <errno.h>


TcpConnection::TcpConnection(EventLoop* loop,
                             const string& nameArg,
                             int sockfd,
                             const InetAddress& peerAddr)
  : loop_(loop),
    name_(nameArg),
    state_(kConnecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024)
{
	//通道可读事件到来的时候会回调TcpConnection::handleread
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));

	//连接关闭，回调handleclose
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
	assert(state_ == kDisconnected);
}


void TcpConnection::send(const char* data, size_t len)
{
	if(state_ == kConnected)
	{
		if(loop_->isInLoopThread())
		{
			sendInLoop(data, len);
		}
		else
		{
			void (TcpConnection::*fp)(const string& message) = &TcpConnection::sendInLoop;
			string message(static_cast<const char*>(data), len);
			loop_->runInLoop(std::bind(fp, this, message));
		}
	}
}

void TcpConnection::send(Buffer* buf)
{
	if(state_ == kConnected)
	{
		if(loop_->isInLoopThread())
		{
			sendInLoop(buf->peek(), buf->readableBytes());
			buf->retrieveAll();
		}
		else
		{
			void (TcpConnection::*fp)(const string& message) = &TcpConnection::sendInLoop;
			loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
		}
	}
}

void TcpConnection::sendInLoop(const string& str)
{
	sendInLoop(str.data(), str.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
	loop_->assertInLoopThread();
	ssize_t nwrote = 0;
	size_t remaining = len;
	bool faultError = false;
	if(state_ == kDisconnected)
	{
		return ;
	}

	//通道没有关注可写事件并且发送缓冲区没有数据，直接write
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
	{
		nwrote = sockets::write(channel_->fd(), data, len);
		if(nwrote > 0)
		{
			remaining = len - nwrote;
			if(remaining == 0 && writeCompleteCallback_)
			{
				loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
			}
		}
		else
		{
			nwrote = 0;
			if (errno != EWOULDBLOCK)
			{
				if (errno == EPIPE || errno == ECONNRESET) 
					faultError = true;
			}
		}
	}


	assert(remaining <= len);
	//有未写完的数据，说明内核发送缓冲区满，要将未写完的数据添加到output buffer中
	if (!faultError && remaining > 0)
	{
		size_t oldLen = outputBuffer_.readableBytes();
		if(oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
		{
			loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
		}
		outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
		if(!channel_->isWriting())
		{
			channel_->enableWriting();  //关注POLLOUT事件
		}
	}
}

void TcpConnection::setTcpNoDelay(bool on)
{
	socket_->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);

	channel_->tie(shared_from_this());
	channel_->enableReading();
	
	connectionCallback_(shared_from_this());
}


void TcpConnection::connectDestroyed()
{
	loop_->assertInLoopThread();
	if(state_ == kConnected)
	{
		setState(kDisconnected);
		channel_->disableAll();

		connectionCallback_(shared_from_this());
	}
	channel_->remove();
}


void TcpConnection::handleRead()
{
	loop_->assertInLoopThread();
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);  //把数据读到缓冲区中
	if(n > 0)
	{
		messageCallback_(shared_from_this(), &inputBuffer_);
	}
	else if(n == 0)
	{
		handleClose();
	}
	else
	{
		errno = savedErrno;
		handleError();
	}
}

//POLLOUT事件触发，回调该函数，内核发送缓冲区有空间了
void TcpConnection::handleWrite()
{
	loop_->assertInLoopThread();
	if (channel_->isWriting())
	{
		ssize_t n = sockets::write(channel_->fd(),
                               outputBuffer_.peek(),
                               outputBuffer_.readableBytes());

		if(n > 0)
		{
			outputBuffer_.retrieve(n);
			if (outputBuffer_.readableBytes() == 0)   //发送缓冲区已清空
			{
				channel_->disableWriting();  //停止关注POLLOUT事件
				if(writeCompleteCallback_)
				{
					loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
				}

				if(state_ == kDisconnecting) 
				{
					shutdownInLoop();  //关闭连接
				}
			}
		}
	}
}




void TcpConnection::handleClose()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnected || state_ == kDisconnecting);

	setState(kDisconnected);
	channel_->disableAll();

	TcpConnectionPtr guardThis(shared_from_this()); //返回自身对象的shared_ptr
	connectionCallback_(guardThis);  //此时引用计数为3
	closeCallback_(guardThis);
}


void TcpConnection::handleError()
{

}

void TcpConnection::shutdown()
{
	if(state_ == kConnected)
	{
		setState(kDisconnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop()
{
	loop_->assertInLoopThread();
	if(!channel_->isWriting()) //没有关注pollout事件
	{
		socket_->shutdownWrite();
	}
}


























































