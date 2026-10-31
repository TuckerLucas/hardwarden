#pragma once

#include <optional>
#include <string>

#include "IDevice.hpp"
#include "Pkcs11.hpp"

class Pkcs11Device : public IDevice
{
public:
    explicit Pkcs11Device(std::string modulePath);
    ~Pkcs11Device() override;

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
    std::string m_modulePath;

    void* m_library = nullptr;            // handle from dlopen
    CK_FUNCTION_LIST_PTR m_fn = nullptr;  // the module's function table
    bool m_initialized = false;           // true after C_Initialize
    CK_SLOT_ID m_slotId = 0;              // the token we talk to
    CK_SESSION_HANDLE m_session = CK_INVALID_HANDLE;

    CK_RV findObjects(CK_OBJECT_CLASS objectClass, std::optional<uint8_t> slot,
                      std::vector<CK_OBJECT_HANDLE>& found);
    CK_RV readPublicKey(CK_OBJECT_HANDLE object, std::vector<uint8_t>& pubKey);

    SystemStatus verifyEd25519(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
                               std::vector<uint8_t> signature);

    SystemStatus verifyP256(std::vector<uint8_t> pubKey, std::vector<uint8_t> payload,
                            std::vector<uint8_t> signature);

    std::vector<uint8_t> rawSignatureToDer(const std::vector<uint8_t>& raw_sig);
};