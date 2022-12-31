#include <IR/frag_query.hpp>
#include <iostream>

namespace server {

void FragSelectStat::print() {
    if (is_leaf) {
        std::cout << "leaf----------------------" << std::endl;
        for (auto &frag : leaf.frags)
            std::cout << frag.table << "." << frag.site << " ";
        std::cout << std::endl;
        for (auto &cond : leaf.conditions)
            std::cout << "{" << cond.field << "}" << std::endl;
        std::cout << "----------------------" << std::endl;
    } else {
        node.left->print();
        node.right->print();
        std::cout << "node----------------------" << std::endl;
        for (auto &join : node.joinOn)
            std::cout << "(" << join.first << "," << join.second << ")" << std::endl;
        std::cout << "----------------------" << std::endl;
    }
}

}