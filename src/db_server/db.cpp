#include <gflags/gflags.h>
#include <butil/logging.h>
#include <iostream>
#include <brpc/server.h>
#include <unistd.h> //这个是新加的
#include "db.pb.h"
#include "cluster_manager.h"
#include "mysql_connector/mysql_connector.h"
#include "optimizer/preprocess.h"

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

    std::vector<string> string2list(string inputStr)
    {
        stringstream Str(inputStr);
        vector<string> temp;
        string cur;
        while (getline(Str, cur, ','))
            temp.push_back(cur);
        return temp;
    } // 工具：将字符串转化为vector

    // string data_fetch(string msg, const SiteManager &manager)
    // {
    //     string result = db::mysql::select_sql(manager.local_site_name, msg);
    //     return result;
    // }

    string data_push(int node_id, string sql, const SiteManager &manager, string site_name)
    {
        string datamsg = std::to_string(node_id) + "," + db::mysql::select_sql(site_name, sql);//获取数据并打包
        string result = send_site_message(site_name, "data_push", datamsg, node_id + 10000, manager);//发送数据
        return result;
    } // 这个函数是在data_site_name上执行sql语句，将结果返回到site_name上,data_push的result一般是ok

    string data_receive(string msg, const SiteManager &manager)
    {
        // 这里要存表
        // 这里要标记某个数据已经执行完毕
        // 这里要新建一个线程,用于执行plan_scan函数
        int pid = fork();
        if (pid == 0)
        {
            plan_scan();
            exit(0);
        }

        // 这里要返回ok
        return "ok";
    } // 这个函数用于存表并开启下一轮执行，data_receive的result一般是ok,ddl 12.9
    // 或许可以把plan存到etcd里面，然后每个节点都去读取，然后执行，这样就不用每次都传输plan了
    string plan_scan(){
        vector<int> to_execute();
        // 这里要从etcd读取plan
        AugmentedPlan plan= read_plan();
        // 这里要从plan中找到可以执行的节点
        to_execute = plan.find_to_execute();
        // 这里要执行to_execute中的节点
        if (to_execute.size() == 0)
        {
            return "ok";
        }
        for(int i=0;i<to_execute.size();i++){
            int node_id = to_execute[i];
            string sql = plan.node[node_id].sql;
            string site_name = plan.node[node_id].site_name;
            string data_site_name = plan.node[node_id].data_site_name;
            data_push(node_id,sql,manager,data_site_name);
        }
        return plan_scan();
    }
    AugmentedPlan read_plan(){
        AugmentedPlan plan;
        // 这里要从etcd读取plan
        return plan;
    }


    string config(const std::string &msg, const SiteManager &manager)
    {
        return "config not written"; // 这里要处理create database, create table, drop database, drop table,create partition, drop partition等语句
    }

    class AugPlanNode
    {
    public:
        int node_id;
        int parent_id; //-1表示是根节点，且该输出了，这里的data_site_name和site_name是一样的
        vector<int> child_id;
        std::string site_name;      // 这里是节点执行sql后数据的目标site
        std::string data_site_name; // 这里是节点执行sql后数据的源site，也即它自己
        string sql;
    }; // 执行计划的节点，增加了节点的site_name和data_site_name，以及对应的sql语句

    class AugmentedPlan
    {
    public:
        int root;     // 根节点id，对于根节点来说，其site_name和data_site_name是一样的
        int node_num; // 节点总数，用于指导下游建立list
        std::vector<AugPlanNode> augplannodes;
    }; // 执行计划的增强版，增加了节点的site_name和data_site_name，以及对应的sql语句

    AugmentedPlan build_augmented_plan(db::opt::Tree query_tree)
    {
        db::AugmentedPlan aug_plan;
        db::AugPlanNode node0;
        node0.node_id = 0;
        node0.parent_id = 3;
        node0.site_name = "ddb0";
        node0.data_site_name = "ddb1";
        node0.sql = "select * from publisher";
        aug_plan.augplannodes.push_back(node0);
        db::AugPlanNode node1;
        node1.node_id = 1;
        node1.parent_id = 3;
        node0.site_name = "ddb0";
        node0.data_site_name = "ddb2";
        node1.sql = "select * from publisher";
        aug_plan.augplannodes.push_back(node1);
        db::AugPlanNode node2;
        node2.node_id = 2;
        node2.parent_id = 3;
        node0.site_name = "ddb0";
        node0.data_site_name = "ddb3";
        node2.sql = "select * from publisher";
        aug_plan.augplannodes.push_back(node2);
        db::AugPlanNode node3;
        node3.node_id = 3;
        node3.parent_id = -1;
        node3.child_id.push_back(0);
        node3.child_id.push_back(1);
        node3.child_id.push_back(2);
        node3.site_name = "ddb0";
        node3.data_site_name = "ddb0";
        node3.sql = "select * from publisher union all select * from node0 union all select * from node1 union all select * from node2";
        aug_plan.augplannodes.push_back(node3);
        aug_plan.root = 3;
        return aug_plan;
    } // 这里是用于进行建立增强执行计划的函数，要注意的是，这里是需要重写的，这个函数运行在cluster端的

    std::string dis_execute(AugmentedPlan aug_plan, const SiteManager &manager);

    string sql_execute(const std::string &msg, const SiteManager &manager)
    {
        db::opt::Tree query_tree = db::opt::build_query_tree();
        db::AugmentedPlan aug_plan = db::build_augmented_plan(query_tree);
        return dis_execute(aug_plan, manager);
    }

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
    }//发送数据到site

    string site_execute(string msg, const SiteManager &manager)
    {
        std::vector<std::string> msg_vec = db::string2list(msg);
        int node_id = stoi(msg_vec[0]);
        std::string data_site_name = msg_vec[1];
        std::string sql = msg_vec[2];
        if (data_site_name == manager.local_site_name)
        {
            return db::mysql::select_sql(manager.local_site_name, sql);
        }

        else
        {
            string msg = db::send_site_message(data_site_name, "data", sql, node_id, manager);
            string sql = "create table node" + to_string(node_id) + " ( id int,name varchar(255),nation varchar(255))";
            std::cout << "sql" << sql << std::endl;
            string sql2 = "insert into node" + to_string(node_id) + " values" + msg;
            std::cout << "sql2" << std::endl;
            db::mysql::create_insert_delete_sql(manager.local_site_name, sql);
            db::mysql::create_insert_delete_sql(manager.local_site_name, sql2);

            return "ok";
        }
    }
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
        else if (msg_type == "data")
        {
            response_message = db::data_receive(msg, manager); // 这里是对数据获取的处理，会写到site_executor.h中
        }
        else if (msg_type == "plan")
        {
            response_message = db::site_plan_execute(msg, manager); // 这里是对plan的处理，会写到executor.h中
        }

        else
        {
            response_message = "error";
        }

        return response_message;
    }

    // 这里是原来的executor文件

    // 这里是原来的aug_planner文件

    std::string dis_execute(AugmentedPlan aug_plan, const SiteManager &manager)
    {
        // std::string msg0 = db::send_site_message("ddb0", "order", "0,ddb1,select * from publisher", 0, manager);
        // std::string msg1 = db::send_site_message("ddb0", "order", "1,ddb2,select * from publisher", 1, manager);
        // std::string msg2 = db::send_site_message("ddb0", "order", "2,ddb3,select * from publisher", 2, manager);
        // std::string msg3 = db::send_site_message("ddb0", "order", "3,ddb0,select * from publisher union all select * from node0 union all select * from node1 union all select * from node2", 3, manager);
        // string sql3 = "drop table node" + to_string(0);
        // db::mysql::create_insert_delete_sql(manager.local_site_name, sql3);
        // string sql4 = "drop table node" + to_string(1);
        // db::mysql::create_insert_delete_sql(manager.local_site_name, sql4);
        // string sql5 = "drop table node" + to_string(2);
        // db::mysql::create_insert_delete_sql(manager.local_site_name, sql5);
        // return msg3;
        // 从总体执行逻辑上看，这个函数拿到AugmentedPlan之后，需要做的事情是：首先将augmentedplan的信息序列化后发到所有的节点，然后收取每个节点执行完成的信息，需要注意的是，对于主节点来说，其收取的是最后产生的数据；
        // 这个函数ddl12月11日完成
    }

    // 这里是原来的site_task_executor文件
    std::string db::site_plan_execute(std::string msg, const SiteManager &manager)
    {
    }
} // namespace db

int main(int argc, char *argv[])
{
    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    // Generally you only need one Server.
    brpc::Server server;

    // Instance of your service.
    db::ClusterServiceImpl cluster_service_impl;

    // Add the service into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&cluster_service_impl,
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0)
    {
        LOG(ERROR) << "Fail to add service";
        return -1;
    }

    butil::EndPoint point;
    if (!FLAGS_listen_addr.empty())
    {
        if (butil::str2endpoint(FLAGS_listen_addr.c_str(), &point) < 0)
        {
            LOG(ERROR) << "Invalid listen address:" << FLAGS_listen_addr;
            return -1;
        }
    }
    else
    {
        point = butil::EndPoint(butil::IP_ANY, FLAGS_port);
    }
    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(point, &options) != 0)
    {
        LOG(ERROR) << "Fail to start EchoServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;
}
