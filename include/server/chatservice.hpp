#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <muduo/net/TcpClient.h>

#include <functional>
#include <mutex>
#include <unordered_map>

#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "json.hpp"
#include "offlinemessagemodel.hpp"
#include "redis.hpp"
#include "usermodel.hpp"
using json = nlohmann::json;
using namespace std;
using namespace muduo;
using namespace muduo::net;
using MsgHandler =  // 处理消息事件的回调类型
    std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

/*
// 定义回调函数
void handleMessage(const TcpConnectionPtr& conn, json& js, Timestamp ts) { }
// 使用别名
MsgHandler handler = handleMessage;
handler(conn, js, ts); // 调用回调
*/
// 业务类
class ChatService {
 public:
  // 获取单例对象的接口函数
  static ChatService* instance();
  // 登录业务
  void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
  // 注册业务
  void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);

  // 一对一聊天
  void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
  // 添加好友
  void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
  //  获取消息对应的处理器
  MsgHandler getHandler(int msgid);
  // redis绑定函数
  void handleRedisSubscribeMessage(int userid, string msg);

  // 处理客户端异常退出
  void clientCloseException(const TcpConnectionPtr& conn);

  // 处理注销业务
  void loginout(const TcpConnectionPtr& conn, json& js, Timestamp time);

  // 服务器异常，业务重置方法
  void reset();

  // 创建群组
  void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
  // 加入群组
  void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
  // 群发消息
  void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

 private:
  ChatService();
  // 存储消息id和处理业务方法
  unordered_map<int, MsgHandler> _msgHandlerMap;

  // 存储在线用户的通信连接
  unordered_map<int, TcpConnectionPtr> _userConnMap;

  // 定义互斥锁，UserConnMap的线程安全
  mutex _connMutex;

  // 数据操作类对象
  UserModel _userModel;
  offlineMsgModel _offlinemsgModel;
  FriendModel _friendModel;
  GroupModel _groupModel;

  // redis操作对象
  Redis _redis;
};

#endif