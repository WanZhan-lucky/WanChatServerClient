#include "friendmodel.hpp"

#include "db.h"
// 添加好友关系
void FriendModel::insertFriend(int userid, int friendid) {
  char sql[1024] = {0};

  sprintf(sql, "insert into Friend(userid, friendid) values(%d, %d)", userid,
          friendid);
  MySQL mysql;

  if (mysql.connect()) {
    mysql.update(sql);
  }
}

// 返回用户好友列表 friendid, -> name 联合查询
vector<User> FriendModel::queryFriend(int userid) {
  char sql[1024] = {0};
  sprintf(sql,
          "select u.id, u.name, u.state from Friend f inner join User u on "
          "f.friendid = u.id where f.userid = %d",
          userid);
  MySQL mysql;
  vector<User> vec;

  if (mysql.connect()) {
    MYSQL_RES* res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        User user;  // 多个对象
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setState(row[2]);
        vec.push_back(user);
      }
      mysql_free_result(res);
      return vec;
    }
  }
  return vec;
}
