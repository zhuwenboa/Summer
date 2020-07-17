#ifndef CALLBACK_H
#define CALLBACK_H
#include<functional>
#include<memory>

namespace Summer
{
class Eventloop;
class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, Buffer*)> MessageCallback;


}//namespace Summer;



#endif