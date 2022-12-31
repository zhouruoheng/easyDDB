#include <IR/query.hpp>
#include <iostream>

namespace server {

SelectStat::SelectStat(const hsql::SelectStatement *select)
    : fields(), tables(), joins(), fieldConditionMap() {
    for (hsql::Expr* expr : *select->selectList) {
        switch (expr->type)
        {
        case hsql::kExprStar:
            fields.push_back("*");
            break;
        default:
            fields.push_back(Table(expr->table) + "_" + Column(expr->name));
            break;
        }
    }
    switch (select->fromTable->type) {
    case hsql::kTableName:
        tables.push_back(select->fromTable->name);
        break;
    default:
        for (hsql::TableRef* tbl : *select->fromTable->list) 
                tables.push_back(tbl->name);
        break;
    }
    if (select->whereClause != nullptr) {
        std::function<void(hsql::Expr*)> fn;
        fn = [&](hsql::Expr *expr) -> void {
            if (expr->opType == hsql::kOpAnd) {
                fn(expr->expr);
                fn(expr->expr2);
            } else {
                hsql::Expr *lc = expr->expr;
                hsql::Expr *rc = expr->expr2; 
                if (lc->type == hsql::kExprColumnRef and rc->type == hsql::kExprColumnRef) {
                    switch (expr->opType)
                    {
                    case hsql::kOpEquals:
                        joins.push_back({
                            Table{lc->table},
                            Field{Table{lc->table} + "_" + Column{lc->name}},
                            Table{rc->table},
                            Field{Table{rc->table} + "_" + Column{rc->name}},
                        });
                        break;
                    default:
                        throw "Value Error";
                        break;
                    }
                } else {
                    switch (expr->opType)
                    {
                    case hsql::kOpEquals:
                    case hsql::kOpLess:
                    case hsql::kOpLessEq:
                    case hsql::kOpGreater:
                    case hsql::kOpGreaterEq:
                        fieldConditionMap[Table(lc->table)].push_back( 
                            FieldCondition{
                                Field{Table{lc->table} + "_" + Column{lc->name}},
                                expr->opType, 
                                parseLiteralVal(rc)
                            }
                        );
                        break;
                    default:
                        throw "Value Error";
                        break;
                    }
                }
            }
        };
        fn(select->whereClause);
    }
}

std::shared_ptr<FragSelectStat> SelectStat::buildFragSelectStatment(const Config &cfg) {
    auto buildSingleFragQuery = [&](const Table &table) -> std::shared_ptr<FragSelectStat> {
        auto &fInfos = cfg.fragmentInfo.find(table)->second;
        auto fragQuery = std::make_shared<FragSelectStat>();
        fragQuery->is_leaf = true;
        for (auto &f : fInfos) {
            fragQuery->leaf.frags.push_back({table, f.site});
        }
        if (fieldConditionMap.count(table)) {
            fragQuery->leaf.conditions = fieldConditionMap[table];
        }
        fragQuery->is_root = false;
        fragQuery->is_finish = false;
        return fragQuery;
    };
    auto buildJoinFragQuery = [&](std::shared_ptr<FragSelectStat> left, std::shared_ptr<FragSelectStat> right, const Table &table, const std::set<Table> &joinTables) -> std::shared_ptr<FragSelectStat> {
        auto fragQuery = std::make_shared<FragSelectStat>();
        fragQuery->is_leaf = false;
        fragQuery->node.left = left;
        fragQuery->node.right = right;

        for (auto &join : joins) {
            if (join.table0 != table && join.table1 != table)
                continue;
            Field f0 = join.field0;
            Field f1 = join.field1;
            if (join.table1 != table)
                std::swap(f0, f1);
            fragQuery->node.joinOn.push_back({f0, f1});
        }
        fragQuery->is_root = false;
        fragQuery->is_finish = false;
        return fragQuery;
    };
    std::shared_ptr<FragSelectStat> root = nullptr;
    std::set<Table> joinTables;
    for (Table &table : tables) {
        joinTables.insert(table);
        auto singleQuery = buildSingleFragQuery(table);
        if (root == nullptr)
            root = singleQuery;
        else {
            root = buildJoinFragQuery(root, singleQuery, table, joinTables);
        }
    }
    root->is_root = true;
    root->fields = fields;
    return root;
}

InsertStat::InsertStat(const hsql::InsertStatement *insert)
  : table(), data() {
    auto parseLiteralVal = [](hsql::Expr* expr) -> json {
        json val;
        switch (expr->type)
        {
        case hsql::kExprLiteralFloat:
            val = expr->fval;
            break;
        case hsql::kExprLiteralInt:
            val = expr->ival;
            break;
        case hsql::kExprLiteralString:
            val = expr->name;
            break;
        default:
            throw "Value Error";
            break;
        }
        return val;
    };
    table = insert->tableName;
    json row;
    for (hsql::Expr* expr : *insert->values) {
        json value = parseLiteralVal(expr);
        row.push_back(value);
    }
    data.push_back(row);
}

std::shared_ptr<FragInsertStat> InsertStat::buildFragInsertStatment(const Config &cfg) {
    auto fragInsert = std::make_shared<FragInsertStat>();
    fragInsert->table = table;

    auto &fInfos = cfg.fragmentInfo.find(table)->second;
    auto &cInfos = cfg.tableInfo.find(table)->second;
    std::map<Column, int> col2id;
    for (size_t i = 0; i < cInfos.size(); i++)
        col2id[cInfos[i].name] = i;
    for (auto &f : fInfos) {
        std::vector<json> collector;
        for (auto &row : data) {
            bool flag = true;
            for (auto &item : f.conditions) {
                std::string column = item.column;
                hsql::OperatorType op = item.op;
                json value = item.value;
                int index = col2id[column];
                if (value.is_number_float()) {
                    flag &= CompareFn<float>(op, row[index].get<float>(), value.get<float>());
                } else if (value.is_number_integer()) {
                    flag &= CompareFn<int>(op, row[index].get<int>(), value.get<int>());
                } else if (value.is_string()) {
                    flag &= CompareFn<std::string>(op, row[index].get<std::string>(), value.get<std::string>());
                }
            }
            if (flag)
                collector.push_back(row);
        }
        if (collector.size() > 0)
            fragInsert->fragData.insert({f.site, collector});
    }
    return fragInsert;
}

CreateStat::CreateStat(const hsql::CreateStatement *create)
: table(), columns() {
    table = create->tableName;
    for (auto &col : *create->columns) {
        columns.push_back({
            col->name,
            col->type.data_type
        });
    }
}

std::shared_ptr<FragCreateStat> CreateStat::buildFragCreateStatment(const Config &cfg) {
    auto fragCreate = std::make_shared<FragCreateStat>();
    fragCreate->table = table;
    for (auto &col : columns) {
        fragCreate->columns.push_back({
            table + "_" + col.name,
            col.columnType
        });
    }
    auto &fInfos = cfg.fragmentInfo.find(table)->second;
    for (auto &f : fInfos)
        fragCreate->sites.push_back(f.site);
    return fragCreate;
}

}
