#pragma once

#include <IR/macro.hpp>
#include <IR/config.hpp>
#include <IR/frag_query.hpp>
#include <optimizer/optimizer.hpp>
#include <optimizer/mysql_connector.hpp>
#include <optimizer/metadata.hpp>

namespace server
{
    class AugPlanNode
    {
    public:
        int node_id;
        int parent_id; //-1表示是根节点
        std::string execute_site;
        std::string sql;
        std::string save_sql;
        int is_finished;
    };

    class AugmentedPlan
    {
    public:
        int node_num;
        std::vector<AugPlanNode> augplannodes;
    };
    class SelectExecutor
    {
    public:
        SelectExecutor(hsql::SQLParserResult result, SitesManager sitesManager, string localsitename);
        SelectExecutor() = default;
        ~SelectExecutor() = default;
        std::string execute();
        AugmentedPlan plan;
        SitesManager manager;
        string localsitename;
        void save_plan_to_etcd(AugmentedPlan plan);
        AugmentedPlan get_plan_from_etcd();
        std::string get_result();
        std::string site_execute(std::string msg);
        std::string plan_scan();
        std::string data_push(int node_id, std::string sql, std::string target_site_name, AugmentedPlan plan, int is_root);
        void delete_data(int node_id);
        std::string data_receive(std::string msg);
        std::string data_receive_root(std::string msg);
        AugmentedPlan build_augmented_plan(Tree query_tree);
        std::vector<int> find_node_to_build(&std::vector<int> already_built, Tree query_tree);
        AugPlanNode transfer_to_AugNode(Tree query_tree, int i, AugmentedPlan plan);

    }
}
