#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"

// 多一个角色role角色，群里面要呈现role，直接继承而来
class GroupUser : public User {
 public:
  void setRole(string role) { this->role = role; };
  string getRole() { return this->role; }
  // 派生变量
 private:
  string role;
};

#endif