#include <gflags/gflags.h>
#include <butil/logging.h>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <brpc/server.h>
#include <unistd.h> //这个是新加的
#include "db.pb.h"
#include "cluster_manager.h"
#include "mysql_connector/mysql_connector.h"
#include "optimizer/preprocess.h"
#include "metadata/metadata.h"

// For ClusterService
DEFINE_bool(echo_attachment, true, "Echo attachment as well");
DEFINE_int32(port, 6666, "TCP Port of this server");
DEFINE_string(listen_addr, "", "Server listen address, may be IPV4/IPV6/UDS."
                               " If this is set, the flag port will be ignored");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
                                 "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
                              "(waiting for cGS");

DEFINE_string(attachment, "", "Carry this along with requests");

// For ClusterManager
DEFINE_string(site_name, "ddb0", "Local site name");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 10000, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");

// Your implementation of db::ClientService
// Notice that implementing brpc::Describable grants the ability to put
// additional information in /status.
namespace db
{
   
    class AugPlanNode
    {
    public:
        int node_id;
        int parent_id; //-1表示是根节点
        std::string execute_site;
        string sql;
        string savesql;
        int is_finished;
    };

    class AugmentedPlan
    {
    public:
        int node_num;
        std::vector<AugPlanNode> augplannodes;
    };

    std::string save_plan_to_etcd(AugmentedPlan plan)
    {
        std::string key = "node_num";
        std::string value = std::to_string(plan.node_num);
        etcd_set(key, value);
        for (int i = 0; i < plan.node_num; i++)
        {
            key = std::to_string(i) + "parent_id";
            value = std::to_string(plan.augplannodes[i].parent_id);
            etcd_set(key, value);
            key = std::to_string(i) + "execute_site";
            value = plan.augplannodes[i].execute_site;
            etcd_set(key, value);
            key = std::to_string(i) + "sql";
            value = plan.augplannodes[i].sql;
            etcd_set(key, value);
            key = std::to_string(i) + "savesql";
            value = plan.augplannodes[i].savesql;
            etcd_set(key, value);
            key = std::to_string(i) + "is_finished";
            value = std::to_string(plan.augplannodes[i].is_finished);
            etcd_set(key, value);
        }
    } // 将plan存到etcd里面
    AugmentedPlan get_plan_from_etcd()
    {
        AugmentedPlan plan;
        plan.node_num = stoi(etcd_get("plan_num"));
        for (int i = 0; i < plan.node_num; i++)
        {
            AugplanNode node;
            node.id = i;
            node.sql = etcd_get(std::to_string(i) + "sql");
            node.save_sql = etcd_get(std::to_string(i) + "save_sql");
            node.is_finished = stoi(etcd_get(std::to_string(i) + "is_finished"));
            node.parent_id = stoi(etcd_get(std::to_string(i) + "parent_id"));
            plan.augplannodes.push_back(node);
        }
    } // 从etcd里面获取plan
    string etcd_get(string key)
    {
        string value;
        return json_to_string(etcd_opt(string_to_json(key, ""), "GET"));
    } // 从etcd里面获取key对应的value
    string etcd_set(string key, string value)
    {
        return json_to_string(etcd_opt(string_to_json(key, value), "PUT"));
    } // 将key和value存到etcd里面

    string db::ClusterServiceImpl::deal_with_msg(const std::string msg_type, const std::string &msg)
    {
        std::string response_message;
        if (msg_type == "sql")
        {
            response_message = db::sql_execute(msg, manager); // 面向cli的两个接口之一，用于处理前端发来的sql语句
        }
        else if (msg_type == "config")
        {
            response_message = db::config(msg, manager); // 面向cli的两个接口之一，用于处理前端发来的配置信息，这里可以最后一周实现
        }
        else if (msg_type == "order")
        {
            response_message = db::site_execute(msg, manager); // 这里是对order的处理，会写到site_executor.h中
        }
        else if (msg_type == "data_push")
        {
            response_message = db::data_receive(msg, manager); // 这里是对数据获取的处理，会写到site_executor.h中
        }
        else if (msg_type == "site_execute")
        {
            response_message = db::site_plan_execute(msg, manager); // 这里是对plan的处理，会写到site_executor.h中
        }
        else
        {
            response_message = "error";
        }

        return response_message;
    }

    string get_result()
    {
        clock_t start = clock();
        while (get_etcd("is_finished") != 1)
        {
        }
        string result_flie_name = get_etcd("result");
        // 读取result文件
        std::ifstream in(result_flie_name);
        std::string result;
        std::string line;
        while (getline(in, line))
        {
            result += line;
        }
        // 删除result文件
        remove(result_flie_name.c_str());
        clock_t end = clock();
        int seconds = (end - start) / CLOCKS_PER_SEC;
        std::cout << "total execution time:" << seconds << "s" << std::endl;
        return result;
    }

    string sql_execute(const std::string &msg, const SiteManager &manager)
    {
        std::string sql = msg;
        db::opt::Tree query_tree = db::opt::build_query_tree(sql);
        db::AugmentedPlan aug_plan = db::build_augmented_plan(query_tree);
        save_plan_to_etcd(aug_plan);
        // read all site_name from manager.site_dict
        std::vector<std::string> site_names;
        for (auto it = manager.site_dict.begin(); it != manager.site_dict.end(); it++)
        {
            site_names.push_back(it->first);
        }
        // send plan to all site
        for (int i = 0; i < site_names.size(); i++)
        {
            std::string site_name = site_names[i];
            std::string msg = "plan";
            int pid = fork();
            if (pid == 0)
            {
                send_site_message(site_name, "site_execute", msg, i, manager);
                exit(0);
            }

        } // 发送所有的plan到所有的site，并直接返回ok
        std::string result = get_result();
        etcd_set("result", "");
        etcd_set("node_num", "0");
        return result;
    }

    std::string db::site_execute(std::string msg, const SiteManager &manager)
    {
        return plan_scan(manager);
    } // 每一个站点接受到执行指令之后执行一次plan_scan

    string plan_scan(const SiteManager &manager)
    {
        AugmentedPlan plan = get_plan_from_etcd();
        vector<int> to_execute;
        for (int i = 0; i < plan.node_num; i++)
        {
            if (plan.augplannodes[i].is_finished == 0)
            {
                int child_num = 0;
                int child_finished_num = 0;
                for (int j = 0; j < plan.node_num; j++)
                {
                    if (plan.augplannodes[j].parent_id == i)
                    {
                        child_num++;
                        if (plan.augplannodes[j].is_finished == 1)
                        {
                            child_finished_num++;
                        }
                    }
                }
                if (child_num == child_finished_num)
                {
                    if (plan.augplannodes[i].execute_site == manager.local_site_name)
                    {
                        to_execute.push_back(i);
                    }
                }
            }
        }
        for (int i = 0; i < to_execute.size(); i++)
        {
            int node_id = to_execute[i];
            if (plan.augplannodes[node_id].parent_id == -1)
            {
                data_push(node_id, plan.augplannodes[node_id].sql, manager, manager.local_site_name, plan, 1); // r若为根节点，则结果直接输出即可
            }
            else
            {
                data_push(node_id, plan.augplannodes[node_id].sql, manager, plan.augplannodes[plan.augplannodes[node_id].parent_id].execute_site, plan, 0); // 若不为根节点，则需要将结果发送给父节点
            }
        }
        return "ok";
    }

    string data_push(int node_id, string sql, const SiteManager &manager, string target_site_name, AugmentedPlan plan, int is_root)
    {
        string datamsg = std::to_string(node_id) + "," + db::mysql::select_sql(manager.local_site_name, sql); // 获取数据并打包
        if (is_root == 1)
        {
            // file_name is current time
            time_t now = time(0);
            tm *ltm = localtime(&now);
            string file_name = std::to_string(1900 + ltm->tm_year) + std::to_string(1 + ltm->tm_mon) + std::to_string(ltm->tm_mday) + std::to_string(ltm->tm_hour) + std::to_string(ltm->tm_min) + std::to_string(ltm->tm_sec) + ".txt";
            std::ofstream out(file_name);
            out << datamsg.substr(datamsg.find(",") + 1);
            etcd_set("result", file_name);
            etcd_set(std::to_string(node_id) + "is_finished", "1");
            etcd_set("is_finished", "1");
        }
        else
        {
            if (target_site_name == manager.local_site_name)
            {
                // 这里存表的sql语句要改一下
                string create_sql = "create table node" + to_string(node_id) + plan[node_id].save_sql;
                string save_sql = "insert into " + std::to_string(node_id) + " values " + datamsg.substr(datamsg.find(",") + 1);
                db::mysql::create_insert_delete_sql(manager.local_site_name, create_sql);
                db::mysql::create_insert_delete_sql(manager.local_site_name, save_sql);
                // 这里要标记某个数据已经执行完毕
                etcd_set(std::to_string(node_id) + "is_finished", "1");
                // 这里要新建一个线程,用于执行plan_scan函数
                int pid = fork();
                if (pid == 0)
                {
                    plan_scan();
                    exit(0);
                }
            }
            else
            {
                string result = send_site_message(target_site_name, "data_push", datamsg, node_id + 10000, manager);
            }
        } // 发送数据

        for (int i = 0; i < plan.node_num; i++)
        {
            if (plan.augplannodes[i].parent_id == node_id)
            {
                delete_data(i, manager.local_site_name);
                // 这里要删除表
            }
        }
        return result;
    } // 该函数在current_site上执行一条sql，然后将其push到target_site上，返回ok或者error
    void delete_data(int node_id, string local_site_name)
    {
        std::string sql = "delete from  " + std::to_string(node_id);
        db::mysql::create_insert_delete_sql(local_site_name, sql);
    }
    string data_receive(string msg, const SiteManager &manager)
    {
        // 这里要存表
        AugmentedPlan plan = get_plan_from_etcd(); // 这里要从etcd里面读取plan
        int node_id = stoi(msg.substr(0, msg.find(",")));
        string data = msg.substr(msg.find(",") + 1);
        string create_sql = "create table node" + to_string(node_id) + plan[node_id].save_sql;
        string save_sql = "insert into " + std::to_string(node_id) + " values " + data;
        db::mysql::create_insert_delete_sql(manager.local_site_name, create_sql);
        db::mysql::create_insert_delete_sql(manager.local_site_name, save_sql);
        // 这里要标记某个数据已经执行完毕
        etcd_set(std::to_string(node_id) + "is_finished", "1");
        // 这里要新建一个线程,用于执行plan_scan函数
        int pid = fork();
        if (pid == 0)
        {
            plan_scan();
            exit(0);
        }
        // 这里要返回ok
        return "ok";
    } // 这个函数用于存表并开启下一轮执行，data_receive的result一般是ok
    // 或许可以把plan存到etcd里面，然后每个节点都去读取，然后执行，这样就不用每次都传输plan了

    AugmentedPlan build_augmented_plan(db::opt::Tree query_tree)
    {
        AugmentedPlan plan;
        // get the total num of query_tree.tr
        plan.node_num = query_tree.tr.size();
        map<int, string> name_map;
        vector<int> already_built;
        while (already_built.size() < plan.node_num)
        {
            vector<int> node_to_execute = find_node_to_build(already_built, query_tree);
            for (int i = 0; i < node_to_execute.size(); i++)
            {
                // build a node
                AugPlanNode = transfer_to_AugNode(query_tree, node_to_execute[i],plan);
                plan.augplannodes.push_back(AugPlanNode);
                already_built.push_back(node_to_execute[i]);
            }
        }
        return plan;
    }
    vector<int> find_node_to_build(&<vector> int already_built, db::opt::Tree query_tree)
    {
        vector<int> node_to_build;
        for (int i = 0; i < query_tree.tr.size(); i++)
        {
            // build a node if it's children are all built
            for (int j = 0; j < query_tree.tr[i].children.size(); j++)
            {
                if (already_built[query_tree.tr[i].children[j]] == 0)
                {
                    break;
                }
                if (j == query_tree.tr[i].children.size() - 1)
                {
                    node_to_execute.push_back(i);
                }
            }
        }
        return node_to_build;
    }
    AugPlanNode transfer_to_AugNode(db::opt::Tree query_tree, int i, AugmentedPlan plan)
    {
        AugPlanNode node;
        node.node_id = i;
        node.parent_id = query_tree.tr[i].parent;
        node.is_finished = 0;
        node.execute_site = name_map[query_tree.tr[i].site];
        vector<string> projection_attr;
        vector<string> selection_attr;
        string select_what;
        string where_what;
        string from_what;
        if (query_tree.tr[i].type == "Fragment")
        {
            if (query_tree.tr[i].projection.size)
            {
                for (int j = 0; j < query_tree.tr[i].projection.size; j++)
                {
                    projection_attr.push_back(query_tree.tr[i].projection[j]);
                }
                for (int j = 0; j < projection_attr.size(); j++)
                {
                    select_what += projection_attr[j];
                    if (j != projection_attr.size() - 1)
                    {
                        select_what += ",";
                    }
                }
            } // if projection is not empty
            else
            {
                select_what = "*";
            } // if projection is empty
            from_what = query_tree.tr[i].fname.first;
            if (query_tree.tr[i].selection.size)
            {
                for (int j = 0; j < query_tree.tr[i].select.size; j++)
                {
                    selection_attr.push_back(query_tree.tr[i].select[j]).getStr();
                }
                for (int j = 0; j < selection_attr.size(); j++)
                {
                    where_what += selection_attr[j] if (j != selection_attr.size() - 1)
                    {
                        where_what += " and ";
                    }
                }
                node.sql = "select " + select_what + " from " + from_what + " where " + where_what;
            }
            else
            {
                node.sql = "select " + select_what + " from " + from_what;
            }
            vector<string> all_attr;
            string save_sql = "(";
            if (query_tree.tr[i].projection.size() > 0)
            {
                all_attr = query_tree.tr[i].projection;
            }
            else
            {
                all_attr = query_tree.tr[i].attr;
            }
            for (auto attr : all_attr)
            {
                string attr_name = attr.split(".")[1];
                string attr_type = etcd_get(attr);
                save_sql += attr_name + " " + attr_type + ",";
            }
            save_sql += ")";
            node.save_sql = save_sql;
        }
        else if (query_tree.tr[i].type == "Join")
        {
            string node_sql;
            string save_sql;
            string select_what;
            string where_what;
            string from_what;
            for(int j=0;j<query_tree.tr[i].children.size();j++){
                from_what+=query_tree.tr[i].children[j];
                if(j!=query_tree.tr[i].children.size()-1){
                    from_what+=",";
                }
            }
            string ltab=query_tree.tr[i].join.ltab;
            string lcol=query_tree.tr[i].join.lcol;
            string rtab=query_tree.tr[i].join.rtab;
            string rcol=query_tree.tr[i].join.rcol;
            where_what=query_tree.tr[i].children[0]+"."+lcol+"="+query_tree.tr[i].children[1]+"."+rcol;
            for(int j=0;j<query_tree.tr[i].projection.size();j++){
                if(std::find(query_tree.tr[i].children[0].attr.begin(),query_tree.tr[i].children[0].attr.end(),query_tree.tr[i].projection[j])!=query_tree.tr[i].children[0].attr.end()){
                    select_what+=query_tree.tr[i].children[0]+"."+query_tree.tr[i].projection[j];

                }
                else if(std::find(query_tree.tr[i].children[0].projection.begin(),query_tree.tr[i].children[0].projection.end(),query_tree.tr[i].projection[j])!=query_tree.tr[i].children[0].projection.end()){
                    select_what+=query_tree.tr[i].children[0]+"."+query_tree.tr[i].projection[j];
                }
                else{
                    select_what+=query_tree.tr[i].children[1]+"."+query_tree.tr[i].projection[j];

                }
                if(j!=query_tree.tr[i].projection.size()-1){
                    select_what+=",";
                }
            }
            node_sql="select "+select_what+" from "+from_what+" where "+where_what;
            node.sql=node_sql;
            vector<string> all_attr;
            save_sql = "(";
            if (query_tree.tr[i].projection.size() > 0)
            {
                all_attr = query_tree.tr[i].projection;
            }
            else
            {
                all_attr = query_tree.tr[i].attr;
            }
            for (auto attr : all_attr)
            {
                string attr_name = attr.split(".")[1];
                string attr_type = etcd_get(attr);
                save_sql += attr_name + " " + attr_type + ",";
            }
            save_sql += ")";
            node.save_sql = save_sql;
        }
        else if (query_tree.tr[i].type == "Union")
        {
            string node_sql;
            for (int j = 0; j < query_tree.tr[i].children.size(); j++)
            {
                node_sql+="select * from "+query_tree.tr[i].children[j];
                if(j!=query_tree.tr[i].children.size()-1){
                    node_sql+=" union all ";
                }
            }
            node.sql=node_sql;
            node.save_sql=plan.augplannodes[query_tree.tr[i].children[0]].save_sql;
        }
        return node;
    }

    // 这里是用于进行建立增强执行计划的函数，要注意的是，这里是需要重写的，这个函数运行在cluster端的

    class ClusterServiceImpl : public ClusterService
    {
    public:
        SiteManager manager;
        ClusterServiceImpl() : manager(FLAGS_site_name){};
        virtual ~ClusterServiceImpl(){};
        virtual void SendClientMsg(google::protobuf::RpcController *cntl_base,
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
            LOG(INFO) << "Received request[log_id=" << cntl->log_id()
                      << "] from " << cntl->remote_side()
                      << " to " << cntl->local_side()
                      << ": " << request->msg()
                      << " (attached=" << cntl->request_attachment() << ")";

            // Fill response.
            string response_message = deal_with_msg(request->msg_type(), request->msg());
            response->set_msg(response_message);

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
        string deal_with_msg(const std::string msg_type, const std::string &msg);
    };
    string send_site_message(std::string site_name, std::string msg_type, std::string msg, int log_id, const SiteManager &manager)
    {
        ClientRequest request;
        ClientResponse response;
        brpc::Controller cntl;

        request.set_msg_type(msg_type);
        request.set_msg(msg);

        cntl.set_log_id(log_id++); // set by user
        // Set attachment which is wired to network directly instead of
        // being serialized into protobuf messages.
        cntl.request_attachment().append(FLAGS_attachment);

        // Because `done'(last parameter) is NULL, this function waits until
        // the response comes back or error occurs(including timedout).
        std::shared_ptr<SiteClient> siteclient = manager.site_dict.find(site_name)->second;
        // manager.site_dict[
        siteclient->stub.SendClientMsg(&cntl, &request, &response, NULL);
        std::stringstream ss;
        if (!cntl.Failed())
        {
            ss << response.msg();
        }
        else
        {
            ss << cntl.ErrorText();
        }
        return ss.str();
    } // 发送数据到site
} // namespace db