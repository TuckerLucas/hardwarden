#pragma once

#include <memory>

#include "IHsm.hpp"

class SecureElement : public IHsm
{
public:
    SecureElement();
    ~SecureElement();

    SystemStatus init() override;
    SystemStatus status() override;
    SystemStatus eraseKey(uint8_t slot) override;
    SystemStatus generateKey(uint8_t slot, Curve curve) override;
    SystemStatus readKey(uint8_t slot, std::vector<uint8_t>& pubKey) override;
    SystemStatus listKeys(std::vector<std::vector<uint8_t>>& pubKeys) override;
    SystemStatus sign(uint8_t slot, Curve curve, std::vector<uint8_t> payload,
                      std::vector<uint8_t>& signature) override;
    SystemStatus verify(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
                        std::vector<uint8_t> signature) override;
    SystemStatus deinit() override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    SystemStatus signEd25519(uint8_t slot, std::vector<uint8_t> payload,
                             std::vector<uint8_t>& signature);

    SystemStatus signP256(uint8_t slot, std::vector<uint8_t> payload,
                          std::vector<uint8_t>& signature);

    SystemStatus verifyEd25519(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
                               std::vector<uint8_t> signature);

    SystemStatus verifyP256(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
                            std::vector<uint8_t> signature);

    std::vector<uint8_t> rawSignatureToDer(const std::vector<uint8_t>& raw_sig);
};