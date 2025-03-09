#include "chatservice.hpp"

#include <muduo/base/Logging.h>

#include <iostream>
#include <string>
#include <vector>

#include "public.hpp"
using namespace muduo;
using namespace std;
ChatService* ChatService::instance() {
  static ChatService service;
  return &service;
}

ChatService::ChatService() {
  // 注册以及Hander回调（基本业务）
  _msgHandlerMap.insert(
      {LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

  _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup,
                                                     this, _1, _2, _3)});
  // 群组事务
  _msgHandlerMap.insert(
      {ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

  _msgHandlerMap.insert(
      {LOGIN_OUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

  // 连接redis服务器
  if (_redis.connect()) {
    // 设置上报消息的回调
    _redis.init_notify_handler(
        std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
  }
}
// 从消息队列订阅消息， redis来调用handleRedisSubscribeMesage消息
void ChatService::handleRedisSubscribeMessage(int userid,
                                              string msg) {  // 通道号，消息
  // json js = json.parse(msg.c_str());
  lock_guard<mutex> lock(_connMutex);
  auto it = _userConnMap.find(userid);
  if (it != _userConnMap.end()) {
    it->second->send(msg);
    return;
  }
  // 存储该用户的离线消息（上报的过程中，下线了）
  _offlinemsgModel.insert(userid, msg);
}
MsgHandler ChatService::getHandler(int msgid) {
  // 日志，错误日志，msgid没有对应事件的回调
  auto it = _msgHandlerMap.find(msgid);
  if (it == _msgHandlerMap.end()) {
    // 返回一个空操作
    return [=](const TcpConnectionPtr&, json&, Timestamp) {
      LOG_ERROR << "msgid: " << msgid << " can't find hander!";
    };
  } else
    return _msgHandlerMap[msgid];
}
void ChatService::clientCloseException(const TcpConnectionPtr& conn) {
  User user;
  {
    lock_guard<mutex> lock(_connMutex);  // 智能解锁
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++) {
      if (it->second == conn) {
        // map表删除用户的连接信息
        user.setId(it->first);
        _userConnMap.erase(it);
        break;
      }
    }
  }

  // 客户端异常退出，也要下线
  _redis.unsubscribe(user.getId());

  if (user.getId() != -1) {
    user.setState("offline");
    _userModel.updateState(user);
    // 更新用户的状态信息  offline
  }
}

void ChatService::reset() {
  // online->offline
  _userModel.resetState();
}
void ChatService::addFriend(const TcpConnectionPtr& conn, json& js,
                            Timestamp time) {
  int userid = js["userid"].get<int>();
  int friendid = js["friendid"].get<int>();
  _friendModel.insertFriend(userid, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr& conn, json& js,
                              Timestamp time) {
  // LOG_ERROR << "Received JSON: " << js.dump();

  int userid = js["id"].get<int>();
  string name = js["groupname"];
  string desc = js["groupdesc"];
  Group group(-1, name, desc);
  if (_groupModel.createGroup(group)) {
    // 存储群组创建人消息
    _groupModel.addGroup(userid, group.getId(),
                         "creator");  // 拿到这个group的id
  }
}
// 加入群组
void ChatService::addGroup(const TcpConnectionPtr& conn, json& js,
                           Timestamp time) {
  int userid = js["id"].get<int>();
  int groupid = js["groupid"].get<int>();
  _groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr& conn, json& js,
                            Timestamp time) {
  int userid = js["id"].get<int>();
  int groupid = js["groupid"].get<int>();
  string message = js["msg"];
  vector<int> useridVec = _groupModel.queryGroupsUsers(userid, groupid);
  lock_guard<mutex> lock(_connMutex);  // 循环，不断加锁解锁，保证线程安全
  for (int groupotherid : useridVec) {
    auto it = _userConnMap.find(groupotherid);  // C++Map不是线程安全的Map
    if (it != _userConnMap.end()) {
      it->second->send(js.dump());
    } else {
      User user = _userModel.query(groupotherid);
      if (user.getState() == "online") {
        _redis.publish(groupotherid, js.dump());
      } else
        _offlinemsgModel.insert(groupotherid, js.dump());
    }
  }
}

void ChatService::oneChat(const TcpConnectionPtr& conn, json& js,
                          Timestamp time) {
  int toid = js["toid"].get<int>();

  {
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(toid);
    if (it != _userConnMap.end()) {
      // 找到了要考虑多线程安全，万一异常退出，在别的地方退出了

      // 服务器主动推送toid的消息
      it->second->send(js.dump());
      return;  // 离开作用域，释放锁
    }
  }

  // 两种情况，1，在其他服务器上，2真的没在线（_userconnMap)只在开启的服务器中
  // 1，数据库查询是否在线
  User user = _userModel.query(toid);
  if (user.getState() == "online") {
    _redis.publish(toid, js.dump());
    return;
  }
  2

      // 离线不在线
      _offlinemsgModel.insert(toid, js.dump());
}
// 业务层登录 ORM object relation map 对象框架映射，业务层操作的都是对象
// DAO层，数据库操作
void ChatService::login(const TcpConnectionPtr& conn, json& js,
                        Timestamp time) {
  LOG_INFO << "do logign service";
  // int id = js["id"];//隐式转换
  int id = js["id"].get<int>();
  string pwd = js["password"];
  User user = _userModel.query(id);
  if (user.getId() == id && user.getPwd() == pwd) {
    if (user.getState() == "online") {
      json reponse;
      reponse["msgid"] = LOGIN_MSG_ACK;
      reponse["errno"] = 2;
      reponse["errmsg"] = "this account is using, input another!";
      conn->send(reponse.dump());
    } else {
      // 登录成功，记录用户连接信息,用户同时登录，涉及多线程安全
      {
        lock_guard<mutex> lock(_connMutex);  // 智能解锁
        _userConnMap.insert({id, conn});     // 用户上线，下线，考虑多线程安全
      }

      // id用户登录成功后，向redis订阅channel()
      _redis.subscribe(id);

      // 登录成功，更新state
      user.setState("online");
      _userModel.updateState(user);  // 并发操作由mysql来操作
      json reponse;
      reponse["msgid"] = LOGIN_MSG_ACK;
      reponse["errno"] = 0;
      reponse["id"] = user.getId();
      reponse["name"] = user.getName();

      // 查询该用户是否有离线消息？
      vector<string> vec = _offlinemsgModel.query(id);
      if (!vec.empty()) {
        reponse["offlinemsg"] = vec;
        // 把该用户的离线消息，把该用户的离线消息删除
        _offlinemsgModel.remove(id);
      }

      // 查询登录用户的好友信息
      vector<User> userVec = _friendModel.queryFriend(id);
      if (!userVec.empty()) {
        // reponse["friends"] = userVec;自定义类型，不可以
        vector<string> usrvec;
        for (User& usr : userVec) {
          json js;
          js["id"] = usr.getId();
          js["name"] = usr.getName();
          js["state"] = usr.getState();
          usrvec.push_back(js.dump());
        }
        reponse["friends"] = usrvec;
      }

      vector<Group> groupuserVec = _groupModel.queryGroups(id);

      if (!groupuserVec.empty()) {
        vector<string> groupV;
        for (Group& group : groupuserVec) {
          json grpjson;
          grpjson["groupid"] = group.getId();
          grpjson["groupname"] = group.getName();
          grpjson["groupdesc"] = group.getDesc();
          vector<string> userV;
          for (GroupUser& user : group.getUsers()) {
            json js;
            js["userid"] = user.getId();
            js["name"] = user.getName();
            js["state"] = user.getState();
            js["role"] = user.getRole();
            userV.push_back(js.dump());
          }
          grpjson["users"] = userV;

          groupV.push_back(grpjson.dump());
        }
        reponse["groups"] = groupV;
      }
      conn->send(reponse.dump());
    }

  } else {
    json reponse;
    reponse["msgid"] = LOGIN_MSG_ACK;
    reponse["errno"] = 1;
    reponse["errmsg"] = "id or password is invalid!";
    conn->send(reponse.dump());
  }
}
void ChatService::loginout(const TcpConnectionPtr& conn, json& js,
                           Timestamp time) {
  int userid = js["id"].get<int>();
  {
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end()) {
      _userConnMap.erase(it);
    }
  }

  // 用户注销，下线，取消订阅通道
  _redis.unsubscribe(userid);

  User user(userid, "", "", "offline");
  _userModel.updateState(user);
}
// 业务层注册 name password
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time) {
  LOG_INFO << "do register service";
  string name = js["name"];
  string pwd = js["password"];
  User user;
  user.setName(name);
  user.setPwd(pwd);
  bool state = _userModel.insert(user);
  if (state) {
    // 注册成功
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 0;
    response["id"] = user.getId();
    conn->send(response.dump());

  } else {
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 1;
    conn->send(response.dump());
  }
}
