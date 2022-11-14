#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.hpp"
#include <string>
#include <vector>
class GroupModel
{
public:
    //创建群组
    bool createGroup(Group &group);
    //用户加群
    void addGroup(int userid,int groupid,string role);
    //查看用户所在群主信息
    vector<Group> queryGroups(int userid);
    //根据指定的groupid查询群主用户id列表，除userid自己，主要用于用户群聊业务，给群主其他成员群发信息
    vector<int>queryGroupUsers(int userid,int groupid);
};

#endif 