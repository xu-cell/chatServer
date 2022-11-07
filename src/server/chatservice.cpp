#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <muduo/base/Logging.h>
using namespace muduo;
using namespace std;
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}
//登陆业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    
}
//注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    
}
//注册消息
ChatService::ChatService()
{
    msgHandlerMap_.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    msgHandlerMap_.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});    
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