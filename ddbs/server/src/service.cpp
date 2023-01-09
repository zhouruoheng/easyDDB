#include <server/service.hpp>

namespace server
{
    
    ServiceImpl::ServiceImpl()
        : localSiteName(FLAGS_site_name), sitesManager(), dbManager(FLAGS_site_name), requestID(0)
    {
        cfg.updateConfig(std::string("res/table_config.json"));
    }

    void ServiceImpl::ClientMsg(
        google::protobuf::RpcController *cntl_base,
        const ClientRequest *request,
        ClientResponse *response,
        google::protobuf::Closure *done)
    {
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl =
            static_cast<brpc::Controller *>(cntl_base);

        // The purpose of following logs is to help you to understand
        // how clients interact with servers more intuitively. You should
        // remove these logs in performance-sensitive servers.
        LOG(INFO) << "(SendClientMsg) Received request[log_id=" << cntl->log_id()
                  << "] from " << cntl->remote_side()
                  << " to " << cntl->local_side()
                  << ": " << request->msg()
                  << " (attached=" << cntl->request_attachment() << ")";

        std::string resp_msg;
        if (request->msg_type() == "sql")
            resp_msg = execSql(request->msg());
        else if (request->msg_type() == "partition")
            resp_msg = execPartition(request->msg());
        else
            resp_msg = "command not found.";

        // Fill response.
        response->set_msg(resp_msg);

        // You can compress the response by setting Controller, but be aware
        // that compression may be costly, evaluate before turning on.
        // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);

        if (FLAGS_echo_attachment)
        {
            // Set attachment which is wired to network directly instead of
            // being serialized into protobuf messages.
            cntl->response_attachment().append(cntl->request_attachment());
        }
    }

    void ServiceImpl::ServerMsg(
        google::protobuf::RpcController *cntl_base,
        const ServerRequest *request,
        ServerResponse *response,
        google::protobuf::Closure *done)
    {
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        brpc::ClosureGuard done_guard(done);

        brpc::Controller *cntl =
            static_cast<brpc::Controller *>(cntl_base);

        // The purpose of following logs is to help you to understand
        // how clients interact with servers more intuitively. You should
        // remove these logs in performance-sensitive servers.
        LOG(INFO) << "(SendClientMsg) Received request[log_id=" << cntl->log_id()
                  << "] from " << cntl->remote_side()
                  << " to " << cntl->local_side()
                  << ": " << request->msg()
                  << " (attached=" << cntl->request_attachment() << ")";

        uint64_t log_id = cntl->log_id();
        if (request->msg_type() == "main")
        {
            json data = json::parse(request->msg());
            json resp_data = json::object({{"info", "(success)"},
                                           {"content", json::object()}});

            auto replaceFn = [](std::string &str)
            {
                for (auto &c : str)
                {
                    if (c == '-')
                        c = '_';
                }
            };

            std::string dataType = data["type"].get<std::string>();
            if (dataType == "updateConfig")
            {
                json config = data["content"];
                cfg.updateConfig(config);
            }
            else if (dataType == "filter")
            {
                std::string newTable = "tmp_" + data["site"].get<std::string>() + "_" + std::to_string(log_id);
                replaceFn(newTable);

                Table table = data["content"]["table"].get<Table>();

                std::string selectList = "";
                if (data["content"]["fields"].is_string())
                    selectList = data["content"]["fields"].get<std::string>();
                else
                {
                    for (auto &field : data["content"]["fields"])
                    {
                        if (selectList.size() > 0)
                            selectList += ", ";
                        selectList += field.get<std::string>();
                    }
                }

                std::string whereClause = "";
                json fieldConditions = data["content"]["fieldConditions"];
                for (auto &fieldCond : fieldConditions)
                {
                    if (whereClause.size() > 0)
                        whereClause += " AND ";
                    whereClause +=
                        table + "." + fieldCond["field"].get<std::string>() + " " + Op2String((hsql::OperatorType)fieldCond["op"].get<int>()) + " " + Value2String(fieldCond["value"]);
                }
                if (whereClause.size() > 0)
                    whereClause = "where " + whereClause;
                std::string drop_sql = "drop table if exists " + newTable + ";";
                std::string selectStat = "select " + selectList + " from " + table + " " + whereClause;
                std::string sql = "create table " + newTable + " as " + selectStat + ";";
                dbManager.execNotSelectSql(drop_sql);
                dbManager.execNotSelectSql(sql);
                resp_data["content"] = json::object({{"table", newTable},
                                                     {"site", localSiteName}});
            }
            else if (dataType == "join")
            {
                json usResult{
                    {"columns", nullptr},
                    {"columnTypes", nullptr},
                    {"data", json::array()}};
                for (auto &rItem : data["content"]["remote"])
                {
                    std::string remoteSite = rItem["site"].get<std::string>();
                    json _request{
                        {"type", "select"},
                        {"site", localSiteName},
                        {"content",
                         {{"fields", "*"},
                          {"table", rItem["table"].get<std::string>()}}}};
                    json _resp = sendMsg(remoteSite, _request.dump());
                    std::string _info = _resp["info"].get<std::string>();
                    if (_info != "(success)")
                    {
                        resp_data = json::object({{"info", _info},
                                                  {"content", nullptr}});
                        goto END;
                    }
                    json sResult = _resp["content"];
                    if (usResult["columns"].is_null())
                        usResult["columns"] = sResult["columns"];
                    if (usResult["columnTypes"].is_null())
                        usResult["columnTypes"] = sResult["columnTypes"];
                    for (auto &rowData : sResult["data"])
                        usResult["data"].push_back(rowData);
                }

                std::string newTable = "tmp_" + data["site"].get<std::string>() + "_" + std::to_string(log_id);
                replaceFn(newTable);
                std::string joinTable = "join_" + data["site"].get<std::string>() + "_" + std::to_string(log_id);
                replaceFn(joinTable);

                { // do create Table
                    std::string typeClause = "";
                    for (auto &col : usResult["columnTypes"].items())
                    {
                        if (typeClause.size() > 0)
                            typeClause += " , ";
                        typeClause += col.key() + " " + col.value().get<std::string>();
                    }
                    std::string drop_sql = "drop table if exists " + joinTable + ";";
                    std::string sql = "create table " + joinTable + "(" + typeClause + ");";
                    dbManager.execNotSelectSql(drop_sql);
                    dbManager.execNotSelectSql(sql);
                }

                { // insert data
                    std::string valueClause = "";
                    for (auto &row : usResult["data"])
                    {
                        std::string rowClause = "";
                        for (auto &item : row)
                        {
                            if (rowClause.size() > 0)
                                rowClause += ",";
                            rowClause += Value2String(item);
                        }
                        if (valueClause.size() > 0)
                            valueClause += ",";
                        valueClause += "(" + rowClause + ")";
                    }
                    std::string sql = "insert into " + joinTable + " values " + valueClause + ";";
                    dbManager.execNotSelectSql(sql);
                }

                { // join
                    std::string leftTable = data["content"]["table"].get<std::string>();
                    std::string &rightTable = joinTable;
                    std::string whereClause = "";
                    for (auto &jItem : data["content"]["joinOn"])
                    {
                        if (whereClause.size() > 0)
                            whereClause += " , ";
                        whereClause +=
                            leftTable + "." + jItem["field0"].get<std::string>() + " = " + rightTable + "." + jItem["field1"].get<std::string>();
                    }
                    if (whereClause.size() > 0)
                        whereClause = " where " + whereClause;
                    std::string drop_sql = "drop table if exists " + newTable + ";";
                    std::string selectStat = " select * from " + leftTable + "," + rightTable + whereClause;
                    std::string sql = "create table " + newTable + " as " + selectStat + ";";
                    dbManager.execNotSelectSql(drop_sql);
                    dbManager.execNotSelectSql(sql);
                }

                { // delete tmp join table
                    std::string sql = "drop table " + joinTable + ";";
                    dbManager.execNotSelectSql(sql);
                }
                resp_data["content"] = json::object({{"table", newTable},
                                                     {"site", localSiteName}});
            }
            else if (dataType == "select")
            {
                Table table = data["content"]["table"].get<Table>();

                std::string selectList = "";
                if (data["content"]["fields"].is_string())
                    selectList = data["content"]["fields"].get<std::string>();
                else
                {
                    for (auto &field : data["content"]["fields"])
                    {
                        if (selectList.size() > 0)
                            selectList += ", ";
                        selectList += field.get<std::string>();
                    }
                }
                std::string sql = "select " + selectList + " from " + table + ";";
                json sResult = dbManager.execSelectSql(sql, table);
                resp_data["content"] = sResult;
            }
            else if (dataType == "insert")
            {
                Table table = data["content"]["table"].get<Table>();
                std::string valueClause = "";
                for (auto &row : data["content"]["data"])
                {
                    std::string rowClause = "";
                    for (auto &item : row)
                    {
                        if (rowClause.size() > 0)
                            rowClause += ",";
                        rowClause += Value2String(item);
                    }
                    if (valueClause.size() > 0)
                        valueClause += ",";
                    valueClause += "(" + rowClause + ")";
                }
                std::string sql = "insert into " + table + " values " + valueClause + ";";
                dbManager.execNotSelectSql(sql);
            }
            else if (dataType == "create")
            {
                Table table = data["content"]["table"].get<Table>();
                std::string typeClause = "";
                for (auto &col : data["content"]["columns"])
                {
                    if (typeClause.size() > 0)
                        typeClause += " , ";
                    typeClause += col["name"].get<std::string>() + " " + Datatype2String((hsql::DataType)col["type"].get<int>());
                }
                std::string sql = "create table " + table + "(" + typeClause + ");";
                dbManager.execNotSelectSql(sql);
            }
            else if (dataType == "drop")
            {
                Table table = data["content"]["table"].get<Table>();
                std::string sql = "drop table " + table + ";";
                dbManager.execNotSelectSql(sql);
            }
        END:
            response->set_msg(resp_data.dump());
        }
        else
        {
            string response_message = deal_with_msg(request->msg_type(), request->msg());
            response->set_msg(response_message);
        }
    }
    std::string ServiceImpl::deal_with_msg(const std::string msg_type, const std::string &msg)
    {
        std::string response_message;
        // if (msg_type == "sql")
        // {
        //     response_message = db::sql_execute(msg, manager); // 面向cli的两个接口之一，用于处理前端发来的sql语句
        // }
        // else if (msg_type == "config")
        // {
        //     response_message = db::config(msg, manager); // 面向cli的两个接口之一，用于处理前端发来的配置信息，这里可以最后一周实现
        // }
        if (msg_type == "data_push")
        {
            response_message = deal_with_data(msg, sitesManager, localSiteName); // 这里是对plan的处理，会写到site_executor.h中
        }
        else if (msg_type == "site_execute")
        {
            response_message = deal_with_order(msg, sitesManager, localSiteName); // 这里是对plan的处理，会写到site_executor.h中
        }
        else
        {
            response_message = "error";
        }

        return response_message;
    }

    std::string ServiceImpl::execSql(const std::string &sql)
    {
        hsql::SQLParserResult result;
        hsql::SQLParser::parse(sql, &result);

        if (result.isValid() && result.size() > 0)
        {
            const hsql::SQLStatement *statement = result.getStatement(0);

            if (statement->isType(hsql::kStmtSelect))
            {      
                vector<metadataTable> Tables = db::opt::getMetadata();
                db::opt::Tree optimized_tree = db::opt::SelectProcess(Tables, result);
                

            }
            else if (statement->isType(hsql::kStmtInsert))
            {
                const auto *insert = static_cast<const hsql::InsertStatement *>(statement);
                std::shared_ptr<server::FragInsertStat> fragInsert;
                try
                {
                    fragInsert = InsertStat(insert).buildFragInsertStatment(cfg);
                }
                catch (const char *msg)
                {
                    return msg;
                }
                for (auto &p : fragInsert->fragData)
                {
                    auto &site = p.first;
                    auto &insertData = p.second;
                    json data{
                        {"type", "insert"},
                        {"site", localSiteName},
                        {"content",
                         {{"table", fragInsert->table},
                          {"data", json::array()}}}};
                    for (auto &row : insertData)
                        data["content"]["data"].push_back(row);
                    json resp = sendMsg(site, data.dump());
                    std::string info = resp["info"].get<std::string>();
                    if (info != "(success)")
                        return info;
                }
                return "(success)";
            }
            else if (statement->isType(hsql::kStmtCreate))
            {
                const auto *create = static_cast<const hsql::CreateStatement *>(statement);
                std::shared_ptr<server::FragCreateStat> fragCreate;
                try
                {
                    fragCreate = CreateStat(create).buildFragCreateStatment(cfg);
                }
                catch (const char *msg)
                {
                    return msg;
                }
                // update Config
                CreateStat createStat(create);
                json columnDefs;
                for (auto &cInfo : createStat.columns)
                {
                    columnDefs.push_back({{"columnName", cInfo.name},
                                          {"columnType", ColumnType2String(cInfo.columnType)}});
                    etcd_set(cInfo.name, ColumnType2String(cInfo.columnType));
                }
                json updateConfig{
                    {"type", "updateConfig"},
                    {"site", localSiteName},
                    {"content",
                     {{"tableInfo",
                       {{createStat.table, columnDefs}}}}}};
                json resp = broadcastMsg(updateConfig.dump());
                std::string info = resp["info"].get<std::string>();
                if (info != "(success)")
                    return info;

                // send create table
                for (auto &site : fragCreate->sites)
                {
                    json data{
                        {"type", "create"},
                        {"site", localSiteName},
                        {"content",
                         {
                             {"table", fragCreate->table},
                             {"columns", json::array()} // {name, type}
                         }}};
                    for (auto &col : fragCreate->columns)
                        data["content"]["columns"].push_back({{"name", col.name},
                                                              {"type", col.columnType}});
                    json resp = sendMsg(site, data.dump());
                    std::string info = resp["info"].get<std::string>();
                    if (info != "(success)")
                        return info;
                }
                return "(success)";
            }
            else if (statement->isType(hsql::kStmtDelete))
            {
            }
            else if (statement->isType(hsql::kStmtDrop))
            {
            }
            // {
            //     const auto *select = static_cast<const hsql::SelectStatement *>(statement);
            //     std::shared_ptr<server::FragSelectStat> fragSelect;
            //     try
            //     {
            //         fragSelect = SelectStat(select).buildFragSelectStatment(cfg);
            //     }
            //     catch (const char *msg)
            //     {
            //         return msg;
            //     }
            //     std::function<std::string(std::shared_ptr<server::FragSelectStat>)> selectFn;
            //     selectFn = [&](std::shared_ptr<server::FragSelectStat> stat) -> std::string
            //     {
            //         if (stat->is_leaf)
            //         {
            //             for (auto &frag : stat->leaf.frags)
            //             {
            //                 json data{
            //                     {"type", "filter"},
            //                     {"site", localSiteName},
            //                     {"content",
            //                      {{"fields", "*"},
            //                       {"table", frag.table},
            //                       {"fieldConditions", json::array()}}}};
            //                 for (auto &cond : stat->leaf.conditions)
            //                 {
            //                     data["content"]["fieldConditions"].push_back(json::object({{"field", cond.field},
            //                                                                                {"op", cond.op},
            //                                                                                {"value", cond.value}}));
            //                 }
            //                 json resp = sendMsg(frag.site, data.dump());
            //                 std::string info = resp["info"].get<std::string>();
            //                 if (info != "(success)")
            //                     return info;
            //                 stat->results.push_back({resp["content"]["table"].get<std::string>(),
            //                                          resp["content"]["site"].get<std::string>()});
            //             }
            //             stat->is_finish = true;
            //         }
            //         else
            //         {
            //             auto left_result = selectFn(stat->node.left);
            //             if (left_result != "(success)")
            //                 return left_result;
            //             auto right_result = selectFn(stat->node.right);
            //             if (right_result != "(success)")
            //                 return right_result;
            //             json joinOn = json::array();
            //             for (auto j : stat->node.joinOn)
            //             {
            //                 joinOn.push_back(json::object({
            //                     {"field0", j.first},
            //                     {"field1", j.second},
            //                 }));
            //             }
            //             for (auto &left_frag : stat->node.left->results)
            //             {
            //                 json data{
            //                     {"type", "join"},
            //                     {"site", localSiteName},
            //                     {"content",
            //                      {{"remote", json::array()},
            //                       {"table", left_frag.table},
            //                       {"joinOn", joinOn}}}};
            //                 for (auto &right_frag : stat->node.right->results)
            //                 {
            //                     data["content"]["remote"].push_back({
            //                         {"table", right_frag.table},
            //                         {"site", right_frag.site},
            //                     });
            //                 }
            //                 json resp = sendMsg(left_frag.site, data.dump());
            //                 std::string info = resp["info"].get<std::string>();
            //                 if (info != "(success)")
            //                     return info;
            //                 stat->results.push_back({resp["content"]["table"].get<std::string>(),
            //                                          resp["content"]["site"].get<std::string>()});
            //             }
            //             stat->is_finish = true;
            //         }
            //         return "(success)";
            //     };
            //     std::function<void(std::shared_ptr<server::FragSelectStat>)> dropFn;
            //     dropFn = [&](std::shared_ptr<server::FragSelectStat> stat) -> void
            //     {
            //         if (!stat->is_leaf)
            //         {
            //             dropFn(stat->node.left);
            //             dropFn(stat->node.right);
            //         }
            //         for (auto &frag : stat->results)
            //         {
            //             json data{
            //                 {"type", "drop"},
            //                 {"site", localSiteName},
            //                 {"content",
            //                  {
            //                      {"table", frag.table},
            //                  }}};
            //             sendMsg(frag.site, data.dump());
            //         }
            //     };
            //     auto info = selectFn(fragSelect);
            //     if (info != "(success)")
            //         return info;
            //     json usResult{
            //         {"columns", nullptr},
            //         {"columnTypes", nullptr},
            //         {"data", json::array()}};
            //     for (auto &frag : fragSelect->results)
            //     {
            //         json data{
            //             {"type", "select"},
            //             {"site", localSiteName},
            //             {"content",
            //              {
            //                  {"fields", json::array()},
            //                  {"table", frag.table},
            //              }}};
            //         for (auto &field : fragSelect->fields)
            //             data["content"]["fields"].push_back(field);
            //         json _resp = sendMsg(frag.site, data.dump());
            //         std::string _info = _resp["info"].get<std::string>();
            //         if (_info != "(success)")
            //             return _info;
            //         json sResult = _resp["content"];
            //         if (usResult["columns"].is_null())
            //             usResult["columns"] = sResult["columns"];
            //         if (usResult["columnTypes"].is_null())
            //             usResult["columnTypes"] = sResult["columnTypes"];
            //         for (auto &rowData : sResult["data"])
            //             usResult["data"].push_back(rowData);
            //     }
            //     dropFn(fragSelect);
            //     return usResult.dump();
        }
        return "[sql is not valid]";
    }

    std::string ServiceImpl::execPartition(const std::string &msg)
    {
        json updateConfig = json::parse(msg);
        json resp = broadcastMsg(updateConfig.dump());
        std::string info = resp["info"].get<std::string>();
        if (info != "(success)")
            return info;
        return "(success)";
    }
}
