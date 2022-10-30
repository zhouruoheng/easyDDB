#pragma once

#include <iostream>
#include <thread>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include <vector>
#include "db.pb.h"

class KVManager {
public:
    std::vector<brpc::Channel> channels;
    std::vector<db::ClusterService_Stub> stubs;

    KVManager() {
        
    }

    ~KVManager() = default;
};
