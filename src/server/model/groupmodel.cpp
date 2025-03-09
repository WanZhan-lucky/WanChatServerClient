
#include "groupmodel.hpp"

#include "db.h"
// 创建群组， AllGroup表
bool GroupModel::createGroup(Group &group) {
  char sql[1024] = {0};

  sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
          group.getName().c_str(), group.getDesc().c_str());
  MySQL mysql;

  if (mysql.connect()) {
    if (mysql.update(sql)) {
      group.setId(mysql_insert_id(mysql.getConnection()));
      return true;
    }
  }
  return false;
}

// 加入群组,GropuUser表
void GroupModel::addGroup(int userid, int groupid, string role) {
  char sql[1024] = {0};

  sprintf(
      sql,
      "insert into GroupUser(groupid, userid, grouprole) values(%d, %d, '%s')",
      groupid, userid, role.c_str());
  MySQL mysql;

  if (mysql.connect()) {
    mysql.update(sql);
  }
}
// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid) {
  char sql[1024] = {0};

  sprintf(sql,
          "select a.id, a.groupname, a.groupdesc from AllGroup a inner join "
          "GroupUser b on a.id = b.groupid where b.userid = %d",
          userid);
  vector<Group> groupVec;

  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      // 查出userid所有的群组信息
      while ((row = mysql_fetch_row(res)) != nullptr) {
        Group group;
        group.setId(atoi(row[0]));
        group.setName(row[1]);
        group.setDesc(row[2]);
        groupVec.push_back(group);
      }
      mysql_free_result(res);
    }

    for (Group &group : groupVec) {  // 查询群组的用户信
      char sqlg[1024] = {0};
      sprintf(sqlg,
              "select a.id,a.name,a.state,b.grouprole from User a \
              inner join GroupUser b on b.userid = a.id where b.groupid=%d",
              group.getId());
      MYSQL_RES *resg = mysql.query(sqlg);
      if (resg != nullptr) {
        MYSQL_ROW rowg;
        while ((rowg = mysql_fetch_row(resg)) != nullptr) {
          GroupUser user;
          user.setId(atoi(rowg[0]));
          user.setName(rowg[1]);
          user.setState(rowg[2]);
          user.setRole(rowg[3]);
          group.getUsers().push_back(user);
        }
      }
      mysql_free_result(resg);
    }
  }
  return groupVec;
}
// 根据groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupsUsers(int userid, int groupid) {
  char sql[1024] = {0};

  sprintf(sql,
          "select userid from GroupUser where groupid = %d and userid != %d",
          groupid, userid);
  vector<int> idVec;
  MySQL mysql;

  if (mysql.connect()) {  // 查询该用户的所有群组
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        idVec.push_back(atoi(row[0]));
      }
      mysql_free_result(res);
    }
  }
  return idVec;
}
