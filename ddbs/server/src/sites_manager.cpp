#include <server/sites_manager.hpp>

namespace server {

SitesManager::SitesManager() 
    : channelMap(), channels() {

    std::ifstream f("res/site_config.json");
    json site_config = json::parse(f);

    for (auto &site_config : site_config) {
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
        options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
        options.max_retry = FLAGS_max_retry;
        std::string url = site_config["addr"].get<std::string>();
        if (channel->Init(url.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
            LOG(ERROR) << "Fail to initialize channel";
            exit(-1);
        }

        std::string site_name = site_config["site-name"].get<std::string>();
        channelMap.insert({site_name, channel.get()});
        channels.emplace_back(std::move(channel));
    }
}

brpc::Channel* SitesManager::getChannel(std::string siteName) {
    auto channel = channelMap[siteName];
    return channel;
}

}