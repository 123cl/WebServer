#include "SocketsOps.h"
#include<cstring>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <sys/socket.h>
#include <sys/uio.h>  // readv
#include <unistd.h>
#include<string>
#include<assert.h>
#include <stdint.h>
#include <endian.h>
using std::string;

void setNonBlockAndCloseOnExec(int sockfd)
{
	int flags = ::fcntl(sockfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int ret = ::fcntl(sockfd, F_SETFL, flags);
}

int sockets::createNonblockingOrDie(sa_family_t family)
{
	int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
	setNonBlockAndCloseOnExec(sockfd);

	return sockfd;
}


void sockets::bindOrDie(int sockfd, const struct sockaddr* addr)
{
	int res = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void sockets::listenOrDie(int sockfd)
{
	int res = ::listen(sockfd, SOMAXCONN);
}

int sockets::accept(int sockfd, struct sockaddr_in6* addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
	int connfd = ::accept(sockfd, (struct sockaddr*)(addr), &addrlen);
	setNonBlockAndCloseOnExec(connfd);
	return connfd;
}

ssize_t sockets::read(int sockfd, void *buf, size_t count)
{
	return ::read(sockfd, buf, count);
}


//与read的不同之处在于，接收的数据可以填充到多个缓冲区中
ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
	return ::readv(sockfd, iov, iovcnt);
}


ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
	return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd)
{
	::close(sockfd);
}

//关闭写的这一半
void sockets::shutdownWrite(int sockfd)
{
	::shutdown(sockfd, SHUT_WR);
}


void sockets::toIp(char* buf, size_t size, const struct sockaddr* addr)
{
	if(addr->sa_family == AF_INET)
	{
		assert(size >= INET_ADDRSTRLEN);
		const struct sockaddr_in* addr4 = (struct sockaddr_in*)(addr);
		::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
	}
	else if(addr->sa_family == AF_INET6)
	{
		assert(size >= INET6_ADDRSTRLEN);
		const struct sockaddr_in6* addr6 = (const struct sockaddr_in6*)(addr);
		::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
	}
}


void sockets::toIpPort(char* buf, size_t size, const struct sockaddr* addr)
{
	toIp(buf, size, addr);
	size_t end = ::strlen(buf);
	const struct sockaddr_in* addr4 = (struct sockaddr_in*)(addr);
	uint16_t port = be16toh(addr4->sin_port);
	assert(size > end);
	snprintf(buf+end, size-end, ":%u", port);
}

















































