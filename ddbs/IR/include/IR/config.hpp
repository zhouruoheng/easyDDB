#pragma once

#include <IR/macro.hpp>

namespace server {

struct FragmentSiteInfo {
    Site site;
    std::vector<ColumnCondition> conditions;
};

using FragmentInfo = std::map<Table, std::vector<FragmentSiteInfo>>;
using TableInfo = std::map<Table, std::vector<ColumnDef>>;

struct Config {
    FragmentInfo fragmentInfo;
    TableInfo tableInfo;

    void updateConfig(const std::string &path);
    void updateConfig(const json &jsonConfig);
};

}
