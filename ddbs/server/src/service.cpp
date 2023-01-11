#include <server/service.hpp>
#include <optimizer/metadata.hpp>

namespace server
{
    void *sendAsyMsg(void *args)
    {
        cout << "sendAsyMsg" << endl;
        Aargs *p = (Aargs *)args;
        std::string msg = p->msg;
        std::string sitename = p->siteName;
        // std::cout << msg << std::endl;
        std::cout << sitename << std::endl;
        ServiceImpl *service = p->service;
        brpc::Channel *channel = service->sitesManager.getChannel(sitename);
        db::Service_Stub stub(channel);
        db::ServerRequest request;
        db::ServerResponse response;
        brpc::Controller cntl;
        cout << "sendprepare" << endl;
        request.set_msg(msg);

        cntl.set_log_id(service->sitesManager.requestID++); // set by user
        // Set attachment which is wired to network directly instead of
        // being serialized into protobuf messages.
        cntl.request_attachment().append("attachment");

        // Because `done'(last parameter) is NULL, this function waits until
        // the response comes back or error occurs(including timedout).

        stub.ServerMsg(&cntl, &request, &response, NULL);
        if (!cntl.Failed())
        {
            LOG(INFO) << "(success) Received response from " << cntl.remote_side()
                      << " to " << cntl.local_side()
                      << ": " <<  " (attached="
                      << cntl.response_attachment() << ")"
                      << " latency=" << cntl.latency_us() << "us" << std::endl;
            json data = json::parse(response.msg());
            // std::cout << "data parsed suss" << std::endl;
            Table table = data["content"]["table"].get<Table>();
            std::cout << table << std::endl;
            std::string typeClause = "";
            for (auto &col : data["content"]["columns"])
            {
                if (typeClause.size() > 0)
                    typeClause += " , ";
                typeClause += col["name"].get<std::string>() + " " + col["type"].get<std::string>();
            }
            std::string drop_sql = "drop table if exists " + table + ";";
            service->dbManager.execNotSelectSql(drop_sql, service->localSiteName);
            std::string sql = "create table " + table + "(" + typeClause + ");";
            std::cout << sql << std::endl;
            service->dbManager.execNotSelectSql(sql, service->localSiteName);
            std::string valueClause = "";
            if (data["content"]["data"].size() == 0)
            {
                delete args;
                return nullptr;
            }
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
            sql = "insert into " + table + " values " + valueClause + ";";
            service->dbManager.execNotSelectSql(sql, service->localSiteName);
        }
        else
        {
            LOG(ERROR) << cntl.ErrorText() << std::endl;
        }
        delete args;
        return nullptr;
    }
    // void ServiceImpl::sendAsyMsg(void* data)
    // {
    //     std::pair<std::string, std::string> *p = (std::pair<std::string, std::string>*)data;
    //     std::string sitename=p->first;
    //     std::string msg=p->second;
    //     brpc::Channel *channel = sitesManager.getChannel(sitename);
    //     db::Service_Stub stub(channel);
    //     db::ServerRequest request;
    //     db::ServerResponse response;
    //     brpc::Controller cntl;
    //     request.set_msg(msg);

    //     cntl.set_log_id(sitesManager.requestID++); // set by user
    //     // Set attachment which is wired to network directly instead of
    //     // being serialized into protobuf messages.
    //     cntl.request_attachment().append("attachment");

    //     // Because `done'(last parameter) is NULL, this function waits until
    //     // the response comes back or error occurs(including timedout).
    //     stub.ServerMsg(&cntl, &request, &response, NULL);
    //     if (!cntl.Failed())
    //     {
    //         LOG(INFO) << "(success) Received response from " << cntl.remote_side()
    //                   << " to " << cntl.local_side()
    //                   << ": " <<  " (attached="
    //                   << cntl.response_attachment() << ")"
    //                   << " latency=" << cntl.latency_us() << "us" << std::endl;
    //         json data = json::parse(response.msg());
    //         Table table = data["content"]["content"]["table"].get<Table>();
    //         std::string typeClause = "";
    //         for (auto &col : data["content"]["columns"])
    //         {
    //             if (typeClause.size() > 0)
    //                 typeClause += " , ";
    //             typeClause += col["name"].get<std::string>() + " " + Datatype2String((hsql::DataType)col["type"].get<int>());
    //         }
    //         std::string sql = "create table " + table + "(" + typeClause + ");";
    //         dbManager.execNotSelectSql(sql);
    //         std::string valueClause = "";
    //         for (auto &row : data["content"]["data"])
    //         {
    //             std::string rowClause = "";
    //             for (auto &item : row)
    //             {
    //                 if (rowClause.size() > 0)
    //                     rowClause += ",";
    //                 rowClause += Value2String(item);
    //             }
    //             if (valueClause.size() > 0)
    //                 valueClause += ",";
    //             valueClause += "(" + rowClause + ")";
    //         }
    //         sql = "insert into " + table + " values " + valueClause + ";";
    //         dbManager.execNotSelectSql(sql);
    //     }
    //     else
    //     {
    //         std::stringstream ss;
    //         ss << cntl.ErrorText();
    //         LOG(ERROR) << ss.str();
    //     }
    //     delete &cntl;
    //     delete &response;
    // }

    AugmentedPlan ServiceImpl::build_augmented_plan(db::opt::Tree query_tree)
    {
        AugmentedPlan plan;
        // get the total num of query_tree.tr
        plan.node_num = query_tree.tr.size();
        cout << "node_num:" << plan.node_num << endl;
        // build the tree from bottom to top
        vector<int> has_built;
        for (int i = 0; i < plan.node_num; i++)
        {
            if (query_tree.tr[i].type == "Fragment")
            {
                has_built.push_back(i);
                plan.augplannodes.push_back(transfer_to_AugNode(query_tree, i, plan));
            }
            for (auto &has_built_node : has_built)
            {
                cout << has_built_node << " ";
            }
        }
        while (has_built.size() < query_tree.tr.size())
        {
            for (int i = 0; i < plan.node_num; i++)
            {
                for (auto &child : query_tree.tr[i].child)
                {
                    if (find(has_built.begin(), has_built.end(), child) == has_built.end())
                    {
                        break;
                    }
                }
                if (find(has_built.begin(), has_built.end(), i) == has_built.end())
                {
                    plan.augplannodes.push_back(transfer_to_AugNode(query_tree, i, plan));
                    has_built.push_back(i);
                }
            }
        }
        // for (int i = 0; i < plan.node_num; i++)
        // {
        //     plan.augplannodes.push_back(transfer_to_AugNode(query_tree, i, plan));
        // }
        return plan;
    }
    AugPlanNode ServiceImpl::transfer_to_AugNode(db::opt::Tree &query_tree, int i, AugmentedPlan &plan)
    {
        cout << "start to build node:" << i << endl;
        AugPlanNode node;

        node.node_id = i;
        node.parent_id = query_tree.tr[i].parent;
        node.execute_site = "ddb" + std::to_string(query_tree.tr[i].site);
        std::string select_what = "";
        std::string from_what = "";
        std::string where_what = "";
        std::string sql = "";
        std::cout << "node type:" << query_tree.tr[i].type << std::endl;
        if (query_tree.tr[i].type == "Fragment")
        {
            if (query_tree.tr[i].projection.size() == 0)
            {
                std::cout << "select *" << std::endl;
                for (auto &col : query_tree.tr[i].attr)
                {
                    std::cout << col << std::endl;
                    // replace . to _
                    col = col.replace(col.find("."), 1, "_");
                    std::cout << col << "col" << std::endl;
                    select_what += col + ",";
                    columnzy column(col, columnname_to_type(col));
                    cout << "column name:" << column.name << endl;
                    cout << "column type:" << column.type << endl;
                    node.columns.push_back(column);
                    std::cout << "node type:" << query_tree.tr[i].type << std::endl;
                    std::cout << select_what << std::endl;
                }
                select_what = select_what.substr(0, select_what.size() - 1);
                std::cout << "select_what:" << select_what << std::endl;
            }
            else
            {
                for (auto &col : query_tree.tr[i].projection)
                {
                    // replace . to _
                    col = col.replace(col.find("."), 1, "_");
                    select_what += col + ",";
                    columnzy column(col, columnname_to_type(col));
                    node.columns.push_back(column);
                }
                select_what = select_what.substr(0, select_what.size() - 1);
            }
            from_what = query_tree.tr[i].fname.first;
            std::cout << "from_what:" << from_what << std::endl;
            for (auto &onecondition : query_tree.tr[i].select)
            {
                if (onecondition.type == "string")
                {
                    where_what += onecondition.table + "_" + onecondition.attr + onecondition.opt + "\'" + onecondition.sval + "\'";
                }
                else
                {
                    where_what += onecondition.table + "_" + onecondition.attr + onecondition.opt + std::to_string(onecondition.ival);
                }
                // if it is not the last one,add and
                where_what += " and ";
            }
            if (where_what.size() > 5)
            {
                where_what = where_what.substr(0, where_what.size() - 5);
                std::cout << "select_what:" << select_what << std::endl;
                sql = "select " + select_what + " from " + from_what + " where " + where_what;
            }
            else
            {
                sql = "select " + select_what + " from " + from_what;
            }
        }
        else if (query_tree.tr[i].type == "Union")
        {
            std::vector<int> all_child = query_tree.tr[i].child;
            int firstchild=all_child[0];
            for (auto &col : query_tree.tr[i].attr)
            {
                // replace . to _
                col = col.replace(col.find("."), 1, "_");
                columnzy column(col, columnname_to_type(col));
                node.columns.push_back(column);
                select_what+=col+",";
            }
            select_what = select_what.substr(0, select_what.size() - 1);
            for (auto &child : all_child)
            {
                from_what = "Node" + std::to_string(child);
                if (child != all_child.back())
                    sql += "select " + select_what + " from " + from_what + " union all ";
                else
                {
                    sql += "select " + select_what + " from " + from_what;
                }
            }
        }
        else if (query_tree.tr[i].type == "Join")
        {
            if (query_tree.tr[i].projection.size() == 0)
            {
                for (auto &col : query_tree.tr[i].attr)
                {
                    // replace . to _
                    col = col.replace(col.find("."), 1, "_");
                    columnzy column(col, columnname_to_type(col));
                    node.columns.push_back(column);
                    col = col.replace(col.find("_"), 1, ".");
                    select_what += find_table_name(col, query_tree.tr[i].child[0], query_tree.tr[i].child[1], query_tree) + "." + col.replace(col.find("."), 1, "_") + ",";
                }
                select_what = select_what.substr(0, select_what.size() - 1);
            }
            else
            {
                for (auto &col : query_tree.tr[i].projection)
                {
                    // replace . to _
                    col = col.replace(col.find("."), 1, "_");
                    columnzy column(col, columnname_to_type(col));
                    node.columns.push_back(column);
                    col = col.replace(col.find("_"), 1, ".");
                    select_what += find_table_name(col, query_tree.tr[i].child[0], query_tree.tr[i].child[1], query_tree) + "." + col.replace(col.find("."), 1, "_") + ",";
                }
                select_what = select_what.substr(0, select_what.size() - 1);
            }
            from_what = "Node" + std::to_string(query_tree.tr[i].child[0]) + " inner join Node" + std::to_string(query_tree.tr[i].child[1]);
            where_what = "Node" + std::to_string(query_tree.tr[i].child[0]) + "." + query_tree.tr[i].join.ltab + "_" + query_tree.tr[i].join.lcol + "=Node" + std::to_string(query_tree.tr[i].child[1]) + "." + query_tree.tr[i].join.rtab + "_" + query_tree.tr[i].join.rcol;
            sql = "select " + select_what + " from " + from_what + " on " + where_what;
        }
        else
        {
            sql = "";
        }
        node.sql = sql;
        std::cout << "sql:" << sql << std::endl;
        std::cout << "finish building node:" << i << std::endl;
        return node;
    }
    std::string ServiceImpl::find_table_name(std::string col, int left, int right, Tree &query_tree)
    {
        std::cout << "find" << col << std::endl;
        std::vector<string> left_attr;
        std::vector<string> right_attr;
        if (query_tree.tr[left].projection.size() == 0)
        {
            left_attr = query_tree.tr[left].attr;
        }
        else
        {
            left_attr = query_tree.tr[left].projection;
        }
        if (query_tree.tr[right].projection.size() == 0)
        {
            right_attr = query_tree.tr[right].attr;
        }
        else
        {
            right_attr = query_tree.tr[right].projection;
        }
        for (auto &left_col : left_attr)
        {
            if (left_col == col)
            {
                return "Node" + std::to_string(left);
            }
        }
        for (auto &right_col : right_attr)
        {
            if (right_col == col)
            {
                return "Node" + std::to_string(right);
            }
        }
        // error
        std::cout << "error : cannot find table name" << col << endl;
        return "";
    }
    std::string ServiceImpl::columnname_to_type(std::string name)
    {
        // split name to tablename and columnname
        std::string tablename = name.substr(0, name.find("_"));
        std::string columnname = name.substr(name.find("_") + 1);
        std::cout << "tablename:" << tablename << std::endl;
        std::cout << "columnname:" << columnname << std::endl;
        vector<ColumnDef> types = cfg.tableInfo[tablename];
        for (auto &type : types)
        {
            std::cout << type.name << std::endl;
            if (type.name == columnname)
            {
                std::cout << Datatype2String(type.columnType) << std::endl;
                return Datatype2String(type.columnType);
            }
        }
        return "";
    }
    // const brpc::CallId ServiceImpl::sendMsgAsync(std::string siteName, std::string data)
    // {
    //     brpc::Channel *channel = sitesManager.getChannel(siteName);
    //     db::Service_Stub stub(channel);
    //     db::ServerRequest request;
    //     db::ServerResponse *response;
    //     brpc::Controller *cntl;
    //     request.set_msg(data);

    //     cntl->set_log_id(requestID++); // set by user
    //     // Set attachment which is wired to network directly instead of
    //     // being serialized into protobuf messages.
    //     cntl->request_attachment().append("attachment");
    //     const brpc::CallId cid = cntl->call_id();
    //     google::protobuf::Closure *done = brpc::NewCallback(onMsgResponse, cntl, response);
    //     stub.ServerMsg(cntl, &request, response, done);
    //     return cid;
    // }
    // void ServiceImpl::onMsgResponse(brpc::Controller *cntl, db::ServerResponse *response)
    // {
    //     if (!cntl->Failed())
    //     {
    //         LOG(INFO) << "(success) Received response from " << cntl->remote_side()
    //                   << " to " << cntl->local_side()
    //                   << ": " << response->msg() << " (attached="
    //                   << cntl->response_attachment() << ")"
    //                   << " latency=" << cntl->latency_us() << "us" << std::endl;
    //         json data = json::parse(response->msg());
    //         Table table = data["content"]["content"]["table"].get<Table>();
    //         std::string typeClause = "";
    //         for (auto &col : data["content"]["columns"])
    //         {
    //             if (typeClause.size() > 0)
    //                 typeClause += " , ";
    //             typeClause += col["name"].get<std::string>() + " " + Datatype2String((hsql::DataType)col["type"].get<int>());
    //         }
    //         std::string sql = "create table " + table + "(" + typeClause + ");";
    //         dbManager.execNotSelectSql(sql);
    //         std::string valueClause = "";
    //         for (auto &row : data["content"]["data"])
    //         {
    //             std::string rowClause = "";
    //             for (auto &item : row)
    //             {
    //                 if (rowClause.size() > 0)
    //                     rowClause += ",";
    //                 rowClause += Value2String(item);
    //             }
    //             if (valueClause.size() > 0)
    //                 valueClause += ",";
    //             valueClause += "(" + rowClause + ")";
    //         }
    //         sql = "insert into " + table + " values " + valueClause + ";";
    //         dbManager.execNotSelectSql(sql);
    //     }
    //     else
    //     {
    //         std::stringstream ss;
    //         ss << cntl->ErrorText();
    //         LOG(ERROR) << ss.str();
    //     }
    //     delete cntl;
    //     delete response;
    // }

    ServiceImpl::ServiceImpl()
        : localSiteName(FLAGS_site_name), sitesManager(), dbManager(FLAGS_site_name), requestID(0)
    {
        cfg.loadConfig(std::string("res/table_config.json"));
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
                  << ": " 
                  << " (attached=" << cntl->request_attachment() << ")";

        std::string resp_msg;
        if (request->msg_type() == "sql")
            resp_msg = execSql(request->msg());
        else if (request->msg_type() == "partition")
            resp_msg = execPartition(request->msg());
        else if (request->msg_type() == "load")
            resp_msg = execLoad(request->msg());
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
                  << ": " 
                  << " (attached=" << cntl->request_attachment() << ")";

        uint64_t log_id = cntl->log_id();
        json data = json::parse(request->msg());
        json resp_data = json::object({{"info", "(success)"},
                                       {"content", json::object()},
                                       {"size", 0}});

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
            dbManager.execNotSelectSql(drop_sql, localSiteName);
            dbManager.execNotSelectSql(sql, localSiteName);
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
                json _resp = sitesManager.sendMsg(remoteSite, _request.dump());
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
                dbManager.execNotSelectSql(drop_sql, localSiteName);
                dbManager.execNotSelectSql(sql, localSiteName);
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
                dbManager.execNotSelectSql(sql, localSiteName);
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
                dbManager.execNotSelectSql(drop_sql, localSiteName);
                dbManager.execNotSelectSql(sql, localSiteName);
            }

            { // delete tmp join table
                std::string sql = "drop table " + joinTable + ";";
                dbManager.execNotSelectSql(sql, localSiteName);
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
            json sResult = dbManager.execSelectSql(sql, table, localSiteName);
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
            dbManager.execNotSelectSql(sql, localSiteName);
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
            dbManager.execNotSelectSql(sql, localSiteName);
        }
        else if (dataType == "drop")
        {
            Table table = data["content"]["table"].get<Table>();
            std::string sql = "drop table " + table + ";";
            dbManager.execNotSelectSql(sql, localSiteName);
        }
        else if (dataType == "delete")
        {
            Table table = data["content"]["table"].get<Table>();
            std::string sql = "delete from " + table + ";";
            dbManager.execNotSelectSql(sql, localSiteName);
        }
        else if (dataType == "start_execute")
        {
            std::cout << "execute" << data["content"]["execute_node"].get<int>() << std::endl;
            int execute_node = data["content"]["execute_node"].get<int>();
            AugmentedPlan plan = AugmentedPlan(data["content"]["plan"].get<json>());
            std::cout << "plan received" << std::endl;
            for (auto &node : plan.augplannodes)
            {
                std::cout << node.node_id << std::endl;
            }
            std::cout << "all node printed" << std::endl;
            vector<int> children_list = plan.get_children_list(execute_node);
            // print children_list
            for (auto &child : children_list)
            {
                std::cout << child << std::endl;
            }
            vector<bthread_t> tids;

            // send start_execute to children
            // for (auto &child : children_list)
            // {
            //     json child_data{
            //         {"type", "start_execute"},
            //         {"site", localSiteName},
            //         {"content",
            //          {{"execute_node", std::to_string(child)}, {"plan", plan.to_json()}}}};
            //     std::string child_msg = child_data.dump();
            //     cids.push_back(sendMsgAsync(plan.augplannodes[child].execute_site, child_msg));
            // }
            // rank the children list
            for (auto &child : children_list)
            {
                json child_data{
                    {"type", "start_execute"},
                    {"site", localSiteName},
                    {"content",
                     {{"execute_node", child}, {"plan", plan.to_json()}}}};
                std::string child_msg = child_data.dump();
                // std::cout << child_msg << std::endl;
                // json data = sitesManager.sendMsg(plan.augplannodes[child].execute_site, child_msg);
                // Table table = data["content"]["content"]["table"].get<Table>();
                // std::string typeClause = "";
                // for (auto &col : data["content"]["columns"])
                // {
                //     if (typeClause.size() > 0)
                //         typeClause += " , ";
                //     typeClause += col["name"].get<std::string>() + " " + Datatype2String((hsql::DataType)col["type"].get<int>());
                // }
                // std::string sql = "create table " + table + "(" + typeClause + ");";
                // dbManager.execNotSelectSql(sql);
                // std::string valueClause = "";
                // for (auto &row : data["content"]["data"])
                // {
                //     std::string rowClause = "";
                //     for (auto &item : row)
                //     {
                //         if (rowClause.size() > 0)
                //             rowClause += ",";
                //         rowClause += Value2String(item);
                //     }
                //     if (valueClause.size() > 0)
                //         valueClause += ",";
                //     valueClause += "(" + rowClause + ")";
                // }
                // sql = "insert into " + table + " values " + valueClause + ";";
                // dbManager.execNotSelectSql(sql);
                // open a bthread
                bthread_t tid;
                // Aargs
                Aargs *args = new Aargs(plan.augplannodes[child].execute_site, child_msg, this);
                std::cout << (bthread_start_background(&tid, NULL, *sendAsyMsg, args));
                cout << "tid:" << tid << endl;
                tids.push_back(tid);
            }
            for (auto &tid : tids)
            {
                bthread_join(tid, NULL);
                std::cout << "join" << tid << std::endl;
            }
            json data{
                {"table", "Node" + std::to_string(execute_node)},
                {"data", json::array()},
                {"columns", json::array()} // {name, type}
            };
            vector<int> isint;
            for (auto &col : plan.augplannodes[execute_node].columns)
            {
                data["columns"].push_back({{"name", col.name},
                                           {"type", col.type}});
                if (col.type == "integer")
                {
                    isint.push_back(1);
                }
                else
                {
                    isint.push_back(0);
                }
            }
            std::string sql = plan.augplannodes[execute_node].sql;
            Table table = "Node" + std::to_string(execute_node);
            for (auto &isinti : isint)
            {
                std::cout << isinti << std::endl;
            }
            json data2 = dbManager.execSelectSqlzy(sql, table, localSiteName, isint);
            data["data"] = data2["content"];
            for (auto &child : children_list)
            {
                std::string sql = "drop table if exists Node" + std::to_string(child) + ";";
                dbManager.execNotSelectSql(sql, localSiteName);
            }

            resp_data["content"] = data;
            resp_data["size"] = data2["size"];
        }
    END:
        response->set_msg(resp_data.dump());
    }
    // std::string ServiceImpl::deal_with_msg(const std::string msg_type, const std::string &msg)
    // {
    //     std::string response_message;
    //     // if (msg_type == "sql")
    //     // {
    //     //     response_message = db::sql_execute(msg, manager); // 面向cli的两个接口之一，用于处理前端发来的sql语句
    //     // }
    //     // else if (msg_type == "config")
    //     // {
    //     //     response_message = db::config(msg, manager); // 面向cli的两个接口之一，用于处理前端发来的配置信息，这里可以最后一周实现
    //     // }
    //     if (msg_type == "data_push")
    //     {
    //         response_message = deal_with_data(msg, sitesManager, localSiteName); // 这里是对plan的处理，会写到site_executor.h中
    //     }
    //     else if (msg_type == "site_execute")
    //     {
    //         response_message = deal_with_order(msg, sitesManager, localSiteName); // 这里是对plan的处理，会写到site_executor.h中
    //     }
    //     else
    //     {
    //         response_message = "error";
    //     }

    //     return response_message;
    // }

    std::string ServiceImpl::execSql(const std::string &sql)
    {
        hsql::SQLParserResult result;
        hsql::SQLParser::parse(sql, &result);

        if (result.isValid() && result.size() > 0)
        {
            const hsql::SQLStatement *statement = result.getStatement(0);

            if (statement->isType(hsql::kStmtSelect))
            {
                cout << "receive select sql" << endl;
                std::vector<metadataTable> Tables = db::opt::getMetadata();
                cout << "get metadata" << endl;
                db::opt::Tree optimized_tree = db::opt::SelectProcess(Tables, &result);
                // cout << "get optimized tree" << endl;
                AugmentedPlan plan = build_augmented_plan(optimized_tree);
                // read planjson from res/test.json
                // std::ifstream i("res/test.json");
                // json planjson;
                // i >> planjson;
                // AugmentedPlan plan(planjson);
                json planjson=plan.to_json();
                std::ofstream o("res/test.json");
                o << std::setw(4) << planjson << std::endl;
                cout << "get augmented plan" << endl;
                int root_id = plan.find_root_id();
                std::string root_site = plan.augplannodes[root_id].execute_site;
                json data{
                    {"type", "start_execute"},
                    {"site", localSiteName},
                    {"content",
                     {{"execute_node", root_id}, {"plan", plan.to_json()}}}};
                cout << "send start_execute" << endl;
                json response = sitesManager.sendMsg(root_site, data.dump());
                cout << "get response" << endl;
                cout << response["size"] << endl;
                return response.dump(); // 此处为brpc同步通信
            }
            // 发送包的定义                    {"type", "site_execute"},
            //          {"site", localSiteName},
            //          {"content",
            //           {{"execute_node", std::to_string(root_id)}}}};
            // 数据包的定义
            //  {"column",
            else if (statement->isType(hsql::kStmtInsert))
            {
                const auto *insert = static_cast<const hsql::InsertStatement *>(statement);
                std::shared_ptr<server::FragInsertStat> fragInsert;
                try
                {
                    fragInsert = InsertStat(insert, cfg).buildFragInsertStatment(cfg);
                }
                catch (const std::string &msg)
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
                          {"data", insertData}}}};
                    json resp = sitesManager.sendMsg(site, data.dump());
                    if (resp["info"].get<std::string>() != "(success)")
                        return resp.dump();
                    LOG(INFO) << "Insert: " << site << " data size: " << insertData.size() << std::endl;
                }
                return json{{"err", 0}}.dump();
            }
            else if (statement->isType(hsql::kStmtCreate))
            {
                const auto *create = static_cast<const hsql::CreateStatement *>(statement);
                std::shared_ptr<server::FragCreateStat> fragCreate;
                try
                {
                    fragCreate = CreateStat(create, cfg).buildFragCreateStatment(cfg);
                }
                catch (const std::string &msg)
                {
                    return msg;
                }
                // update Config
                CreateStat createStat(create, cfg);
                json columnDefs;
                for (auto &cInfo : createStat.columnDefs)
                {
                    columnDefs.push_back({{"columnName", cInfo.name},
                                          {"columnType", columnType2str(cInfo.columnType)}});
                }
                json updateConfig{
                    {"type", "updateConfig"},
                    {"site", localSiteName},
                    {"content",
                     {{"tableInfo",
                       {{createStat.table, columnDefs}}}}}};
                json resp = sitesManager.broadcastMsg(updateConfig.dump());
                if (resp["info"].get<std::string>() != "(success)")
                    return resp.dump();

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
                    for (auto &def : fragCreate->columnDefsMap[site])
                        data["content"]["columns"].push_back({{"name", def.name},
                                                              {"type", def.columnType}});
                    json resp = sitesManager.sendMsg(site, data.dump());
                    if (resp["info"].get<std::string>() != "(success)")
                        return resp.dump();
                }
                putTables();
                return json{{"err", 0}}.dump();
            }
            else if (statement->isType(hsql::kStmtDelete))
            {
                const auto *del = static_cast<const hsql::DeleteStatement *>(statement);
                std::shared_ptr<server::FragDeleteStat> fragDelete;
                try
                {
                    fragDelete = DeleteStat(del, cfg).buildFragDeleteStatment(cfg);
                }
                catch (const std::string &msg)
                {
                    return msg;
                }

                for (auto &site : fragDelete->sites)
                {
                    json data{
                        {"type", "delete"},
                        {"site", localSiteName},
                        {"content",
                         {{"table", fragDelete->table}}}};
                    json resp = sitesManager.sendMsg(site, data.dump());
                    if (resp["info"].get<std::string>() != "(success)")
                        return resp.dump();
                }
                return json{{"err", 0}}.dump();
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
        return json{{"err", 1}, {"info", "[sql is not valid]"}}.dump();
    }

    std::string ServiceImpl::execPartition(const std::string &msg)
    {
        json updateConfig = json::parse(msg);
        json cmd = {
            {"type", "updateConfig"},
            {"content", updateConfig}
        };
        json resp = sitesManager.broadcastMsg(cmd.dump());
        return resp.dump();
    }

    template <typename T>
    T str2T(std::string x)
    {
        T v;
        std::stringstream ss(x);
        ss >> v;
        return v;
    }

    std::string ServiceImpl::execLoad(const std::string &msg)
    {
        json json_msg = json::parse(msg);
        std::string table = json_msg["table"].get<std::string>();
        json &data = json_msg["data"];

        auto fragInsert = std::make_shared<FragInsertStat>();
        fragInsert->table = table;

        if (!cfg.fragmentInfo.count(table) || !cfg.tableInfo.count(table))
            return json{{"err", 1}, {"info", "need fragment or table info"}}.dump();

        auto &fInfos = cfg.fragmentInfo.find(table)->second;
        auto &tInfos = cfg.tableInfo.find(table)->second;

        std::map<Column, int> globalIndexMap;
        for (size_t i = 0; i < tInfos.size(); i++)
            globalIndexMap[table + "_" + tInfos[i].name] = i;

        for (auto &f : fInfos)
        {
            json collector;

            std::vector<Field> fields;
            std::vector<hsql::DataType> filedTypes;
            std::vector<size_t> filedIndexes;
            if (f.columns[0] == "*")
            {
                for (size_t i = 0; i < tInfos.size(); i++)
                {
                    std::string field = table + "_" + tInfos[i].name;
                    fields.push_back(field);
                    filedTypes.push_back(tInfos[i].columnType);
                    filedIndexes.push_back(i);
                }
            }
            else
            {
                for (auto &col : f.columns)
                {
                    std::string field = table + "_" + col;
                    fields.push_back(field);
                    filedTypes.push_back(tInfos[globalIndexMap[field]].columnType);
                    filedIndexes.push_back(globalIndexMap[field]);
                }
            }

            std::map<Column, int> indexMap;
            for (size_t i = 0; i < fields.size(); i++)
                indexMap[fields[i]] = i;

            for (auto &row : data)
            {
                json v_row;
                for (size_t i : filedIndexes)
                    v_row.push_back(row[i]);

                bool flag = true;
                for (size_t i = 0, s = f.conditions.size(); i < s; i++)
                {
                    auto &item = f.conditions[i];
                    std::string field = table + "_" + item.column;
                    hsql::OperatorType op = item.op;
                    json value = item.value;

                    size_t index = indexMap[field];
                    std::string rowValue = v_row[index].get<std::string>();
                    // std::cout << "rowValue: " << rowValue << " field: " << field << " op: " << op << " value: " << value.dump() << " index: " << index << std::endl;
                    switch (filedTypes[index])
                    {
                    case hsql::DataType::REAL:
                        flag &= CompareFn<float>(op, str2T<float>(rowValue), value.get<float>());
                        break;
                    case hsql::DataType::TEXT:
                        flag &= CompareFn<std::string>(op, rowValue, value.get<std::string>());
                        break;
                    case hsql::DataType::INT:
                        flag &= CompareFn<int>(op, str2T<int>(rowValue), value.get<int>());
                        break;
                    default:
                        throw "Value Error";
                    }
                }
                if (flag)
                    collector.push_back(v_row);
            }
            std::cout << "collector size: {" << f.site << "," << collector.size() << "}" << std::endl;
            if (collector.size() > 0)
                fragInsert->fragData.insert({f.site, collector});
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
                  {"data", insertData}}}};
            json resp = sitesManager.sendMsg(site, data.dump());
            if (resp["info"].get<std::string>() != "(success)")
                return resp.dump();
            etcd_opt(string2json("/table/" + table + "/fragment/" + site, std::to_string(insertData.size())), "PUT");
        }
        return json{{"err", 0}}.dump();
    }

}
