#include "Socket.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>  // snprintf


Socket::~Socket()
{
	sockets::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr)
{
	sockets::bindOrDie(sockfd_, addr.getSockAddrInet());
}


void Socket::listen()
{
	sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr)
{
	struct sockaddr_in6 addr;
	memset(&addr, 0, sizeof addr);
	int connfd = sockets::accept(sockfd_, &addr);
	return connfd;
}


void Socket::shutdownWrite()
{
	sockets::shutdownWrite(sockfd_);
}


//TCP_NODELAY禁用Nagle算法
//禁用此算法可以避免连续发包出现的延迟，对于编写低延迟的网络服务很重要
void Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}


void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}


void Socket::setReusePort(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
}


void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}




























