#include "aug_planner.h"

db::exe::AugmentedPlan build_augmented_plan(db::opt::Tree query_tree){
    db::exe::AugmentedPlan aug_plan;
    db::exe::AugPlanNode node0;
    node0.node_id=0;
    node0.parent_id=3;
    node0.site_id=0;
    node0.data_site_id=1;
    node0.sql="select * from publisher";
    aug_plan.augplannodes.push_back(node0);
    db::exe::AugPlanNode node1;
    node1.node_id=1;
    node1.parent_id=3;
    node1.site_id=0;
    node1.data_site_id=2;
    node1.sql="select * from publisher";
    aug_plan.augplannodes.push_back(node1);
    db::exe::AugPlanNode node2;
    node2.node_id=2;
    node2.parent_id=3;
    node2.site_id=0;
    node2.data_site_id=3;
    node2.sql="select * from publisher";
    aug_plan.augplannodes.push_back(node2);
    db::exe::AugPlanNode node3;
    node3.node_id=3;
    node3.parent_id=-1;
    node3.child_id.push_back(0);
    node3.child_id.push_back(1);
    node3.child_id.push_back(2);
    node3.site_id=0;
    node3.data_site_id=0;
    node3.sql="select * from publisher union all select * from node0 union all select * from node1 union all select * from node2";
    aug_plan.augplannodes.push_back(node3);
    aug_plan.root=3;
    return aug_plan;
}