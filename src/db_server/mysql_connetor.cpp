#include <stdio.h>
#include <iostream>
#include <mysql.h>
using namespace std;
int main(int argc,char *argv[])
{
    MYSQL conn;
    int res;
    mysql_init(&conn);
    //"root":数据库管理员 "123":root密码 "test":数据库的名字
    if(mysql_real_connect(&conn, "localhost","root","123","test",0,NULL,CLIENT_FOUND_ROWS))
    {
        cout << "connect success" << endl;
        res = mysql_query(&conn, "insert into test values('user','123456')");
    if(res)
    {
        printf("error\n");
    }
    else
    {
        printf("OK\n");
    }
    mysql_close(&conn);
    }else
    {
        cout << "connect failed" << endl;
    }
    return 0;
}
