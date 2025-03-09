#include "offlinemessagemodel.hpp"

#include "db.h"

void offlineMsgModel::insert(int userid, string msg) {
  // 1,sql语句
  char sql[1024] = {0};
  //   sprintf(sql, "insert into offlineMessager(userid, message) values(%d,
  //   '%s')",
  //           userid, msg);  // 字符串要的是，给的是string类型，转一下

  sprintf(sql, "insert into offlineMessage values(%d, '%s')", userid,
          msg.c_str());  // 字符串要的是，给的是string类型，转一下

  MySQL mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}
// 删除用户离线消息
void offlineMsgModel::remove(int userid) {
  // 1,sql语句
  char sql[1024] = {0};
  sprintf(sql, "delete from offlineMessage where userid = %d",
          userid);  // 字符串要的是，给的是string类型，转一下

  MySQL mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}
// 查询用户离线消息
vector<string> offlineMsgModel::query(int userid) {
  // 1,sql语句
  char sql[1024] = {0};
  sprintf(sql, "select message from offlineMessage where userid = %d",
          userid);  // 字符串要的是，给的是string类型，转一下
  vector<string> vec;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      // 把user_id用户的离线消息放入vec中
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        vec.push_back(row[0]);  // 取第一列数据
      }
      mysql_free_result(res);
      return vec;
    }
  }
  return vec;
}