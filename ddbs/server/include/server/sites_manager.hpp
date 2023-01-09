#pragma once

#include <server/marco.hpp>
#include <mutex>
namespace server {

class SitesManager {
public:
    SitesManager();
    ~SitesManager() = default;

    brpc::Channel* getChannel(std::string siteName);

    // std::string send_site_message(std::string msg_type, std::string site_name, std::string data);
    json sendMsg(std::string siteName, std::string data);
    json broadcastMsg(std::string data);
    // const brpc::CallId sendMsgAsync(std::string siteName, std::string data);

    size_t requestID;
    std::map<std::string, brpc::Channel*> channelMap; // name to index
    std::vector<std::unique_ptr<brpc::Channel>> channels;
};

}
