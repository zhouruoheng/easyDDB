#pragma once

#include <iostream>
#include <thread>
#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "db.pb.h"

DECLARE_string(protocol);
DECLARE_string(connection_type);
DECLARE_string(load_balancer);
DECLARE_int32(timeout_ms);
DECLARE_int32(max_retry);


class SiteClient {
public:
    brpc::Channel channel;
    db::ClusterService_Stub stub;

    SiteClient() : channel(), stub(&channel) {}
    ~SiteClient() = default;
};

class SiteManager {
public:
    std::map<std::string, std::shared_ptr<SiteClient>> site_dict; // name to index
    std::vector<std::shared_ptr<SiteClient>> sites;
    std::string local_site_name;
    
    SiteManager(const std::string &_local_site_name);
    ~SiteManager() = default;
};
