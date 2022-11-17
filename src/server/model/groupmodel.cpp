 #include "groupmodel.hpp"
 #include "db.h"

  bool GroupModel::createGroup(Group &group)
  {
    
    //组装sql
    char sql[1024] = {0};
    sprintf(sql,"insert into allgroup(groupname,groupdesc) values('%s', '%s')",group.getName().c_str(),group.getDesc().c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setID(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
  }
  void GroupModel::addGroup(int userid, int groupid, string role)
  {
    
    //组装sql
    char sql[1024] = {0};
    sprintf(sql,"insert into groupuser values(%d, %d, '%s')",groupid,userid,role.c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
  }
  //查看用户的群组信息
  vector<Group> GroupModel::queryGroups(int userid)
  {
     //组装sql
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup  a inner join \
    groupuser b on a.id = b.groupid where b.userid=%d", userid);
    vector<Group>vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res  != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setID(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                vec.push_back(group);
            }

            mysql_free_result(res);
        }
    }
    //查询群组的用户信息
    for (Group &group : vec)
    {
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a \
        inner join groupuser b on b.userid = a.id where b.groupid=%d",
                group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setID(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }

            mysql_free_result(res);
        }
    }
    return vec;
  }
  //根据指定的groupid查询群主用户id列表，除userid自己，主要用于用户群聊业务，给群主其他成员群发信息
  vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
  {
 //组装sql
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid,userid);
    vector<int>vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res  != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(atoi(row[0]));
            }

            mysql_free_result(res);
            
        }
    }
    return vec;
  }