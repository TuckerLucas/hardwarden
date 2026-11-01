#pragma once

#include <vector>

#include "AuditTypes.hpp"
#include "SystemStatus.hpp"
#include "Types.hpp"

class IAuditLogger
{
public:
    virtual SystemStatus fetch(std::vector<AuditEntry>& entries) = 0;
    virtual SystemStatus log(Operation operation, DeviceType device, SystemStatus auditResult,
                             std::string options) = 0;

    virtual ~IAuditLogger() = default;
};