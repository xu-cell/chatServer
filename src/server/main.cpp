#include "chatserver.hpp"
#include <iostream>
#include "chatservice.hpp"
#include <signal.h>
using namespace std;
void resetHandler(int)
{
    ChatService::instance()->reset();

}
int main()
{
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr("127.0.0.1",9999);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
    return 0;
}