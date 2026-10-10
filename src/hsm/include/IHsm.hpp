#pragma once

#include <cstdint>
#include <vector>

#include "HsmTypes.hpp"
#include "SystemStatus.hpp"

class IHsm
{
public:
    virtual SystemStatus init() = 0;
    virtual SystemStatus status() = 0;
    virtual SystemStatus eraseKey(uint8_t slot) = 0;
    virtual SystemStatus generateKey(uint8_t slot, Curve curve) = 0;
    virtual SystemStatus readKey(uint8_t slot, std::vector<uint8_t>& pubKey) = 0;
    virtual SystemStatus listKeys(std::vector<std::vector<uint8_t>>& pubKeys) = 0;
    virtual SystemStatus sign(uint8_t slot, Curve curve, std::vector<uint8_t> payload,
                              std::vector<uint8_t>& signature) = 0;
    virtual SystemStatus verify(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
                                std::vector<uint8_t> signature) = 0;
    virtual SystemStatus deinit() = 0;

    virtual ~IHsm() = default;
};