#include "chatserver.hpp"
#include <functional>
#include "json.hpp"
#include "chatservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
//初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : server_(loop, listenAddr, nameArg), loop_(loop)
{
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
    server_.setThreadNum(4);
}
//启动服务
void ChatServer::start()
{
    server_.start();
}

//有连接或者连接断开时执行的回调---由用户设置
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    //用户断开连接
    if(!conn->connected())
    {
        conn->shutdown();
    }
}
//发生读写事件时执行的回调
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp receiveTime)
{
    //获取读缓冲的所有数据
    string buf = buffer->retrieveAllAsString();
    //数据的反序列化
    json js = json::parse(buf);
    //根据业务消息的类型，获取其绑定的处理方法
    auto handler = ChatService::instance()->getmsgHandler(js["msgid"].get<int>());
    //执行业务
    handler(conn,js,receiveTime);

}
