#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include <string>
#include <vector>

#include "group.hpp"
using namespace std;

class GroupModel {
 public:
  // 创建群组
  bool createGroup(Group &group);
  // 加入群组
  void addGroup(int userid, int groupid, string role);
  // 查询用户所在群组信息
  vector<Group> queryGroups(int userid);
  // 根据groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
  vector<int> queryGroupsUsers(int userid, int groupid);  // 群聊
};
#endif