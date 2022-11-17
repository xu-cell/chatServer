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
void readTaskHandler(int clientid);

//获得系统的当前时间
string getCurrentTime();

//主菜单页面程序
void mainMenu(int);

//展示登陆成功用户的基本信息
void showCurrentUserData();


//聊天客户端实现，主线程用来作为发送线程，子线程用于接收线程
int main(int argc,char **argv)
{
    if(argc < 3)
    {
        cerr << "error! please type ./ChatClient 127.0.0.1 9999" << endl;
        exit(1);
    }

    //获取IP地址 + 端口号
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建连接的socket
    int clientid = socket(AF_INET,SOCK_STREAM,0);
    if(clientid == -1)
    {
        cerr << "create client socket error!" << endl;
        exit(1);
    }

    //创建服务器的地址体
    sockaddr_in addrin;
    memset(&addrin,0,sizeof addrin);
    socklen_t addrin_len = sizeof(addrin);
    addrin.sin_port = htons(port);
    addrin.sin_family = AF_INET;
    addrin.sin_addr.s_addr = inet_addr(ip);

    //连接服务器
    if((connect(clientid, (sockaddr*)&addrin,addrin_len)) == -1)
    {
        cerr << "connect server error!" << endl;
        exit(1);
        close(clientid);
    }
    //初始化读写线程的信号量，当子线程处理完登陆和注册信息的时候，就会唤醒主线程
    sem_init(&rwsem,0,0);
    
    //启动子线程-接受线程：负责解析服务端回应的信息
    std::thread readTask(readTaskHandler,clientid);
    readTask.detach();

    //客户端主页面逻辑登陆注册退出.主要为向服务器发送数据
    for(;;)
    {
        //显示首页面菜单
        cout << "========================" << endl;
        cout << "1. Login" << endl;
        cout << "2. Register" << endl;
        cout << "3. Quit" << endl;
        cout << "========================" << endl;
        cout << "Choice:";
        int choice = 0;
        cin >> choice;
        //读取调缓冲区的空格
        cin.get();
        switch(choice)
        {
            //登陆
            case 1:
            {
                int id = 0;
                char password[50] = {0};
                cout << "userid:";
                cin >> id;
                cin.get();
                cout << "userpassword:";
                cin.getline(password,50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = password;
                string request = js.dump();
                
                g_isLoginSuccess = false;
                int len = send(clientid,request.c_str(),strlen(request.c_str())+1,0);
                if(len == -1)
                {
                    cerr << "send login msg error:"  << request << endl;
                }
                //等待信号量，等待子线程处理完登陆的响应消息后，就会通知唤醒这里
                sem_wait(&rwsem);

                if(g_isLoginSuccess)
                {
                    //进入聊天主菜单界面
                    isMainMenuRunning = true;
                    mainMenu(clientid);
                }

            }
            break;
            //注册
            case 2:
            {
                char username[50] = {0};
                char userpassword[50] = {0};
                cout << "username:";
                cin.getline(username,50);
                cout << "userpassword:";
                cin.getline(userpassword,50);

                json js;
                js["msgid"] = REG_MSG;
                js["name"] = username;
                js["password"] = userpassword;
                string request = js.dump();

                int len = send(clientid,request.c_str(),strlen(request.c_str())+1,0);
                if(len == -1)
                {
                    cerr << "sned reg msg error:" << request << endl;
                }
                sem_wait(&rwsem);

            }
            break;
            //退出
            case 3:
            {
                close(clientid);
                sem_destroy(&rwsem);
                exit(0);
            }
            break;

            default:
            {
                cerr << "invalid input!" << endl;
            }
            break;
        }
    }

    return 0;
}

//处理注册的响应逻辑
void doRegResponse(json &responsejs)
{
    if(0 != responsejs["errno"].get<int>())
    {
        cerr << "name is already exist, register errno!" << endl;
    }    
    else
    {
        cout << "name register success, userid is " << responsejs["id"] << ",not forget it!" << endl;
        
    }
};
// 处理登录的响应逻辑
void doLoginResponse(json &responsejs)
{
    if(0 != responsejs["errno"].get<int>())
    {
        cerr << responsejs["errmsg"] << endl;
        g_isLoginSuccess = false;
    }
    else
    {
        g_currentUser.setID(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);
        if(responsejs.contains("friends"))
        {
           g_currentUserFriendList.clear();
           vector<string>vec = responsejs["friends"];
           for(auto &str:vec)
           {
                json js = json::parse(str);
                User user;
                user.setID(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
           }
        }

        if(responsejs.contains("groups"))
        {
            g_currentUserGroupList.clear();

            vector<string>vec1 = responsejs["groups"];
            for(string &groupstr:vec1)
            {
                json js = json::parse(groupstr);
                Group group;
                group.setID(js["id"].get<int>());
                group.setName(js["groupname"]);
                group.setDesc(js["groupdesc"]);
                vector<string>vec2 = js["users"];
                for(string &userstr:vec2)
                {
                    GroupUser user;
                    json jsuser = json::parse(userstr);
                    user.setID(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
            }
        }
        //显示登陆用户的基本信息
        showCurrentUserData();
        // 显示当前用户的离线消息  个人聊天信息或者群组消息
        if (responsejs.contains("offlinemsg"))
        {
            vector<string> vec = responsejs["offlinemsg"];
            for (string &str : vec)
            {
                json js = json::parse(str);
                // time + [id] + name + " said: " + xxx
                if (ONE_CHAT_MSG == js["msgid"].get<int>())
                {
                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                         << " said: " << js["msg"].get<string>() << endl;
                }
                else
                {
                    cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                         << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }

        g_isLoginSuccess = true;
    }
}
// 子线程 - 接收线程
void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd,buffer,1024,0);
        if(len == -1|| 0 == len)
        {
            close(clientfd);
            exit(1);
        }
        // 接收ChatServer转发的数据，反序列化生成json数据对象
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgtype)
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgtype)
        {
            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (LOGIN_MSG_ACK == msgtype)
        {
            doLoginResponse(js); // 处理登录响应的业务逻辑
            sem_post(&rwsem);    // 通知主线程，登录结果处理完成
            continue;
        }

        if (REG_MSG_ACK == msgtype)
        {
            doRegResponse(js);
            sem_post(&rwsem); // 通知主线程，注册结果处理完成
            continue;
        }
    }
}
// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getID() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getID() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getID() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
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
    help();
    char buffer[1024] = {0};
    while(isMainMenuRunning)
    {
        cin.getline(buffer,1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if(idx == -1)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0,idx);
        }
        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        //调用相应命令时的回调函数。mainMenu对修改封闭，后面在增加新功能的时候不需要在修改这个函数
        it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));

    }
}

// "help" command handler
void help(int, string)
{
    cout << "<<< show command list >>>" << endl;
    for(auto &com : commandMap)
    {
        cout << com.first << ":" << com.second << endl;
    }
    cout << endl;
}

// "addfriend" command handler userid friendid
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getID();
    js["friendid"] = friendid;
    
    string request = js.dump();
    int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len == -1)
    {
        cerr << "send addfriend msg error:" << request << endl;
    }
}

// "chat" command handler friend + message
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr << "chat command invalid:" << str << endl;
        return;
    }

    int friendid = atoi((str.substr(0,idx)).c_str());
    string msg = str.substr(idx+1,str.size()-idx);


    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getID();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string request = js.dump();

    int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len == -1)
    {
        cerr << "send chat message error:" << request << endl;
    }
}


// "creategroup" command handler  groupname:groupdesc
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getID();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}

// "addgroup" command handler
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getID();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup msg error -> " << buffer << endl;
    }
}

// "groupchat" command handler   groupid:message
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }
    int groupid = atoi((str.substr(0,idx)).c_str());
    string msg = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getID();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string resquest = js.dump();

    int len = send(clientfd,resquest.c_str(),strlen(resquest.c_str())+1,0);
    if(len == -1)
    {
        cerr << "groupchat msg send error:" << resquest << endl;
    }


}

// "loginout" command handler
void loginout(int clientfd, string)
{
    json js;
    js["msgid"] = LOGIN_OUT;
    js["id"] = g_currentUser.getID();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send loginout msg error -> " << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }   
}
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date,"%d-%02d-%02d %02d:%02d:%02d",(int)ptm->tm_year + 1900,(int)ptm->tm_mon + 1,(int)ptm->tm_mday,(int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
     return std::string(date);

}
