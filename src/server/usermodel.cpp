#include "usermodel.hpp"

#include  "db.h"
#include <iostream>
using namespace std;
//user表的增加方法
bool  UserModel::insert(User &user)
{
    //组装sql
    char sql[1024] = {0};
    sprintf(sql,"insert into User(name,password,state) values('%s', '%s', '%s')",user.getName().c_str(),user.getPassword().c_str(),user.getState().c_str());
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