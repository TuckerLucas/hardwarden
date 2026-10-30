#pragma once

#include <cassert>
#include <string>
#include <unordered_map>

enum class Operation
{
    // Global flags
    HELP,

    // Operations
    STATUS,
    LOGS,
    ERASE_KEY,
    GENERATE_KEY,
    READ_KEY,
    LIST_KEYS,
    SIGN,
    VERIFY,

    NONE,
};

enum class Device
{
    TROPIC01,

    NONE,
};

enum class DataSource
{
    DATA,
    FILE
};

inline std::string operationToString(Operation operation)
{
    static const std::unordered_map<Operation, std::string> op_map = {
        {Operation::STATUS, "status"},       {Operation::LOGS, "logs"},
        {Operation::ERASE_KEY, "erase-key"}, {Operation::GENERATE_KEY, "generate-key"},
        {Operation::READ_KEY, "read-key"},   {Operation::LIST_KEYS, "list-keys"},
        {Operation::SIGN, "sign"},           {Operation::VERIFY, "verify"},
    };

    auto it = op_map.find(operation);
    assert(it != op_map.end() && "Unhandled operation value in operationToString function");
    return (it != op_map.end()) ? it->second : "UNKNOWN";
}

inline std::string dataSourceToString(DataSource dataSource)
{
    static const std::unordered_map<DataSource, std::string> ss_map = {
        {DataSource::DATA, "data"},
        {DataSource::FILE, "file"},
    };

    auto it = ss_map.find(dataSource);
    assert(it != ss_map.end() && "Unhandled sign source value in dataSourceToString function");
    return (it != ss_map.end()) ? it->second : "UNKNOWN";
}