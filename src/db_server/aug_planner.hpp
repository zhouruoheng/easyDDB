#include "query_optimizer.hpp"

class AugPlanNode
{
public:
    int parent;
    std::vector<int> child;
    std::string type; // Fragment,Union,Join
    int site;
    std::pair<std::string, int> fname;
    std::string join;
    std::vector<std::string> attr, projection;
    std::vector<Condition> select;
};
