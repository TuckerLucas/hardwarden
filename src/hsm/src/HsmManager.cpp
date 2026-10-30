#include "HsmManager.hpp"

HsmManager::HsmManager(IHsm& se, IAuditLogger& logger) : m_se(se), m_logger(logger)
{
}

SystemStatus HsmManager::status()
{
    SystemStatus result;
    Operation operation = Operation::STATUS;
    std::string options = "";

    result = m_se.init();

    if (result != SystemStatus::OK)
    {
        m_logger.log(operation, result, options);
        return result;
    }

    result = m_se.status();

    if (result != SystemStatus::OK)
    {
        m_logger.log(operation, result, options);
        return result;
    }

    result = m_se.deinit();

    m_logger.log(operation, result, options);
    return result;
}

SystemStatus HsmManager::eraseKey(uint8_t slot)
{
    SystemStatus result;
    Operation operation = Operation::ERASE_KEY;
    std::string options = "slot=" + std::to_string(slot);

    result = m_se.init();

    if (result != SystemStatus::OK)
    {
        m_logger.log(operation, result, options);
        return result;
    }

    result = m_se.eraseKey(slot);

    if (result != SystemStatus::OK)
    {
        m_logger.log(operation, result, options);
        return result;
    }

    result = m_se.deinit();

    m_logger.log(operation, result, options);
    return result;
}

SystemStatus HsmManager::generateKey(uint8_t slot, Curve curve)
{
    SystemStatus result;
    Operation operation = Operation::GENERATE_KEY;
    std::string curve_str = (curve == Curve::P256) ? "p256" : "ed25519";
    std::string options =
        "slot=" + std::to_string(slot) + (slot < 10 ? "  " : " ") + "curve=" + curve_str;

    result = m_se.init();

    if (result != SystemStatus::OK)
    {
        m_logger.log(operation, result, options);
        return result;
    }

    result = m_se.generateKey(slot, curve);

    if (result != SystemStatus::OK)
    {
        m_logger.log(operation, result, options);
        return result;
    }

    result = m_se.deinit();

    m_logger.log(operation, result, options);
    return result;
}

SystemStatus HsmManager::readKey(uint8_t slot, std::vector<uint8_t>& pubKey)
{
    SystemStatus result;
    std::string options = "slot=" + std::to_string(slot);

    result = m_se.init();

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::READ_KEY, result, options);
        return result;
    }

    result = m_se.readKey(slot, pubKey);

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::READ_KEY, result, options);
        return result;
    }

    result = m_se.deinit();

    m_logger.log(Operation::READ_KEY, result, options);
    return result;
}

SystemStatus HsmManager::listKeys(std::vector<std::vector<uint8_t>>& pubKeys)
{
    SystemStatus result;
    std::string options = "";

    result = m_se.init();

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::LIST_KEYS, result, options);
        return result;
    }

    result = m_se.listKeys(pubKeys);

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::LIST_KEYS, result, options);
        return result;
    }

    result = m_se.deinit();

    m_logger.log(Operation::LIST_KEYS, result, options);
    return result;
}

SystemStatus HsmManager::sign(uint8_t slot, std::vector<uint8_t> payload,
                              std::vector<uint8_t>& signature, DataSource dataSource,
                              std::string filepath)
{
    SystemStatus result;
    std::string options = "slot=" + std::to_string(slot) + (slot < 10 ? "  " : " ") +
                          "type=" + dataSourceToString(dataSource);

    if (dataSource == DataSource::FILE)
    {
        options += " path=" + filepath;
    }

    result = m_se.init();

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::SIGN, result, options);
        return result;
    }

    std::vector<uint8_t> pubKey;

    result = m_se.readKey(slot, pubKey);

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::SIGN, result, options);
        return result;
    }

    Curve curve = (pubKey.size() == 32) ? Curve::Ed25519 : Curve::P256;

    if (curve == Curve::Ed25519 && payload.size() > ED25519_MAX_MSG_SIZE)
    {
        m_logger.log(Operation::SIGN, SystemStatus::HSM_ERROR_SIGN_PAYLOAD_TOO_LARGE, options);
        return SystemStatus::HSM_ERROR_SIGN_PAYLOAD_TOO_LARGE;
    }

    result = m_se.sign(slot, curve, payload, signature);

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::SIGN, result, options);
        return result;
    }

    result = m_se.deinit();

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::SIGN, result, options);
        return result;
    }

    m_logger.log(Operation::SIGN, result, options);
    return result;
}

SystemStatus HsmManager::verifyWithHsmKey(uint8_t slot, std::vector<uint8_t> payload,
                                          std::vector<uint8_t> signature, DataSource dataSource,
                                          std::string filepath)
{
    SystemStatus result;
    std::string options = "slot=" + std::to_string(slot) + (slot < 10 ? "  " : " ") +
                          "type=" + dataSourceToString(dataSource);

    if (dataSource == DataSource::FILE)
    {
        options += " path=" + filepath;
    }

    result = m_se.init();

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::VERIFY, result, options);
        return result;
    }

    std::vector<uint8_t> pubKey;

    result = m_se.readKey(slot, pubKey);

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::VERIFY, result, options);
        return result;
    }

    result = m_se.verify(pubKey, payload, signature);

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::VERIFY, result, options);
        return result;
    }

    result = m_se.deinit();

    if (result != SystemStatus::OK)
    {
        m_logger.log(Operation::VERIFY, result, options);
        return result;
    }

    m_logger.log(Operation::VERIFY, result, options);
    return result;
}