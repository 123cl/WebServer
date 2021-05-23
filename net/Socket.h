#ifndef SOCKET_H
#define SOCKET_H

#include "../base/noncopyable.h"


class InetAddress;

class Socket : noncopyable
{
public:
	explicit Socket(int sockfd) : sockfd_(sockfd)
	{
	}

	~Socket();

	int fd() const { return sockfd_; }

	void bindAddress(const InetAddress& localaddr);
	void listen();

	int accept(InetAddress* peeraddr);

	void shutdownWrite();

	void setTcpNoDelay(bool on);

	void setReuseAddr(bool on);
	void setReusePort(bool on);

	// TCP keepalive是指定期探测连接是否存在，如果应用层有心跳的画，这个选项不是必须设置
	void setKeepAlive(bool on); 

private:
	const int sockfd_;
};




#endif
