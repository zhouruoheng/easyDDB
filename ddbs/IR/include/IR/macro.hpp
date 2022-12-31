#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <memory>
#include <map>
#include <set>
#include <functional>
#include <nlohmann/json.hpp>
#include <SQLParser.h>
#include <util/sqlhelper.h>

namespace server {

using json = nlohmann::json;
using u32 = unsigned int;
using i32 = int;
using f32 = float;

struct LiteralVal {
    enum class LiteralType {
        String,
        Integer,
        Float,
    };

    LiteralType type;
    std::string name;
    union LiteralValData {
        f32 fv;
        i32 iv;
    } data;

    LiteralVal(const char *data) : type(LiteralType::String), name(data) {}
    LiteralVal(f32 val) : type(LiteralType::String), name() { data.fv = val; }
    LiteralVal(i32 val) : type(LiteralType::String), name() { data.iv = val; }
};

using Column = std::string;
using Field = std::string;  // Field = Table + "_" + Column
using Table = std::string;
using Site  = std::string;

struct ColumnDef {
    std::string name;
    hsql::DataType columnType;
};

struct Condition {
    hsql::OperatorType op;
    json value;
};

struct ColumnCondition {
    Column column;
    hsql::OperatorType op;
    json value;
};

struct FieldCondition {
    Field field;
    hsql::OperatorType op;
    json value;
};

struct Join {
    Table table0;
    Field field0;
    Table table1;
    Field field1;
};

inline bool operator <(const Join &a, const Join &b) {
    return a.table0 + "_" + a.field0 + "_" + a.table1 + "_" + a.field1 
        < b.table0 + "_" + b.field0 + "_" + b.table1 + "_" + b.field1;
}

struct Fragment {
    Table table;
    Site site;
};

inline bool operator <(const Fragment &a, const Fragment &b) {
    return std::make_pair(a.table, a.site) < std::make_pair(b.table, b.site);
}

inline std::string parseOp(hsql::OperatorType opType) {
    switch (opType)
    {
    case hsql::kOpEquals:
        return "=";
        break;
    case hsql::kOpNotEquals:
        return "!=";
        break;
    case hsql::kOpLess:
        return "<";
        break;
    case hsql::kOpLessEq:
        return "<=";
        break;
    case hsql::kOpGreater:
        return ">";
        break;
    case hsql::kOpGreaterEq:
        return ">=";
        break;
    default:
        throw "Value Error";
        break;
    }
};

inline json parseLiteralVal(hsql::Expr* expr) {
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

inline hsql::OperatorType parseOpInverse(std::string op) {
    // std::cout << "op: " << op << std::endl;
    if (op == "=")
        return hsql::kOpEquals;
    if (op == "!=")
        return hsql::kOpNotEquals;
    if (op == "<")
        return hsql::kOpLess;
    if (op == "<=")
        return hsql::kOpLessEq;
    if (op == ">")
        return hsql::kOpGreater;
    if (op == ">=")
        return hsql::kOpGreaterEq;
    throw "Value Error!";
};

inline hsql::DataType parseColumnTypeInverse(std::string type) {
    if (type == "integer")
        return hsql::DataType::INT;
    if (type == "real")
        return hsql::DataType::REAL;
    if (type == "text")
        return hsql::DataType::TEXT;
    throw "Value Error!";
};

template<typename T> bool CompareFn(hsql::OperatorType op, T a, T b) {
    switch (op)
    {
    case hsql::kOpEquals:
        return a == b;
        break;
    case hsql::kOpNotEquals:
        return a != b;
        break;
    case hsql::kOpLess:
        return a < b;
        break;
    case hsql::kOpLessEq:
        return a <= b;
        break;
    case hsql::kOpGreater:
        return a > b;
        break;
    case hsql::kOpGreaterEq:
        return a >= b;
        break;    
    default:
        throw "Value Error";
        return false;
    }
}

}
