#include "chatserver.hpp"

#include <functional>
#include <iostream>
#include <string>

#include "chatservice.hpp"
#include "json.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr,
                       const string& nameArg)
    : _server(loop, listenAddr, nameArg) {
  _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
  _server.setMessageCallback(
      std::bind(&ChatServer::onMessage, this, _1, _2, _3));
  _server.setThreadNum(4);
}
void ChatServer::start() { _server.start(); }

// 给服务层通知
void ChatServer::onConnection(const TcpConnectionPtr& conn) {
  if (!conn->connected()) {
    cout << "connectioned is over" << endl;
    ChatService::instance()->clientCloseException(conn);  // 处理异常退出
    conn->shutdown();
  } else {
    cout << "connectioned success" << endl;
  }
}
// 有三个线程循环调用这个事件
void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer,
                           Timestamp time) {
  string buf = buffer->retrieveAllAsString();
  // 数据反序列化
  json js = json::parse(buf);
  // 完全解耦合
  // 通过js['msgid'] 获取-》业务-》conn js time
  auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
  // 回调
  msgHandler(conn, js, time);
}