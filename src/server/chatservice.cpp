#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <vector>
#include <muduo/base/Logging.h>
using namespace muduo;
using namespace std;
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}
//登陆业务  id + pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user;
    user = usermodel_.query(id);
    if(user.getID() == id && user.getPassword() == pwd)
    {
        if(user.getState() == "online")
        {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "用户已经登陆，不允许重复登陆";
            conn->send(response.dump());
        }
        else
        {
            //用户登陆成功后，业务层维护连接
            {
                lock_guard<mutex>lock(ConnMutex_);
                userConn_.insert({id,conn});
            }
            

            user.setState("online");
            usermodel_.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["id"] = user.getID();
            response["name"] = user.getName();
            response["errno"] = 0;
            //查询用户是否有离线消息
            vector<string>vec = offlinemsgmodel_.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                //返回离线消费后，清空该用户的所有离线消息
                offlinemsgmodel_.remove(id);
            }

            //返回用户的好友信息
            vector<User> userVec = friendmodel_.query(id);
            if(!userVec.empty())
            {
                vector<string>vec2;
                for(User &user:userVec)
                {
                    json js;
                    js["id"] = user.getID();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        //用户不存在或者密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户不存在或者密码错误";
        conn->send(response.dump());
    }
}
//注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool state = usermodel_.insert(user);
    if(state)
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["id"] = user.getID();
        response["errno"] = 0;
        conn->send(response.dump());
    }    
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump()); 
    }
}
void ChatService::ClientCloseEX(const TcpConnectionPtr &conn)
{
    User user;
    {
        //更新业务层连接信息
        lock_guard<mutex>lock(ConnMutex_);
        for(auto it = userConn_.begin();it != userConn_.end();it++)
        {
            if (it->second == conn)
            {
                user.setID(it->first);
                userConn_.erase(it);
                break;
            }
        }
    }
    //更新用户信息
    if(user.getID() != -1)
    {
        user.setState("offline");
        usermodel_.updateState(user);
    }

}
//注册消息
ChatService::ChatService()
{
    msgHandlerMap_.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    msgHandlerMap_.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});    
    msgHandlerMap_.insert({ONE_CHAT_MSG,std::bind(&ChatService::One_Chat,this,_1,_2,_3)});
    msgHandlerMap_.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});
    msgHandlerMap_.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    msgHandlerMap_.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    msgHandlerMap_.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
    
}
//获取消息对应的处理器
msgHandler ChatService::getmsgHandler(int msgid)
{
    auto it = msgHandlerMap_.find(msgid);
    if(it == msgHandlerMap_.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time){
            LOG_ERROR << "msgid : " << msgid << " can not find the handler! ";
        }; 
    }
    else
    {
        return msgHandlerMap_[msgid];
    }
}
//点对点聊天业务
void ChatService::One_Chat(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(ConnMutex_);
        auto it = userConn_.find(toid);
        //接受方在线
        if(it != userConn_.end())
        {
            it->second->send(js.dump());
            return;
        }
    }

    //接受方不在线
    offlinemsgmodel_.insert(toid,js.dump());
}
void ChatService::reset()
{
    usermodel_.resetState();
}
 //添加好友
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    friendmodel_.insert(userid,friendid);
}
//创建群主
void ChatService::createGroup(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int userid = js["userid"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1,name,desc);
    if(groupmodel_.createGroup(group))
    {
        groupmodel_.addGroup(userid,group.getId(),"creator");
    }

}
//加群
void ChatService::addGroup(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int userid = js["userid"];
    int groupid = js["groupid"];
    groupmodel_.addGroup(userid,groupid,"normal");
}
//群聊
void ChatService::groupChat(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int userid = js["userid"];
    int groupid = js["groupid"];

    vector<int>groupusers = groupmodel_.queryGroupUsers(userid,groupid);
    lock_guard<mutex>lock(ConnMutex_);
    for(auto id:groupusers)
    {
        auto it = userConn_.find(id);
       
        if(it != userConn_.end())
        {
            //转发群消息
            it->second->send(js.dump());
        }
        
        else
        {
            User user = usermodel_.query(id);
            if(user.getState() == "online")
            {
                
            }
            else
            {
                offlinemsgmodel_.insert(id,js.dump());
            }
        }
    }
}



