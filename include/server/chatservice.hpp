#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include "usermodel.hpp"
#include "groupmodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "json.hpp"
#include "redis.hpp"
#include <mutex>
using json  = nlohmann::json;
using namespace std;
using namespace muduo;
using namespace muduo::net;
    
//消息到来时事件回调方法类型
using msgHandler = std::function<void(const TcpConnectionPtr& conn,json& js,Timestamp)>;
//单例模式实现服务器业务类 -- 解耦合网络通信与业务处理
class ChatService
{
public:
    static ChatService* instance();
    //登陆业务
    void login(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //注册业务
    void reg(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //客户端异常退出处理
    void ClientCloseEX(const TcpConnectionPtr& conn);
    //点对点聊天
    void One_Chat(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //创建群组的业务
    void createGroup(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //加群
    void addGroup(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //用户注销
    void logOut(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //群聊
    void groupChat(const TcpConnectionPtr& conn,json& js,Timestamp time);
    void handleRedisSubscribeMessage(int id,string str);
    //添加好友
    void addFriend(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //服务器CTRL_C 挂掉之后的处理代码；
    void reset();
    //获取消息对应的处理器
    msgHandler getmsgHandler(int msgid);
private:
    ChatService();
    //存储业务信息id和其对应的业务处理函数
    unordered_map<int,msgHandler> msgHandlerMap_;

    //业务层维护客户的连接
    unordered_map<int,TcpConnectionPtr> userConn_;
    //数据操作类对象
    UserModel usermodel_;
    OfflineMsgModel offlinemsgmodel_;
    FriendModel friendmodel_;
    GroupModel groupmodel_;
    //业务层维护的用户连接，是所有线程共享的，所以我们要保证线程安全
    mutex ConnMutex_;

    //redis对象
    Redis _redis;
};


#endif