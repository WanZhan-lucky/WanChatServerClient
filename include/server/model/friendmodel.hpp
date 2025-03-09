#ifndef FRIEND_MODEL_H
#define FRIEND_MODEL_H

#include <vector>

#include "user.hpp"
using namespace std;
class FriendModel {
 public:
  // 添加好友关系
  void insertFriend(int userid, int friendid);

  // 返回用户好友列表 friendid, -> name 联合查询
  vector<User> queryFriend(int userid);
};
#endif