#include "mysql_connector.h"
using namespace std;

vector<string> db::mysql::string2list(string inputStr, char split)
{
    stringstream Str(inputStr);
    vector<string> temp;
    string cur;
    while (getline(Str, cur, split))
        temp.push_back(cur);
    return temp;
}

string db::mysql::select_sql(string site, string sql)
{
    int PORT;
    char *UNIX_SOCKET;
    if (site == "ddb1")
        PORT = 3316, UNIX_SOCKET = "/home/mysql1/mysql.sock";
    else
        PORT = 3306, UNIX_SOCKET = "/home/mysql1/mysql.sock";
    MYSQL connection;
    mysql_init(&connection);
    if (mysql_real_connect(&connection, "127.0.0.1", "root", "12345678", "ddb", PORT, UNIX_SOCKET, CLIENT_FOUND_ROWS))
    {
        cout << "Mysql connection success!" << endl;
        int res = mysql_query(&connection, sql.c_str());
        if (res)
        {
            cout << "sql error!" << endl;
            mysql_close(&connection);
            return {};
        }
        MYSQL_RES *result = mysql_store_result(&connection);
        if (result == NULL)
        {
            cout << "The result is empty!" << endl;
            mysql_close(&connection);
            return {};
        }
        MYSQL_ROW row;
        string query_res;
        while (row = mysql_fetch_row(result))
        {
            unsigned long *col = mysql_fetch_lengths(result);
            string temp = "(";
            for (int i = 0; i < mysql_num_fields(result); i++)
                temp = temp + "'" + row[i] + "'" + ",";
            query_res = query_res + temp.substr(0, temp.size() - 1) + "),";
        }
        mysql_close(&connection);
        query_res.pop_back();
        return query_res;
    }
    else
        cout << "Mysql connection fail!" << endl;
    mysql_close(&connection);
    return {};
}

string create_insert_delete_sql(string site, string sql)
{
    int PORT;
    char *UNIX_SOCKET;
    if (site == "ddb1")
        PORT = 3316, UNIX_SOCKET = "/home/mysql1/mysql.sock";
    else
        PORT = 3306, UNIX_SOCKET = "/home/mysql1/mysql.sock";
    MYSQL connection;
    mysql_init(&connection);
    if (mysql_real_connect(&connection, "127.0.0.1", "root", "12345678", "ddb", PORT, UNIX_SOCKET, CLIENT_FOUND_ROWS))
    {
        cout << "Mysql connection success!" << endl;
        int res = mysql_query(&connection, sql.c_str());
        mysql_close(&connection);
        if (res)
            return "FAILED";
        else
            return "OK";
    }
}

// int main()
// {
//     string query_res=db::mysql::select_sql("ddb0","select name,nation from publisher");
//     //cout<<query_res<<endl;
//     cout<<create_insert_delete_sql("ddb0", "create table publisherTemp (name varchar(255), nation varchar(255))")<<endl;
//     cout<<create_insert_delete_sql("ddb0", "insert into publisherTemp values " + query_res )<<endl;
//     cout<<create_insert_delete_sql("ddb0", "drop table publisherTemp")<<endl;
//     //cout<<query_res<<endl;
//     return 0;
// }
