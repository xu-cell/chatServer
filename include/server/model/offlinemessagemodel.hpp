#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include <vector>
#include <string>
using namespace std;
class OfflineMsgModel
{
public:
    //插入离线消息
    void insert(int userid, string msg);
    //查询离线消息，并返回
    vector<string> query(int userid);
    //删除离线消息
    void remove(int userid);
};



#endif