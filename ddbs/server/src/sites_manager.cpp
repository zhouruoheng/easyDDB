#include <server/sites_manager.hpp>

namespace server
{

    SitesManager::SitesManager()
        : channelMap(), channels(), requestID(0)
    {

        std::ifstream f("res/site_config.json");
        json site_config = json::parse(f);

        for (auto &site_config : site_config)
        {
            auto channel = std::unique_ptr<brpc::Channel>(new brpc::Channel);
            // A Channel represents a communication line to a Server. Notice that
            // Channel is thread-safe and can be shared by all threads in your program.

            // Normally, you should not call a Channel directly, but instead construct
            // a stub Service wrapping it. stub can be shared by all threads as well.
            // site.stub

            // Initialize the channel, NULL means using default options.
            brpc::ChannelOptions options;
            options.protocol = FLAGS_protocol;
            options.connection_type = FLAGS_connection_type;
            options.timeout_ms = FLAGS_timeout_ms /*milliseconds*/;
            options.max_retry = FLAGS_max_retry;
            std::string url = site_config["addr"].get<std::string>();
            if (channel->Init(url.c_str(), FLAGS_load_balancer.c_str(), &options) != 0)
            {
                LOG(ERROR) << "Fail to initialize channel";
                exit(-1);
            }

            std::string site_name = site_config["site-name"].get<std::string>();
            channelMap.insert({site_name, channel.get()});
            channels.emplace_back(std::move(channel));
        }
    }

    brpc::Channel *SitesManager::getChannel(std::string siteName)
    {
        auto channel = channelMap[siteName];
        return channel;
    }

    // std::string SitesManager::send_site_message(std::string msg_type, std::string site_name, std::string data)
    // {
    //     brpc::Channel *channel = getChannel(site_name);
    //     db::Service_Stub stub(channel);
    //     db::ServerRequest request;
    //     db::ServerResponse response;
    //     brpc::Controller cntl;
    //     request.set_msg_type(msg_type);

    //     request.set_msg(data);

    //     cntl.set_log_id(requestID++); // set by user
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
    //                   << ": " << response.msg() << " (attached="
    //                   << cntl.response_attachment() << ")"
    //                   << " latency=" << cntl.latency_us() << "us" << std::endl;
    //         return response.msg();
    //     }
    //     else
    //     {
    //         std::stringstream ss;
    //         ss << cntl.ErrorText();
    //         return ss.str();
    //     }
    // } // 发送数据到site----zy

    // create a AsyMsg
    // const brpc::CallId SitesManager::sendMsgAsync(std::string siteName, std::string data){
    //     brpc::Channel *channel = getChannel(siteName);
    //     db::Service_Stub stub(channel);
    //     db::ServerRequest request;
    //     db::ServerResponse* response;
    //     brpc::Controller* cntl;
    //     request.set_msg(data);

    //     cntl->set_log_id(requestID++); // set by user
    //     // Set attachment which is wired to network directly instead of
    //     // being serialized into protobuf messages.
    //     cntl->request_attachment().append("attachment");
    //     const brpc::CallId cid = cntl->call_id();
    //     google::protobuf::Closure* done = brpc::NewCallback(&SitesManager::onMsgResponse, cntl, response);
    //     stub.ServerMsg(cntl, &request, response, done);
    //     return cid;
    // }
    json SitesManager::sendMsg(std::string siteName, std::string data)
    {
        brpc::Channel *channel = getChannel(siteName);
        db::Service_Stub stub(channel);
        db::ServerRequest request;
        db::ServerResponse response;
        brpc::Controller cntl;
        request.set_msg(data);

        cntl.set_log_id(requestID++); // set by user
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
                      << ": " << response.msg() << " (attached="
                      << cntl.response_attachment() << ")"
                      << " latency=" << cntl.latency_us() << "us" << std::endl;
            return json::parse(response.msg());
        }
        else
        {
            std::stringstream ss;
            ss << cntl.ErrorText();
            return json::object({{"info", ss.str()},
                                 {"content", nullptr}});
        }
    };

    json SitesManager::broadcastMsg(std::string data)
    {
        for (auto &p : channelMap)
        {
            json resp = sendMsg(p.first, data);
            std::string info = resp["info"].get<std::string>();
            if (info != "(success)")
                return resp;
        }
        return json::object({{"info", "(success)"},
                             {"content", nullptr}});
    }

}