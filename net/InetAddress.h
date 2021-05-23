#ifndef INETADDRESS_H
#define INETADDRESS_H

#include<string.h>
#include<string>
#include <netinet/in.h>
#include <assert.h>
#include<stdint.h>

template<typename To, typename From>
To implicit_cast(From const &f)
{
        return f;
}

class InetAddress
{
public:
	InetAddress(uint16_t port = 0); //仅仅指定port，ip此时为INADDR——ANY;
	InetAddress(const std::string& ip, uint16_t port);
	InetAddress(const struct sockaddr_in& addr) : addr_(addr)
	{

	}

	std::string toIp() const;
	std::string toIpPort() const;

	const struct sockaddr* getSockAddrInet() const 
	{ 
		return static_cast<const struct sockaddr*>(implicit_cast<const void*>(&addr_)); 
	}

	void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }
	sa_family_t family() const { return addr_.sin_family; }

private:
	struct sockaddr_in addr_;
};


#endif
