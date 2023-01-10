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

    SelectStat(const hsql::SelectStatement *select, const Config &cfg);
    std::shared_ptr<FragSelectStat> buildFragSelectStatment(const Config &cfg);
};

class InsertStat {
public:
    Table table;
    std::vector<json> data;  // array(array(...))

    InsertStat(const hsql::InsertStatement *insert, const Config &cfg);
    std::shared_ptr<FragInsertStat> buildFragInsertStatment(const Config &cfg);
};

class CreateStat {
public:
    Table table;
    std::vector<ColumnDef> columnDefs;

    CreateStat(const hsql::CreateStatement *create, const Config &cfg);
    std::shared_ptr<FragCreateStat> buildFragCreateStatment(const Config &cfg);
};

class DeleteStat {
public:
    Table table;

    DeleteStat(const hsql::DeleteStatement *del, const Config &cfg);
    std::shared_ptr<FragDeleteStat> buildFragDeleteStatment(const Config &cfg);
};

}
