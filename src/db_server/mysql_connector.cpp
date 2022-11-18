#include <stdio.h>
#include <iostream>
#include <mysql.h>
using namespace std;
int main(int argc,char *argv[])
{
    MYSQL conn;
    int res;
    mysql_init(&conn);
    if(mysql_real_connect(&conn, "127.0.0.1","root","12345678","test",3316,"/home/mysql2/mysql.sock",CLIENT_FOUND_ROWS))
    {
        cout << "connect success" << endl;
        res = mysql_query(&conn, "create table test (id INT)");
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
