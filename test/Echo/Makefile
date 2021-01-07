object = Echo.o ../../src/Acceptor.o ../../src/Channel.o ../../src/Epoll.o ../../src/Eventloop.o  ../../src/InetAddress.o \
	  ../../src/IomultiplexingBase.o ../../src/Socket.o ../../src/SockOps.o ../../src/TcpConnection.o ../../src/TcpServer.o \
	 ../../src/EventloopThread.o ../../src/EventloopThreadPool.o ../../src/TimerTree.o ../../src/Timer.o ../../src/util.o \
	 ../../src/Mysql_pool.o ../../src/Connector.o 
Echo: $(object)
	g++ -o Echo $(object) -lpthread -lmysqlclient -g
Echo.o: Echo.cpp
../../src/Acceptor.o: ../../src/Acceptor.cpp
../../src/Channel.o: ../../src/Channel.cpp
../../src/Epoll.o: ../../src/Epoll.cpp
../../src/Eventloop.o: ../../src/Eventloop.cpp 
../../src/InetAddress.o: ../../src/InetAddress.cpp 
../../src/IomultiplexingBase.o: ../../src/IomultiplexingBase.cpp 
../../src/Socket.o: ../../src/Socket.cpp 
../../src/SockOps.o: ../../src/SockOps.cpp
../../src/TcpConnection.o: ../../src/TcpConnection.cpp 
../../src/TcpServer.o: ../../src/TcpServer.cpp 
../../src/EventloopThread.o: ../../src/EventloopThread.cpp 
../../src/EventloopThreadPool.o: ../../src/EventloopThreadPool.cpp 
../../src/Timer.o : ../../src/Timer.cpp 
../../src/TimerTree.o: ../../src/TimerTree.cpp
../../src/Connector.o: ../../src/Connector.cpp
../../src/Mysql_pool.o: ../../src/Mysql_pool.cpp
../../src/util.o: ../../src/util.cpp
clean: 
	- rm ../../src/*.o