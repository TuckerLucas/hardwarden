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
