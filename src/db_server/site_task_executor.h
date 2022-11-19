//这个主要用于执行data请求和order请求，这两个请求都是对site的执行，所以都会调用site_executor.h中的函数
#include "site_task_executor.h"
#inlcude "mysql_connector.h"


namespace db::exe{
    char* data_fetch(char* msg);
    char* site_executor(char* msg);
    char* send_site_message(int site_id,char* msg_type, char* msg);
} // namespace db::site