#pragma once

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include <brpc/server.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <db.pb.h>
#include <nlohmann/json.hpp>
#include <SQLParser.h>
#include <util/sqlhelper.h>
#include <IR/query.hpp>


namespace server {

using json = nlohmann::json;

// For ClusterService
DECLARE_bool(echo_attachment);
DECLARE_int32(port);
DECLARE_string(listen_addr);
DECLARE_int32(idle_timeout_s);
DECLARE_int32(logoff_ms);

// For ClusterManager
DECLARE_string(site_name);
DECLARE_string(protocol);
DECLARE_string(connection_type);
DECLARE_string(load_balancer);
DECLARE_int32(timeout_ms);
DECLARE_int32(max_retry);

inline std::string Op2String(hsql::OperatorType op) {
    switch (op)
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
        return "";
    }
}

inline std::string Value2String(json value) {
    // std::cout << "value= " << value.dump() << std::endl;
    if (value.is_number_float())
        return std::to_string(value.get<float>());
    else if (value.is_number_integer())
        return std::to_string(value.get<int>());
    else
        return "'" + value.get<std::string>() + "'";
}

inline std::string Datatype2String(hsql::DataType type) {
    switch (type)
    {
    case hsql::DataType::FLOAT:
        return "float";
        break;
    case hsql::DataType::INT:
        return "integer";
        break;
    case hsql::DataType::TEXT:
        return "text";
        break;
    default:
        throw "Value Error";
        return "";
    }
}

inline std::string ColumnType2String(hsql::DataType type) {
    if (type == hsql::DataType::INT)
        return "integer";
    if (type == hsql::DataType::FLOAT 
        || type == hsql::DataType::DOUBLE 
        || type == hsql::DataType::REAL)
        return "real";
    if (type == hsql::DataType::TEXT)
        return "text";
    throw "Value Error!";
};

};
