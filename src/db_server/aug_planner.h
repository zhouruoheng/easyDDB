#include "executor.h"


namespace db::exe{

class AugPlanNode{
public:
    int node_id;
    int parent_id;//-1表示是根节点，且该输出了，这里的data_site_id和site_id是一样的
    vector<int> child_id;
    int site_id;
    int data_site_id;
    char* sql;
};


class AugmentedPlan{
public:
    int root;
    std::vector<AugPlanNode> augplannodes;
};

AugmentedPlan build_augmented_plan(db::opt::Tree query_tree);



} // namespace db::exe
