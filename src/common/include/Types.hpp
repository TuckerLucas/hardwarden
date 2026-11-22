#pragma once

#include <cassert>
#include <optional>
#include <string>
#include <unordered_map>

enum class Operation
{
    // Global flags
    HELP,

    // Operations
    STATUS,
    LOGS,
    ERASE_KEY,
    GENERATE_KEY,
    READ_KEY,
    LIST_KEYS,
    SIGN,
    VERIFY,

    NONE,
};

enum class DeviceType
{
    TROPIC01_SPI,
    TROPIC01_USB,
    YUBIKEY,

    NONE,
};

struct SlotRange
{
    unsigned int min;
    unsigned int max;
};

enum class DataSource
{
    DATA,
    FILE
};

inline SlotRange slotRange(DeviceType device)
{
    switch (device)
    {
        case DeviceType::TROPIC01_SPI:
        case DeviceType::TROPIC01_USB:
            return {0, 31};
        case DeviceType::YUBIKEY:
            return {5, 24};
        default:
            assert(false && "Unhandled device value in slotRange()");
            return {0, 0};
    }
}

// The secret a device needs before an operation can run.
enum class Secret
{
    NONE,
    PIN,             // unlocks the use of private keys
    MANAGEMENT_KEY,  // allows changing what is stored in a slot
};

inline Secret requiredSecret(DeviceType device, Operation operation)
{
    switch (device)
    {
        case DeviceType::YUBIKEY:
            switch (operation)
            {
                case Operation::SIGN:
                    return Secret::PIN;
                case Operation::GENERATE_KEY:
                case Operation::ERASE_KEY:
                    return Secret::MANAGEMENT_KEY;
                default:
                    return Secret::NONE;
            }
        default:
            // Other devices, and device-less operations, need no secret.
            return Secret::NONE;
    }
}

// The factory default of a secret, used when the user leaves the prompt empty.
inline std::optional<std::string> defaultSecret(DeviceType device, Secret secret)
{
    if (device != DeviceType::YUBIKEY)
    {
        return std::nullopt;
    }

    switch (secret)
    {
        case Secret::PIN:
            return "123456";
        case Secret::MANAGEMENT_KEY:
            return "010203040506070801020304050607080102030405060708";
        default:
            return std::nullopt;
    }
}

inline std::string deviceTypeToString(DeviceType device)
{
    static const std::unordered_map<DeviceType, std::string> device_map = {
        {DeviceType::TROPIC01_SPI, "tropic01-spi"},
        {DeviceType::TROPIC01_USB, "tropic01-usb"},
        {DeviceType::YUBIKEY, "yubikey"},
        {DeviceType::NONE, ""},
    };

    auto it = device_map.find(device);
    assert(it != device_map.end() && "Unhandled device value in deviceTypeToString()");
    return (it != device_map.end()) ? it->second : "unknown";
}

inline std::string operationToString(Operation operation)
{
    static const std::unordered_map<Operation, std::string> op_map = {
        {Operation::STATUS, "status"},       {Operation::LOGS, "logs"},
        {Operation::ERASE_KEY, "erase-key"}, {Operation::GENERATE_KEY, "generate-key"},
        {Operation::READ_KEY, "read-key"},   {Operation::LIST_KEYS, "list-keys"},
        {Operation::SIGN, "sign"},           {Operation::VERIFY, "verify"},
    };

    auto it = op_map.find(operation);
    assert(it != op_map.end() && "Unhandled operation value in operationToString function");
    return (it != op_map.end()) ? it->second : "UNKNOWN";
}

inline std::string dataSourceToString(DataSource dataSource)
{
    static const std::unordered_map<DataSource, std::string> ss_map = {
        {DataSource::DATA, "data"},
        {DataSource::FILE, "file"},
    };

    auto it = ss_map.find(dataSource);
    assert(it != ss_map.end() && "Unhandled sign source value in dataSourceToString function");
    return (it != ss_map.end()) ? it->second : "UNKNOWN";
}