#pragma once

#include <sqlite3.h>

#include <cstdint>
#include <string>
#include <vector>

#include "IAuditLogger.hpp"

class AuditLogger : public IAuditLogger
{
public:
    AuditLogger(const std::string& connection_string);
    ~AuditLogger();

    SystemStatus fetch(std::vector<AuditEntry>& entries) override;
    SystemStatus log(Operation operation, DeviceType device, SystemStatus auditResult,
                     std::string options) override;

private:
    sqlite3* m_db = nullptr;
};