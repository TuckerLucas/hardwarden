#pragma once

#include <cstdint>
#include <string>

#include "IAuditLogger.hpp"
#include "IDevice.hpp"

class DeviceManager
{
public:
    DeviceManager(IDevice& se, IAuditLogger& logger);

    SystemStatus status();
    SystemStatus eraseKey(uint8_t slot);
    SystemStatus generateKey(uint8_t slot, Curve curve);
    SystemStatus readKey(uint8_t slot, std::vector<uint8_t>& pubKey);
    SystemStatus listKeys(std::vector<std::vector<uint8_t>>& pubKeys);
    SystemStatus sign(uint8_t slot, std::vector<uint8_t> payload, std::vector<uint8_t>& signature,
                      DataSource dataSource, std::string filepath = "");
    SystemStatus verify(uint8_t slot, std::vector<uint8_t> payload, std::vector<uint8_t> signature,
                        DataSource dataSource, std::string filepath = "");

private:
    IDevice& m_se;
    IAuditLogger& m_logger;
};