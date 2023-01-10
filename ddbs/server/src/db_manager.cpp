#include <server/db_manager.hpp>

namespace server
{

    DbManager::DbManager(std::string _unix_socket)
        : unix_socket(_unix_socket)
    {
    }

    DbManager::~DbManager()
    {
    }

    json DbManager::execSelectSql(const std::string &sql, const Table &table, std::string site_name)
    {
        int PORT;
        char *UNIX_SOCKET;
        if (site_name == "ddb1")
            PORT = 3316, UNIX_SOCKET = "/home/mysql2/mysql.sock";
        else
            PORT = 3306, UNIX_SOCKET = "/home/mysql1/mysql.sock";
        LOG(INFO) << "(DBManager): execSelectSql: " << sql << std::endl;
        MYSQL connection;
        mysql_init(&connection);
        if (mysql_real_connect(&connection, "127.0.0.1", "root", "12345678", "ddb", PORT, UNIX_SOCKET, CLIENT_FOUND_ROWS))
        {
            int res = mysql_query(&connection, sql.c_str());
            if (res)
            {
                LOG(INFO) << "[select sql error]: " << sql << std::endl;
                mysql_close(&connection);
                return {
                    {"info", nullptr},
                    {"content", nullptr}};
            }
            MYSQL_RES *result = mysql_store_result(&connection);
            if (result == NULL)
            {
                LOG(INFO) << "[select sql error, result empty]: " << sql << std::endl;
                mysql_close(&connection);
                return {
                    {"info", nullptr},
                    {"content", nullptr}};
            }
            json json_result = {
                {"info", "(success)"},
                {"content", json::array()}};
            int nCol = mysql_num_fields(result);
            MYSQL_ROW row;
            std::string query_res;
            while (row = mysql_fetch_row(result))
            {
                json json_row = json::array();
                for (int i = 0; i < nCol; i++)
                    json_row.push_back(std::string(row[i]));
                json_result["content"].push_back(json_row);
            }
            mysql_free_result(result);
            mysql_close(&connection);
            return json_result;
        }
        LOG(INFO) << "[connection error] " << std::endl;
        exit(-1);
    }

    void DbManager::execNotSelectSql(const std::string &sql, std::string site_name)
    {
        int PORT;
        char *UNIX_SOCKET;
        if (site_name == "ddb1")
            PORT = 3316, UNIX_SOCKET = "/home/mysql2/mysql.sock";
        else
            PORT = 3306, UNIX_SOCKET = "/home/mysql1/mysql.sock";
        LOG(INFO) << "(DBManager): execNotSelectSql: " << sql << std::endl;
        MYSQL connection;
        mysql_init(&connection);
        if (mysql_real_connect(&connection, "127.0.0.1", "root", "12345678", "ddb", PORT, UNIX_SOCKET, CLIENT_FOUND_ROWS))
        {
            int res = mysql_query(&connection, sql.c_str());
            if (res)
            {
                LOG(INFO) << "[sql error]: " << sql << std::endl;
                mysql_close(&connection);
                exit(-1);
            }
            mysql_close(&connection);
        }
        LOG(INFO) << "[connection error] " << std::endl;
        exit(-1);
    }

}
