#pragma once

struct AuditEntry
{
    std::string timestamp;
    std::string device;
    std::string operation;
    std::string auditResult;
    std::string options;
};