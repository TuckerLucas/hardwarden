#pragma once

#include <vector>

#include "IAuditLogger.hpp"

class MockAuditLogger : public IAuditLogger
{
public:
    SystemStatus fetchResult = SystemStatus::OK;
    SystemStatus logResult = SystemStatus::OK;

    bool logCalled = false;
    Operation lastOperation = Operation::NONE;
    SystemStatus lastSystemStatus = SystemStatus::NOK;
    std::string lastOptions = "";

    SystemStatus fetch(std::vector<AuditEntry>& entries) override
    {
        return fetchResult;
    }

    SystemStatus log(Operation operation, SystemStatus auditResult, std::string options) override
    {
        logCalled = true;
        lastOperation = operation;
        lastSystemStatus = auditResult;
        lastOptions = options;

        return logResult;
    }
};