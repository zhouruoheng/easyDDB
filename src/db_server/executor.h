//这个文件主要写明对client提供的两种接口的回复
#include "query_optimizer.h"






#include <iostream>

#include "query_optimizer.h"
#include "aug_planner.h"
#include "aug_plan_executor.h"
#include "site_task_executor.h"



namespace db::exe{
    char* sql_execute(const std::string& msg);
    char* config(const std::string& msg);
} // namespace db::exe
