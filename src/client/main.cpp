#include "json.hpp"
#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "user.hpp"
#include "group.hpp"
#include "public.hpp"

//记录客户端当前登陆的user信息
User g_currentUser;

//记录user的好友信息
vector<User>g_currentUserFriendList;

//记录user的群组信息
vector<Group>g_currentUserGroupList;

//全部变量控制用户主菜单页面程序，当用户注销时置为false，程序退出主界面，进入登陆页面
bool isMainMenuRunning = false;

//用于读写线程之间的通信
sem_t rwsem;

//记录登陆状态
atomic_bool g_isLoginSuccess{false};

//接受线程，接收服务器的返回消息
void readTaskHandler();

//获得系统的当前时间
string getCurrentTime();

//主菜单页面程序
void mainMenu(int);

//展示登陆成功用户的基本信息
void showCurrentUserData();


//聊天客户端实现，主线程用来作为发送线程，子线程用于接收线程
int main()
{



    return 0;
}

//处理注册的响应逻辑
void doRegResponse(json &responsejs)
{

};
// 处理登录的响应逻辑
void doLoginResponse(json &responsejs)
{

}
// 子线程 - 接收线程
void readTaskHandler(int clientfd)
{

}
// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{

}


// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "loginout" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};


//注册客户端命令的回调函数
unordered_map<string,function<void(int,string)>> commandHandlerMap = 
{
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}
};

// 主聊天页面程序
void mainMenu(int clientfd)
{

}

// "help" command handler
void help(int, string)
{

}

// "addfriend" command handler
void addfriend(int clientfd, string str)
{

}

// "chat" command handler
void chat(int clientfd, string str)
{

}


// "creategroup" command handler  groupname:groupdesc
void creategroup(int clientfd, string str)
{

}

// "addgroup" command handler
void addgroup(int clientfd, string str)
{

}

// "groupchat" command handler   groupid:message
void groupchat(int clientfd, string str)
{

}

// "loginout" command handler
void loginout(int clientfd, string)
{

}
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime()
{
    
}