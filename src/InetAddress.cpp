#include"InetAddress.h"
#include"SockOps.h"

#include<endian.h>
#include<netdb.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>

//任一网关
static const in_addr_t KInaddrAny = INADDR_ANY;

//回环网关
static const in_addr_t KInaddrLoopback = INADDR_LOOPBACK;

using namespace Summer;

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
     memset(&addr_, 0, sizeof(addr_));
     addr_.sin_family = AF_INET;
     in_addr_t ip = loopbackOnly ? KInaddrLoopback : KInaddrAny;
     addr_.sin_addr.s_addr = htobe32(ip);
     addr_.sin_port = htobe16(port);
}

InetAddress::InetAddress(const char* IP, uint16_t port)
{
     addr_.sin_family = AF_INET;
     addr_.sin_port = htobe16(port);
     addr_.sin_addr.s_addr = inet_addr(IP);  
}

