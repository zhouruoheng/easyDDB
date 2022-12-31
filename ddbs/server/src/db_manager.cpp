#include <server/db_manager.hpp>

namespace server {

DbManager::DbManager(std::string site_name) {
    std::string db_name = "database/" + site_name + ".db";
    int rc = sqlite3_open(db_name.c_str(), &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(-1);
    }    
}

DbManager::~DbManager() {
    sqlite3_close(db);
}

json DbManager::getTableInfo(const Table &table) {
    json data;
    sqlite3_stmt *stmt;
    int rc;
    //where rc is an int variable if wondering :/
    std::string sql = "pragma table_info ( " + table + " );";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc!=SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error");
        exit(-1);
    }
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string columnName = (char*)sqlite3_column_text(stmt, 1);
        std::string columnType = (char*)sqlite3_column_text(stmt, 2);
        data[columnName] = columnType;
    }
    sqlite3_finalize(stmt);
    return data;
}


json DbManager::execSelectSql(const std::string &sql, const Table &table) {
    LOG(INFO) << "(DBManager): execSelectSql: " << sql << std::endl;
    json data{
        {"columns", json::array()},
        {"columnTypes", nullptr},
        {"data", json::array()},
    };
    char **azResult = 0;
    int nRow = 0;
    int nColumn = 0;
    char *zErrMsg = 0;
    int rc; 
    rc = sqlite3_get_table(db, sql.c_str(), &azResult, &nRow, &nColumn, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(-1);
    }
    for (int i = 0; i < nColumn; i++)
        data["columns"].push_back(azResult[i]);
    for (int i = 0; i < nRow; i++) {
        json row = json::array();
        for (int j = 0; j < nColumn; j++) {
            int index = i * nColumn + j + nColumn;
            row.push_back(azResult[index]);
        }
        data["data"].push_back(row);
    }
    sqlite3_free_table(azResult);
    data["columnTypes"] = getTableInfo(table);
    return data;
}

void DbManager::execNotSelectSql(const std::string &sql) {
    LOG(INFO) << "(DBManager): execNotSelectSql: " << sql << std::endl;
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(-1);
    }
}

}