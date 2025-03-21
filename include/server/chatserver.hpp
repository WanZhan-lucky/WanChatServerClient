#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer {
 public:
  ChatServer(EventLoop* loop, const InetAddress& listenAddr,
             const string& nameArg);
  void start();

 private:
  void onConnection(const TcpConnectionPtr&);
  void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
  TcpServer _server;
  EventLoop* loop;
};

#endif