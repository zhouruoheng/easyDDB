#include <mysql/mysql.h> 

using namespace std;


namespace db::mysql{
    class MysqlConnector{
    public:
        MysqlConnector();
        ~MysqlConnector();
        bool Connect(const char* host, const char* user, const char* passwd, const char* db, unsigned int port, const char* unix_socket, unsigned long client_flag);
        bool Query(const char* sql);
        bool StoreResult();
        bool FetchRow();
        bool FreeResult();
        bool Close();
        int GetFieldCount();
        int GetRowCount();
        const char* GetField(int index);
        const char* GetField(const char* field);
        const char* GetRow(int index);
        const char* GetRow(const char* field);
        const char* GetError();
        char* select_result(const char* sql);
        char* save_data(const char* sql);
    private:
        MYSQL* mysql;
        MYSQL_RES* result;
        MYSQL_ROW row;
        int field_count;
        int row_count;
        int current_row;
    };
} // namespace db::mysql
