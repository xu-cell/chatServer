#include "usermodel.hpp"

#include  "db.h"
#include <iostream>
using namespace std;
//user表的增加方法
bool  UserModel::insert(User &user)
{
    //组装sql
    char sql[1024] = {0};
    sprintf(sql,"insert into user(name,password,state) values('%s', '%s', '%s')",user.getName().c_str(),user.getPassword().c_str(),user.getState().c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            user.setID(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

//根据用户id查询用户信息
User UserModel::query(int id)
{
    //组装sql
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = '%d'", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res  != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr)
            {
                User user;
                user.setID(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                //释放 res 资源
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}
//更新用户信息
bool UserModel::updateState(User user)
{
    //组装sql
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id=%d",user.getState().c_str(),user.getID());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}