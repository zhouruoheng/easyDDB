#pragma once

#include <IR/macro.hpp>

namespace server {

struct FragmentSiteInfo {
    Site site;
    std::vector<Column> columns;
    std::vector<ColumnCondition> conditions;
};

using FragmentInfo = std::map<Table, std::vector<FragmentSiteInfo>>;
using TableInfo = std::map<Table, std::vector<ColumnDef>>;

struct Config {
    std::string cfgPath;
    json config;
    FragmentInfo fragmentInfo;
    TableInfo tableInfo;

    void loadConfig(const std::string &path);
    void updateConfig(const json &jsonConfig);
    void saveConfig();
};

}
