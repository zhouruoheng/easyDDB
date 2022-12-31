#pragma once

#include <server/marco.hpp>
#include <sqlite3/sqlite3.h>

namespace server {

class DbManager {
public:
    DbManager(std::string site_name);
    ~DbManager();
    sqlite3 *db;

    json getTableInfo(const Table &table);
    json execSelectSql(const std::string &sql, const Table &table);
    void execNotSelectSql(const std::string &sql);
};

};