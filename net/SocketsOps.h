#ifndef SOCKETSOPS_H
#define SOCKETSOPS_H

#include<arpa/inet.h>

namespace sockets
{
	int createNonblockingOrDie(sa_family_t family);  //创建非阻塞套接字

	void bindOrDie(int sockfd, const struct sockaddr* addr);
	void listenOrDie(int sockfd);
	int accept(int sockfd, struct sockaddr_in6* addr);
	ssize_t read(int sockfd, void *buf, size_t count);
	ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
	ssize_t write(int sockfd, const void *buf, size_t count);
	void close(int sockfd);
	void shutdownWrite(int sockfd);


	void toIpPort(char* buf, size_t size, const struct sockaddr* addr);
	void toIp(char* buf, size_t size, const struct sockaddr* addr);

	struct sockaddr_in6 getLocalAddr(int sockfd);
	struct sockaddr_in6 getPeerAddr(int sockfd);
}


#endif
