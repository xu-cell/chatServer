#include "chatserver.hpp"
#include <iostream>
#include "chatservice.hpp"
#include <signal.h>
using namespace std;
void resetHandler(int)
{
    ChatService::instance()->reset();

}
int main(int argc, char **argv)
{
     if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 9999" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
    return 0;
}