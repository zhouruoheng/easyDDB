#include <IR/config.hpp>
#include <iostream>
#include <fstream>

namespace server {

void Config::loadConfig(const std::string &path) {
    std::ifstream f(path);
    json jsonConfig = json::parse(f);
    updateConfig(jsonConfig);
    cfgPath = path;
    config = jsonConfig;
    f.close();
}

void Config::updateConfig(const json &jsonConfig) {
    if (jsonConfig.contains("fragmentInfo")) {
        for (auto &kv : jsonConfig["fragmentInfo"].items()) {
            config["fragmentInfo"][kv.key()] = kv.value();
            std::vector<FragmentSiteInfo> fsInfos;
            for (auto &fsInfo : kv.value()) {
                Site site = fsInfo["site"].get<std::string>();
                std::vector<Column> columns;
                for (auto &c : fsInfo["attr"])
                    columns.push_back(c.get<std::string>());
                std::vector<ColumnCondition> conditions;
                for (auto &c : fsInfo["conditions"]) {
                    conditions.push_back(ColumnCondition{
                        c["column"].get<std::string>(),
                        str2opType(c["op"].get<std::string>()),
                        c["value"],
                    });
                }
                fsInfos.push_back({
                    site,
                    columns,
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
            config["tableInfo"][kv.key()] = kv.value();
            std::vector<ColumnDef> cInfos;
            for (auto &cInfo : kv.value()) {
                cInfos.push_back({
                    cInfo["columnName"].get<std::string>(),
                    str2columnType(cInfo["columnType"].get<std::string>())
                });
            }
            tableInfo.insert({
                kv.key(),
                cInfos
            });
        }
    }
}


void Config::saveConfig() {
    std::cout << "cfgPath: " << cfgPath << std::endl;
    std::cout << "cfg: " << config.dump(4) << std::endl;
    std::ofstream o(cfgPath);
    o << config.dump(4) << std::endl;
    o.close();
}

}
