#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include <client/cli.hpp>
#include <db.pb.h>
#include <nlohmann/json.hpp>

// test use begin
#include <SQLParser.h>
#include <util/sqlhelper.h>
// end

using json = nlohmann::json;

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "localhost:1231", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 10000, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");

void test_fn(const std::string &query) {
    using namespace hsql;

    hsql::SQLParserResult result;
    hsql::SQLParser::parse(query, &result);

    if (result.isValid() && result.size() > 0) {
        const hsql::SQLStatement* statement = result.getStatement(0);

        if (statement->isType(hsql::kStmtSelect)) {
            const auto* select = static_cast<const hsql::SelectStatement*>(statement);
            hsql::printStatementInfo(select);
        }
        else if (statement->isType(hsql::kStmtInsert)) {
            const auto* insert = static_cast<const hsql::InsertStatement*>(statement);
            hsql::printStatementInfo(insert);
        }
        else if (statement->isType(hsql::kStmtCreate)) {
            const auto* create = static_cast<const hsql::CreateStatement*>(statement);
            hsql::printStatementInfo(create);
            for (auto &column: *create->columns) {
                std::cout << "(" << column->name << "," << column->type << ")" << std::endl;
            }
        }
        else if (statement->isType(hsql::kStmtDelete)) {
            const auto* del = static_cast<const hsql::DeleteStatement*>(statement);
            if (del->schema != nullptr)
                std::cout << "schema: " << del->schema << std::endl;
            if (del->tableName != nullptr)
                std::cout << "tableName: " << del->tableName << std::endl;
            if (del->expr != nullptr)
                printExpression(del->expr, 1);
        }
    }
}

std::string send_request(
    db::Service_Stub &stub, 
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
    stub.ClientMsg(&cntl, &request, &response, NULL);
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
    std::cout << ss.str() << std::endl;
    return response.msg();
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
    db::Service_Stub stub(&channel);

    // Initialize the channel, NULL means using default options.
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms /*milliseconds*/;
    options.max_retry = FLAGS_max_retry;
    if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        exit(-1);
    }
    
    client::Shell shell([&](const std::string &msg) -> void {
        // test_fn(msg);
        std::string recv_msg = send_request(stub, "sql", msg, ++log_id);
        printf("recv msg: %s\n", recv_msg.c_str());
        json json_msg = json::parse(recv_msg);
        if (json_msg.contains("data")) {
            printf("row numbers: %d\n", (int)json_msg["data"].size());
        }
    });

    shell.register_conf_cmd(
        "partition",
        [&](const std::string &msg) -> void {
            std::string recv_msg = send_request(stub, "partition", msg, ++log_id);
            printf("recv msg: %s\n", recv_msg.c_str());
        }
    );

    shell.register_conf_cmd(
        "load",
        [&](const std::string &name) -> void {
            std::string _name = name;
            if (name.find('/') != std::string::npos)
                _name = _name.substr(name.find_last_of('/') + 1);
            std::ifstream f(name + ".tsv");
            std::string line;
            json data;
            while (std::getline(f, line)) {
                json row;
                std::string value;
                for (auto &c : line) {
                    if (c != '\t')
                        value += c;
                    else {
                        row.push_back(value);
                        value = "";
                    }
                }
                row.push_back(value);
                data.push_back(row);
            }
            json msg = {
                {"table", _name},
                {"data", data}
            };
            std::string recv_msg = send_request(stub, "load", msg.dump(), ++log_id);
            printf("recv msg: %s\n", recv_msg.c_str());
        }
    );

    shell.run();
    return 0;
}
