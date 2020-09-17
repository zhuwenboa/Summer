#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<sys/stat.h>
#include<string.h>
#include<pthread.h>
#include<iostream>
#include<sys/mman.h>
#include<stdarg.h>
#include<errno.h>
#include<sys/uio.h>
#include<map>
#include"../../src/Eventloop.h"
#include"../../src/TcpServer.h"
#include"../../src/TcpConnection.h"
#include"../../src/Channel.h"


#define PORT 8888

using namespace Summer;

class Echo
{
public:  
    Echo(Eventloop* loop, InetAddress& listenAddr) : loop_(loop), server_(loop, listenAddr)
    {
        //server_.setConnectionCallback(std::bind(&Echo::oneConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
                std::bind(&Echo::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    }

    void start()
    {
        server_.SetThreadNum(6);
        server_.start();
    }

private:  
    void oneConnection(const TcpConnectionPtr& conn)
    {
        std::cout << "I am connecting success\n";
    }
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf)
    {
        //std::cout << "message = " << buf->retireAllthing() << "\n";
        std::string mes = buf->retireAllthing();
        conn->send(mes, mes.size());
        std::cout << "message = " << mes << "\n";
    }

private:  
    Eventloop* loop_;
    TcpServer server_;
};

int main()
{
    Eventloop loop;    
    InetAddress listenAddr(PORT);
    Echo echo(&loop, listenAddr);
    echo.start();
    loop.loop();
}