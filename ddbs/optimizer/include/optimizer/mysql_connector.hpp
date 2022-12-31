#pragma once
#include <bits/stdc++.h>
#include <mysql/mysql.h>
#include <string>
using namespace std;

namespace db::mysql
{
    vector<string> string2list(string inputStr, char split);
    string select_sql(string site, string sql);
    string create_insert_delete_sql(string site, string sql);
}