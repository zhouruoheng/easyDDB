#pragma once

#include <server/marco.hpp>
#include <server/sites_manager.hpp>
#include <server/db_manager.hpp>
#include <server/select_executor.hpp>

// Your implementation of db::ClientService
// Notice that implementing brpc::Describable grants the ability to put
// additional information in /status.
namespace server {

using namespace db;

class ServiceImpl : public Service {
public:
    ServiceImpl();
    virtual ~ServiceImpl() = default;
    virtual void ClientMsg(google::protobuf::RpcController* cntl_base,
                      const ClientRequest* request,
                      ClientResponse* response,
                      google::protobuf::Closure* done);
    virtual void ServerMsg(google::protobuf::RpcController* cntl_base,
                      const ServerRequest* request,
                      ServerResponse* response,
                      google::protobuf::Closure* done);

    std::string send_site_message(std::string msg_type,std::string siteName, std::string data);
    json sendMsg(std::string siteName, std::string data);
    json broadcastMsg(std::string data);

    std::string execSql(const std::string &sql);
    std::string execPartition(const std::string &msg);
    std::string deal_with_msg(const std::string msg_type, const std::string &msg);
    const brpc::CallId sendMsgAsync(std::string siteName, std::string data);
    void onMsgResponse(brpc::Controller* cntl, db::ServerResponse* response);
    std::string columnname_to_type(std::string name);
    AugmentedPlan build_augmented_plan(db::opt::Tree query_tree);
    AugPlanNode transfer_to_AugNode(db::opt::Tree query_tree, int i, AugmentedPlan &plan);
    
    Config cfg;
    std::string localSiteName;
    SitesManager sitesManager;
    DbManager dbManager;
    uint64_t requestID;
};
void* sendAsyMsg(void *args);
struct Aargs{
    std::string siteName;
    std::string msg;
    ServiceImpl* service;
    Aargs(std::string siteName, std::string msg, ServiceImpl* service):siteName(siteName), msg(msg), service(service){}
};

}
