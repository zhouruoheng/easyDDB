#include "kv_manager.hpp"

#include <fstream>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

KVManager::KVManager() : site_dict(), sites() {
    std::ifstream f("res/cluster_config.json");
    json cluster_config = json::parse(f);
    for (auto &site_config : cluster_config) {
        
        std::shared_ptr<KVSite> ptr_site = std::make_shared<KVSite>();

        // A Channel represents a communication line to a Server. Notice that 
        // Channel is thread-safe and can be shared by all threads in your program.
        // site.channel

        // Normally, you should not call a Channel directly, but instead construct
        // a stub Service wrapping it. stub can be shared by all threads as well.
        // site.stub

        // Initialize the channel, NULL means using default options.
        brpc::ChannelOptions options;
        options.protocol = FLAGS_protocol;
        options.connection_type = FLAGS_connection_type;
        options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
        options.max_retry = FLAGS_max_retry;
        std::string url = cluster_config["addr"].get<std::string>();
        if (ptr_site->channel.Init(url.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
            LOG(ERROR) << "Fail to initialize channel";
            exit(-1);
        }

        std::string site_name = cluster_config["site-name"].get<std::string>();
        site_dict.insert({site_name, ptr_site});
        sites.push_back(ptr_site);
    }
}
