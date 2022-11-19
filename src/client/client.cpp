#include <iostream>
#include <thread>
#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include "db.pb.h"
#include "cli.hpp"

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "localhost:1234", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");

std::string send_request(
    db::ClusterService_Stub &stub, 
    const std::string &msg_type,
    const std::string &msg, 
    int log_id
) {
    db::ClientRequest request;
    db::ClientResponse response;
    brpc::Controller cntl;

    request.set_msg_type(msg_type);
    request.set_msg(msg);

    cntl.set_log_id(log_id ++);  // set by user
    // Set attachment which is wired to network directly instead of 
    // being serialized into protobuf messages.
    cntl.request_attachment().append(FLAGS_attachment);

    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    stub.SendClientMsg(&cntl, &request, &response, NULL);
    std::stringstream ss;
    if (!cntl.Failed()) {
        ss << "Received response from " << cntl.remote_side()
            << " to " << cntl.local_side()
            << ": " << response.msg() << " (attached="
            << cntl.response_attachment() << ")"
            << " latency=" << cntl.latency_us() << "us";
    } else {
        ss << cntl.ErrorText();
    }
    return ss.str(); 
}

int main(int argc, char *argv[]) {
    // setup brpc
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    int log_id = 0;

    // A Channel represents a communication line to a Server. Notice that 
    // Channel is thread-safe and can be shared by all threads in your program.
    brpc::Channel channel;

    // Normally, you should not call a Channel directly, but instead construct
    // a stub Service wrapping it. stub can be shared by all threads as well.
    db::ClusterService_Stub stub(&channel);

    // Initialize the channel, NULL means using default options.
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
    options.max_retry = FLAGS_max_retry;
    if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        exit(-1);
    }
    
    cli::Shell shell([&](const std::string &msg) -> void {
        std::string recv_msg = send_request(stub, "sql", msg, ++log_id);
        printf("execute sql: %s\n", msg.c_str());
        printf("recv msg: %s\n", recv_msg.c_str());
    });

    shell.register_conf_cmd(
        "partition",
        [&](const std::string &msg) -> void {
            std::string recv_msg = send_request(stub, "conf", msg, ++log_id);
            printf("execute conf: %s\n", msg.c_str());
            printf("recv msg: %s\n", recv_msg.c_str());
        }
    );

    shell.run();
    return 0;
}
