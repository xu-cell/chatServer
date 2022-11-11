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
