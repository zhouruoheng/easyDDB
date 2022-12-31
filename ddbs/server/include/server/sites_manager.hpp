#pragma once

#include <server/marco.hpp>

namespace server {

class SitesManager {
public:
    SitesManager();
    ~SitesManager() = default;

    brpc::Channel* getChannel(std::string siteName);

    std::map<std::string, brpc::Channel*> channelMap; // name to index
    std::vector<std::unique_ptr<brpc::Channel>> channels;
};

}
