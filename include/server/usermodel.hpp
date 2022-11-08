#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"
//user表的操作类
class UserModel
{
public:
    //user表的增加方法
    bool insert(User &user);
};

#endif