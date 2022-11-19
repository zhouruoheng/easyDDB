#include <stdio.h>
#include <iostream>
#include <mysql/mysql.h>
#include "mysql_connector.h"

using namespace std;
using namespace db::mysql;
char* db::mysql::MysqlConnector::select_result(const char *sql)
{
    MYSQL mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int field_count;
    int row_count;
    int current_row;
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, "localhost", "root", "123456", "test", 0, NULL, 0))
    {
        cout << "connect error" << endl;
        return NULL;
    }
    if (mysql_query(&mysql, sql))
    {
        cout << "query error" << endl;
        return NULL;
    }
    result = mysql_store_result(&mysql);
    if (result == NULL)
    {
        cout << "store result error" << endl;
        return NULL;
    }
    field_count = mysql_num_fields(result);
    row_count = mysql_num_rows(result);
    current_row = 0;
    row = mysql_fetch_row(result);
    if (row == NULL)
    {
        cout << "fetch row error" << endl;
        return NULL;
    }
    char *res = row[0];
    mysql_free_result(result);
    mysql_close(&mysql);
    return res;
}
char* db::mysql::MysqlConnector::save_data(const char *sql)
{
    MYSQL mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int field_count;
    int row_count;
    int current_row;
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, "localhost", "root", "123456", "test", 0, NULL, 0))
    {
        cout << "connect error" << endl;
        return NULL;
    }
    if (mysql_query(&mysql, sql))
    {
        cout << "query error" << endl;
        return NULL;
    }
    mysql_close(&mysql);
    return NULL;
}