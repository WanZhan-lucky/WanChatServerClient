#include "usermodel.hpp"

#include <iostream>

#include "db.h"
using namespace std;
bool UserModel::insert(User& user) {
  // 1,sql语句
  char sql[1024] = {0};
  sprintf(sql,
          "insert into User(name, password, state) values('%s', '%s', '%s')",
          user.getName().c_str(), user.getPwd().c_str(),
          user.getState().c_str());  // 字符串要的是，给的是string类型，转一下

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
      // 获取插入成功的生成的主键id
      user.setId(mysql_insert_id(mysql.getConnection()));
      return true;
    }
  }
  return false;
}

User UserModel::query(int id) {
  // 1,sql语句
  char sql[1024] = {0};
  sprintf(sql, "select * from User where id = %d",
          id);  // 字符串要的是，给的是string类型，转一下

  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* res = mysql.query(sql);
    if (res != nullptr) {
      MYSQL_ROW row = mysql_fetch_row(res);
      if (row != nullptr) {
        User user;
        // atoi和stoi分别对应C和C++语言，各自
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setPwd(row[2]);
        user.setState(row[3]);

        mysql_free_result(res);
        // 每次执行查询都会累积未释放的内存，最终导致程序内存耗尽，可能崩溃或变慢
        return user;
      }
    }
  }
  return User();
}

bool UserModel::updateState(User user) {
  // 1,sql语句
  char sql[1024] = {0};
  sprintf(sql, "update User set state = '%s' where id = %d",
          user.getState().c_str(),
          user.getId());  // 字符串要的是，给的是string类型，转一下

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) return true;
  }
  return false;
  // Mysql自动释放连接资源
}

void UserModel::resetState() {
  // 1,sql语句
  char sql[1024] = "update User set state='offline' where state='online'";

  MySQL mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}