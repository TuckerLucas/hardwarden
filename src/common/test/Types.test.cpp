#include "Types.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("requiredSecret")
{
    SECTION("TROPIC01 never needs a secret")
    {
        auto device = GENERATE(DeviceType::TROPIC01_SPI, DeviceType::TROPIC01_USB);
        auto operation =
            GENERATE(Operation::STATUS, Operation::ERASE_KEY, Operation::GENERATE_KEY,
                     Operation::READ_KEY, Operation::LIST_KEYS, Operation::SIGN, Operation::VERIFY);

        REQUIRE(requiredSecret(device, operation) == Secret::NONE);
    }

    SECTION("YubiKey needs the PIN to sign")
    {
        REQUIRE(requiredSecret(DeviceType::YUBIKEY, Operation::SIGN) == Secret::PIN);
    }

    SECTION("YubiKey needs the management key to generate or erase a key")
    {
        auto operation = GENERATE(Operation::GENERATE_KEY, Operation::ERASE_KEY);

        REQUIRE(requiredSecret(DeviceType::YUBIKEY, operation) == Secret::MANAGEMENT_KEY);
    }

    SECTION("YubiKey needs no secret to read public data")
    {
        auto operation = GENERATE(Operation::STATUS, Operation::READ_KEY, Operation::LIST_KEYS,
                                  Operation::VERIFY);

        REQUIRE(requiredSecret(DeviceType::YUBIKEY, operation) == Secret::NONE);
    }

    SECTION("Device-less operations need no secret")
    {
        REQUIRE(requiredSecret(DeviceType::NONE, Operation::LOGS) == Secret::NONE);
        REQUIRE(requiredSecret(DeviceType::NONE, Operation::HELP) == Secret::NONE);
    }
}

TEST_CASE("defaultSecret")
{
    SECTION("YubiKey has factory defaults for the PIN and the management key")
    {
        REQUIRE(defaultSecret(DeviceType::YUBIKEY, Secret::PIN) == "123456");
        REQUIRE(defaultSecret(DeviceType::YUBIKEY, Secret::MANAGEMENT_KEY) ==
                "010203040506070801020304050607080102030405060708");
    }

    SECTION("Devices without secrets have no defaults")
    {
        auto device = GENERATE(DeviceType::TROPIC01_SPI, DeviceType::TROPIC01_USB);
        auto secret = GENERATE(Secret::PIN, Secret::MANAGEMENT_KEY);

        REQUIRE_FALSE(defaultSecret(device, secret).has_value());
    }

    SECTION("No secret has no default")
    {
        REQUIRE_FALSE(defaultSecret(DeviceType::YUBIKEY, Secret::NONE).has_value());
    }
}

TEST_CASE("ed25519MaxMessageSize")
{
    SECTION("TROPIC01 signs Ed25519 messages up to 4096 bytes")
    {
        auto device = GENERATE(DeviceType::TROPIC01_SPI, DeviceType::TROPIC01_USB);

        REQUIRE(ed25519MaxMessageSize(device) == 4096);
    }

    SECTION("YubiKey signs Ed25519 messages up to 3061 bytes")
    {
        REQUIRE(ed25519MaxMessageSize(DeviceType::YUBIKEY) == 3061);
    }
}

TEST_CASE("slotRange")
{
    SECTION("TROPIC01 uses slots 0-31")
    {
        auto device = GENERATE(DeviceType::TROPIC01_SPI, DeviceType::TROPIC01_USB);

        REQUIRE(slotRange(device).min == 0);
        REQUIRE(slotRange(device).max == 31);
    }

    SECTION("YubiKey uses slots 5-24")
    {
        REQUIRE(slotRange(DeviceType::YUBIKEY).min == 5);
        REQUIRE(slotRange(DeviceType::YUBIKEY).max == 24);
    }
}

// These names are written to the audit log, so changing one splits a device's log history.
TEST_CASE("deviceTypeToString")
{
    REQUIRE(deviceTypeToString(DeviceType::TROPIC01_SPI) == "tropic01-spi");
    REQUIRE(deviceTypeToString(DeviceType::TROPIC01_USB) == "tropic01-usb");
    REQUIRE(deviceTypeToString(DeviceType::YUBIKEY) == "yubikey");
    REQUIRE(deviceTypeToString(DeviceType::NONE) == "");
}
