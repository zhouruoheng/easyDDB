#pragma once

#include <IR/macro.hpp>
#include <IR/config.hpp>
#include <IR/frag_query.hpp>
#include <optimizer/optimizer.hpp>
#include <optimizer/mysql_connector.hpp>
#include <optimizer/metadata.hpp>
#include <server/sites_manager.hpp>

namespace server
{
    using db::opt::Tree;
    class AugPlanNode
    {
    public:
        int node_id;
        int parent_id; //-1表示是根节点
        std::string execute_site;
        std::string sql;
        std::string save_sql;
    };

    class AugmentedPlan
    {
    public:
        AugmentedPlan(json plan_json);
        AugmentedPlan() = default;
        int node_num;
        std::vector<AugPlanNode> augplannodes;
        int find_root_id();
        json to_json();
    };
    class SelectExecutor
    {
    public:
        AugmentedPlan plan;
        SitesManager &manager;
        std::string localsitename;

        SelectExecutor(hsql::SQLParserResult *result, SitesManager &sitesManager, std::string localsitename);
        SelectExecutor(SitesManager &sitesManager, std::string localsitename, AugmentedPlan plan);
        SelectExecutor() = default;
        ~SelectExecutor() = default;
        std::string get_result();

        std::string data_push(int node_id, std::string sql, std::string target_site_name, AugmentedPlan plan, int is_root);
        void delete_data(int node_id);

        
        
        
        std::string execute();
        void save_plan_to_etcd(AugmentedPlan plan);
        AugmentedPlan get_plan_from_etcd();
        std::string data_receive(std::string msg);
        std::string site_execute(std::string msg);
        std::string plan_scan();
    };
    std::string deal_with_order(const std::string &msg, SitesManager &sitesManager, std::string localsitename);
    std::string deal_with_data(const std::string &msg, SitesManager &sitesManager, std::string localsitename);
    AugmentedPlan build_augmented_plan(Tree query_tree);
    std::vector<int> find_node_to_build(std::vector<int> &already_built, Tree query_tree);
    AugPlanNode transfer_to_AugNode(Tree query_tree, int i, AugmentedPlan plan);
    void save_plan_to_etcd(AugmentedPlan plan);
    AugmentedPlan get_plan_from_etcd();
}
