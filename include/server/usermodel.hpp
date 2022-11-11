#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"
//user表的操作类
class UserModel
{
public:
    //user表的增加方法
    bool insert(User &user);
    //根据用户id查询用户信息
    User query(int id);
    //更新用户信息
    bool updateState(User user); 
    //服务器意外结束，将用户状态全部调整为offline
    void resetState();
};

#endif