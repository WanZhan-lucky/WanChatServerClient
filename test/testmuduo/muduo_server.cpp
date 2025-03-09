/*
TcpServer;
TcpClient;

epoll+线程池
*/
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

#include <functional>
#include <iostream>
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;
/*
 */
class ChatServer {
 public:
  ChatServer(EventLoop* loop,                // recator：事件
             const InetAddress& listenAddr,  // ip+port
             const string& nameArg)          // 线程名字
      : _server(loop, listenAddr, nameArg), _loop(loop) {
    // 给服务器注册用户连接的创建和断开回调
    _server.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, _1));

    // 给服务器注册用户读写事件的回调
    _server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置服务器端的线程数量, 1个IO线程，3个work线程
    _server.setThreadNum(4);
  }

  void start() { _server.start(); }

 private:
  // 专门处理用户的连接创建和断开，epoll, listenfd, accept
  void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
      cout << conn->peerAddress().toIpPort() << "->"
           << conn->localAddress().toIpPort() << "state:online" << endl;
    } else {
      cout << conn->peerAddress().toIpPort() << "->"
           << conn->localAddress().toIpPort() << "state:offline" << endl;
      conn->shutdown();
      //_loop->quit():
    }
  }
  // 专门处理用户的读写时间
  void onMessage(const TcpConnectionPtr& conn,  // 连接
                 Buffer* buffer,                // 缓冲区
                 Timestamp time) {              // 时间信息
    string buf = buffer->retrieveAllAsString();
    cout << "recv data: " << buf << "time: " << time.toString() << endl;
    conn->send(buf);
  }
  TcpServer _server;
  EventLoop* _loop;
};

int main() {
  EventLoop loop;  // epoll;
  InetAddress addr("127.0.0.1", 6000);
  ChatServer server(&loop, addr, "chatServer");

  server.start();  // listenfd epoll_ctl=>epoll
  loop.loop();     // epoll_wait, 阻塞方式等待新用户连接，已连接用户的读写事件
  return 0;
}