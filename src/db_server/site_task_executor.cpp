#include "site_task_executor.h"


#DEFINE site_id=0




char* db::exe::data_fetch(char* msg){
    char* result = db::mysql::MysqlConnector::select_result(msg);
    return result;
}

char* db::exe::site_execute(char* msg){
    vector<string> msg_vec = split(msg, ",");
    int node_id = stoi(msg_vec[0]);
    int data_site_id = stoi(msg_vec[1]);
    char* sql= msg_vec[2].c_str();
    if data_site_id=site_id:
        return db::mysql::MysqlConnector::select_result(sql);

    else
        char* msg = db::exe::send_site_message(data_site_id, "data", sql);
        db::mysql::MysqlConnector::save_data(msg);//此处需要定义存储mysql数据的函数
        return "ok";
}

char* db::exe::send_site_message(int site_id,char* msg_type, char* msg){
    return "还没写";
}


