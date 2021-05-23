#include "InetAddress.h"
#include "SocketsOps.h"

#include <netdb.h>
#include <netinet/in.h>
#include<string>
using std::string;

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

InetAddress::InetAddress(uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = htobe32(kInaddrAny);
	addr_.sin_port = htobe16(port);
}

InetAddress::InetAddress(const string& ip, uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_port = htobe16(port);
	inet_pton(AF_INET, ip.data(), (void*)&(addr_.sin_addr));
}


string InetAddress::toIp() const
{
	char buf[32];
	sockets::toIp(buf, sizeof buf, getSockAddrInet());
	return buf;
}

string InetAddress::toIpPort() const
{
	char buf[32];
	sockets::toIpPort(buf, sizeof buf, getSockAddrInet());
	return buf;
}
