#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    //初始化聊天服务器对象
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    //启动服务
    void start();

private:
    //有连接或者连接断开时执行的回调---由用户设置
    void onConnection(const TcpConnectionPtr &conn);
    //发生读写事件时执行的回调
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buffer,
                   Timestamp receiveTime);

    TcpServer server_; // muduo库服务器对象
    EventLoop *loop_; //事件循环对象---mainloop--用于监听用户的连接
};

#endif