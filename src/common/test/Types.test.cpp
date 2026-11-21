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
