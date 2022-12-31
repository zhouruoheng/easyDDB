#pragma once

#include <IR/macro.hpp>
#include <IR/config.hpp>
#include <IR/frag_query.hpp>

namespace server {

class SelectStat {
public:
    std::vector<Field> fields;
    std::vector<Table> tables;
    std::vector<Join> joins;
    std::map<Table, std::vector<FieldCondition>> fieldConditionMap;

    SelectStat(const hsql::SelectStatement *select);
    std::shared_ptr<FragSelectStat> buildFragSelectStatment(const Config &cfg);
};

class InsertStat {
public:
    Table table;
    std::vector<json> data;  // array(array(...))

    InsertStat(const hsql::InsertStatement *insert);
    std::shared_ptr<FragInsertStat> buildFragInsertStatment(const Config &cfg);
};

class CreateStat {
public:
    Table table;
    std::vector<ColumnDef> columns;

    CreateStat(const hsql::CreateStatement *create);
    std::shared_ptr<FragCreateStat> buildFragCreateStatment(const Config &cfg);
};

}
