#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
#include "user.hpp"
using namespace std;

//用户好友列表的操作方法
class FriendModel
{
public:
    //添加好友
    void insert(int userid,int friendid);

    //返回好友的信息
    vector<User>query(int userid);
};


#endif