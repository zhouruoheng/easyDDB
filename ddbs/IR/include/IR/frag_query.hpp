#pragma once

#include <IR/macro.hpp>

namespace server {

class FragSelectStat {
public:
    struct FragQueryLeaf {
        std::vector<Fragment> frags;
        std::vector<FieldCondition> conditions;
    };

    struct FragQueryNode {
        std::shared_ptr<FragSelectStat> left;
        std::shared_ptr<FragSelectStat> right;
        std::vector<std::pair<Field, Field>> joinOn;
    };
    
    bool is_leaf;
    FragQueryLeaf leaf;
    FragQueryNode node;

    bool is_root;
    std::vector<Field> fields;

    bool is_finish;
    std::vector<Fragment> results;
};

class FragInsertStat {
public:
    Table table;
    std::map<Site, json> fragData;
};

class FragCreateStat {
public:
    Table table;
    std::map<Site, std::vector<ColumnDef>> columnDefsMap;
    std::vector<Site> sites;
};

class FragDeleteStat {
public:
    Table table;
    std::vector<Site> sites;
};

}
