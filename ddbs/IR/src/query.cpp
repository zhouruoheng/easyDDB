#include <IR/query.hpp>
#include <iostream>

namespace server {

SelectStat::SelectStat(const hsql::SelectStatement *select, const Config &cfg)
    : fields(), tables(), joins(), fieldConditionMap() {
    switch (select->fromTable->type) {
    case hsql::kTableName:
        tables.push_back(select->fromTable->name);
        break;
    default:
        for (hsql::TableRef* tbl : *select->fromTable->list) 
                tables.push_back(tbl->name);
        break;
    }
    for (hsql::Expr* expr : *select->selectList) {
        switch (expr->type)
        {
        case hsql::kExprStar:
            for (auto &table : tables) {
                if (!cfg.fragmentInfo.count(table))
                    throw std::string("need frag Info");
                for (auto &def : cfg.tableInfo.find(table)->second)
                    fields.push_back(Table(table) + "_" + Column(def.name));
            }
            break;
        default:
            fields.push_back(Table(expr->table) + "_" + Column(expr->name));
            break;
        }
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
                                expr2value(rc)
                            }
                        );
                        break;
                    default:
                        throw "Value Error";
                    }
                }
            }
        };
        fn(select->whereClause);
    }
}

std::shared_ptr<FragSelectStat> SelectStat::buildFragSelectStatment(const Config &cfg) {
    auto buildJoinFragQuery = [&](std::shared_ptr<FragSelectStat> left, std::shared_ptr<FragSelectStat> right, const Table &table) -> std::shared_ptr<FragSelectStat> {
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
    auto buildSingleFragQuery = [&](const Table &table) -> std::shared_ptr<FragSelectStat> {
        if (!cfg.fragmentInfo.count(table))
            throw std::string("need frag Info");
        auto &fInfos = cfg.fragmentInfo.find(table)->second;
        if (fInfos[0].columns[0] != "*") {
            std::map<Column, int> columnCounter;
            for (auto &f : fInfos)
                for (auto &c : f.columns)
                    columnCounter[c] += 1;
            std::vector<Column> joinFields;
            for (auto &kv : columnCounter)
                if (kv.second == (int)fInfos.size())
                    joinFields.push_back(table + "_" + kv.first);
            std::shared_ptr<FragSelectStat> root = nullptr;
            for (auto &f : fInfos) {
                auto fragQuery = std::make_shared<FragSelectStat>();
                fragQuery->is_leaf = true;
                fragQuery->leaf.frags.push_back({table, f.site});
                if (fieldConditionMap.count(table)) {
                    for (auto &fc : fieldConditionMap[table]) {
                        if (std::find(joinFields.begin(), joinFields.end(), fc.field) != joinFields.end()) {
                            fragQuery->leaf.conditions.push_back(fc);
                        }
                    }
                }
                fragQuery->is_root = false;
                fragQuery->is_finish = false;
                if (root == nullptr)
                    root = fragQuery;
                else {
                    auto newRoot = std::make_shared<FragSelectStat>();
                    newRoot->is_leaf = false;
                    newRoot->node.left = root;
                    newRoot->node.right = fragQuery;
                    for (auto &f : joinFields)
                        newRoot->node.joinOn.push_back({f, f});
                    root = newRoot;
                }
            }
            return root;
        }
        else {
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
        }
    };
    std::shared_ptr<FragSelectStat> root = nullptr;
    for (Table &table : tables) {
        auto singleQuery = buildSingleFragQuery(table);
        if (root == nullptr)
            root = singleQuery;
        else {
            root = buildJoinFragQuery(root, singleQuery, table);
        }
    }
    root->is_root = true;
    root->fields = fields;
    return root;
}

InsertStat::InsertStat(const hsql::InsertStatement *insert, const Config &cfg)
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
    if (!cfg.fragmentInfo.count(table) || !cfg.tableInfo.count(table))
        throw std::string("need frag and table Info");
    auto &fInfos = cfg.fragmentInfo.find(table)->second;
    auto &tInfos = cfg.tableInfo.find(table)->second;

    std::map<Column, int> globalIndexMap;
    for (size_t i = 0; i < tInfos.size(); i++)
        globalIndexMap[table + "_" + tInfos[i].name] = i;

    for (auto &f : fInfos) {
        json collector;
    
        std::vector<Field> fields;
        std::vector<size_t> filedIndexes;
        if (f.columns[0] == "*") {
            for (size_t i = 0; i < tInfos.size(); i++) {
                std::string field = table + "_" + tInfos[i].name;
                fields.push_back(field);
                filedIndexes.push_back(globalIndexMap[field]);
            }
        } else {
            for (auto &col : f.columns) {
                std::string field = table + "_" + col;
                fields.push_back(field);
                filedIndexes.push_back(globalIndexMap[field]);
            }
        }

        std::map<Column, int> indexMap;
        for (size_t i = 0; i < fields.size(); i++)
            indexMap[fields[i]] = i;

        for (auto &row : data) {
            json v_row;
            for (size_t i : filedIndexes)
                v_row.push_back(row[i]);

            bool flag = true;
            for (auto &item : f.conditions) {
                std::string field = table + "_" + item.column;
                hsql::OperatorType op = item.op;
                json value = item.value;
                int index = indexMap[field];
                if (value.is_number_float()) {
                    flag &= CompareFn<float>(op, v_row[index].get<float>(), value.get<float>());
                } else if (value.is_number_integer()) {
                    flag &= CompareFn<int>(op, v_row[index].get<int>(), value.get<int>());
                } else if (value.is_string()) {
                    flag &= CompareFn<std::string>(op, v_row[index].get<std::string>(), value.get<std::string>());
                }
            }
            if (flag)
                collector.push_back(v_row);
        }
        if (collector.size() > 0)
            fragInsert->fragData.insert({f.site, collector});
    }
    return fragInsert;
}

CreateStat::CreateStat(const hsql::CreateStatement *create, const Config &cfg)
: table(), columnDefs() {
    table = create->tableName;
    for (auto &col : *create->columns) {
        columnDefs.push_back({
            col->name,
            col->type.data_type
        });
    }
}

std::shared_ptr<FragCreateStat> CreateStat::buildFragCreateStatment(const Config &cfg) {
    auto fragCreate = std::make_shared<FragCreateStat>();
    fragCreate->table = table;
    if (!cfg.fragmentInfo.count(table))
        throw std::string("need frag Info");
    auto &fInfos = cfg.fragmentInfo.find(table)->second;
    for (auto &f : fInfos) {
        fragCreate->sites.push_back(f.site);
        if (f.columns[0] != "*") {
            std::map<Column, hsql::DataType> columnTypeMap;
            for (auto &col : columnDefs)
                columnTypeMap[col.name] = col.columnType;
            for (auto &c : f.columns) {
                fragCreate->columnDefsMap[f.site].push_back({
                    table + "_" + c,
                    columnTypeMap[c]
                });
            }
        } else {
            for (auto &col : columnDefs) {
                fragCreate->columnDefsMap[f.site].push_back({
                    table + "_" + col.name,
                    col.columnType
                });
            }            
        }
    }
    return fragCreate;
}

DeleteStat::DeleteStat(const hsql::DeleteStatement *del, const Config &cfg)
: table(del->tableName) {
}

std::shared_ptr<FragDeleteStat> DeleteStat::buildFragDeleteStatment(const Config &cfg) {
    auto fragDelete = std::make_shared<FragDeleteStat>();
    fragDelete->table = table;
    if (!cfg.fragmentInfo.count(table))
        throw std::string("need frag Info");
    auto &fInfos = cfg.fragmentInfo.find(table)->second;
    for (auto &f : fInfos)
        fragDelete->sites.push_back(f.site);
    return fragDelete;
}

}
