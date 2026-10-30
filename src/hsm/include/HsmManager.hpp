#pragma once

#include <cstdint>
#include <string>

#include "IAuditLogger.hpp"
#include "IHsm.hpp"

class HsmManager
{
public:
    HsmManager(IHsm& se, IAuditLogger& logger);

    SystemStatus status();
    SystemStatus eraseKey(uint8_t slot);
    SystemStatus generateKey(uint8_t slot, Curve curve);
    SystemStatus readKey(uint8_t slot, std::vector<uint8_t>& pubKey);
    SystemStatus listKeys(std::vector<std::vector<uint8_t>>& pubKeys);
    SystemStatus sign(uint8_t slot, std::vector<uint8_t> payload, std::vector<uint8_t>& signature,
                      DataSource dataSource, std::string filepath = "");
    SystemStatus verifyWithHsmKey(uint8_t slot, std::vector<uint8_t> payload,
                                  std::vector<uint8_t> signature, DataSource dataSource,
                                  std::string filepath = "");

private:
    IHsm& m_se;
    IAuditLogger& m_logger;
};