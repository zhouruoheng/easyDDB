#pragma once

#include <server/marco.hpp>
#include <mysql/mysql.h>

namespace server {

class DbManager {
public:
    DbManager(std::string _unix_socket);
    ~DbManager();
   

    json getTableInfo(const Table &table);
    json execSelectSql(const std::string &sql, const Table &table, std::string site_name);
    void execNotSelectSql(const std::string &sql,std::string site_name);

    std::string unix_socket;
};

};