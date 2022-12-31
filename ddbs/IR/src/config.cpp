#include <IR/config.hpp>
#include <iostream>
#include <fstream>

namespace server {

void Config::updateConfig(const std::string &path) {
    std::ifstream f(path);
    json jsonConfig = json::parse(f);
    updateConfig(jsonConfig);
}

void Config::updateConfig(const json &jsonConfig) {
    if (jsonConfig.contains("fragmentInfo")) {
        for (auto &kv : jsonConfig["fragmentInfo"].items()) {
            std::vector<FragmentSiteInfo> fsInfos;
            for (auto &fsInfo : kv.value()) {
                Site site = fsInfo["site"].get<std::string>();
                std::vector<ColumnCondition> conditions;
                for (auto &c : fsInfo["conditions"]) {
                    conditions.push_back(ColumnCondition{
                        c["column"].get<std::string>(),
                        parseOpInverse(c["op"].get<std::string>()),
                        c["value"],
                    });
                }
                fsInfos.push_back({
                    site,
                    conditions
                });
            }
            fragmentInfo.insert({
                kv.key(),
                fsInfos
            });
        }
    }
    if (jsonConfig.contains("tableInfo")) {
        for (auto &kv : jsonConfig["tableInfo"].items()) {
            std::vector<ColumnDef> cInfos;
            for (auto &cInfo : kv.value()) {
                cInfos.push_back({
                    cInfo["columnName"].get<std::string>(),
                    parseColumnTypeInverse(cInfo["columnType"].get<std::string>())
                });
            }
            tableInfo.insert({
                kv.key(),
                cInfos
            });
        }
    }
}


}