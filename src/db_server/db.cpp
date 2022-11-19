#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include "db.pb.h"
#include "cluster_manager.h"
#include "executor.h"
#include "site_task_executor.h"

// For ClusterService
DEFINE_bool(echo_attachment, true, "Echo attachment as well");
DEFINE_int32(port, 6666, "TCP Port of this server");
DEFINE_string(listen_addr, "", "Server listen address, may be IPV4/IPV6/UDS."
            " If this is set, the flag port will be ignored");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
             "(waiting for client to close connection before server stops)");

// For ClusterManager
DEFINE_string(site_name, "site-1", "Local site name");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");

// Your implementation of db::ClientService
// Notice that implementing brpc::Describable grants the ability to put
// additional information in /status.
namespace db {


class ClusterServiceImpl : public ClusterService {
private:
    SiteManager manager;
public:
    ClusterServiceImpl() : manager(FLAGS_site_name) {};
    virtual ~ClusterServiceImpl() {};
    virtual void SendClientMsg(google::protobuf::RpcController* cntl_base,
                      const ClientRequest* request,
                      ClientResponse* response,
                      google::protobuf::Closure* done) {
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(cntl_base);

        // The purpose of following logs is to help you to understand
        // how clients interact with servers more intuitively. You should 
        // remove these logs in performance-sensitive servers.
        LOG(INFO) << "Received request[log_id=" << cntl->log_id() 
                  << "] from " << cntl->remote_side() 
                  << " to " << cntl->local_side()
                  << ": " << request->msg()
                  << " (attached=" << cntl->request_attachment() << ")";

        // Fill response.
        char *response_message=deal_with_msg(request->msg_type(),request->msg());
        response->set_msg(response_message);

        // You can compress the response by setting Controller, but be aware
        // that compression may be costly, evaluate before turning on.
        // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);

        if (FLAGS_echo_attachment) {
            // Set attachment which is wired to network directly instead of
            // being serialized into protobuf messages.
            cntl->response_attachment().append(cntl->request_attachment());
        }
    }
    char* deal_with_msg(int msg_type, const std::string& msg);
};

}  // namespace db

char* db::ClusterServiceImpl::deal_with_msg(int msg_type, const std::string& msg) {
    char *response_message;
    switch (msg_type) {
        case "sql":
            response_message = db::exe::sql_execute(msg);//这里是对sql语句的处理，会写到executor.h中
            break;
        case "conf":
            response_message = db::exe::config(msg);//这里是对配置信息的处理，会写到executor.h中
            break; 
        case "data":
            response_message = db::exe::data_fetch(msg);//这里是对数据请求的的处理，会写到site_executor.h中
            break;
        case "order":
            response_message = db::exe::site_execute(msg);//这里是对site的执行的处理，会写到site_executor.h中
            break;
        default:
            response_message = "error";
            break;
    }
    return response_message;
}





int main(int argc, char* argv[]) {
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
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add service";
        return -1;
    }

    butil::EndPoint point;
    if (!FLAGS_listen_addr.empty()) {
        if (butil::str2endpoint(FLAGS_listen_addr.c_str(), &point) < 0) {
            LOG(ERROR) << "Invalid listen address:" << FLAGS_listen_addr;
            return -1;
        }
    } else {
        point = butil::EndPoint(butil::IP_ANY, FLAGS_port);
    }
    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(point, &options) != 0) {
        LOG(ERROR) << "Fail to start EchoServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;

}
