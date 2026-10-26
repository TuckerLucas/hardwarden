#include "CliParser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

#include "CliTestConstants.hpp"

TEST_CASE("CliParser parseCommand")
{
    CliParser parser;

    SECTION("no operation")
    {
        const char* argv[] = {"hsmctl"};

        auto parsed_command = parser.parseCommand(std::size(argv), argv);

        REQUIRE(parsed_command.operation == Operation::NONE);
        REQUIRE(parsed_command.error == ParseError::MISSING_OPERATION);
    }

    SECTION("invalid operation")
    {
        const char* argv[] = {"hsmctl", "invalid-operation"};

        auto result = parser.parseCommand(std::size(argv), argv);

        REQUIRE(result.operation == Operation::NONE);
        REQUIRE(result.error == ParseError::INVALID_OPERATION);
    }

    SECTION("help")
    {
        SECTION("success")
        {
            const char* argv[] = {"hsmctl", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::HELP);
            REQUIRE(parsed_command.error == ParseError::NONE);
        }

        SECTION("invalid option")
        {
            const char* argv[] = {"hsmctl", "--help", "--some-option"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::HELP);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
        }
    }

    SECTION("status")
    {
        SECTION("success")
        {
            const char* argv[] = {"hsmctl", "tropic01", "status"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.device == Device::TROPIC01);
            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help == false);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hsmctl", "status"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.device == Device::NONE);
            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("help")
        {
            const char* argv[] = {"hsmctl", "tropic01", "status", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help == true);
        }

        SECTION("help without device")
        {
            const char* argv[] = {"hsmctl", "status", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
            REQUIRE(parsed_command.requires_help == false);
        }

        SECTION("invalid option")
        {
            const char* argv[] = {"hsmctl", "tropic01", "status", "--some-option"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
        }
    }

    SECTION("logs")
    {
        SECTION("success")
        {
            const char* argv[] = {"hsmctl", "logs"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LOGS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help == false);
        }

        SECTION("help")
        {
            const char* argv[] = {"hsmctl", "logs", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LOGS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help == true);
        }

        SECTION("invalid option")
        {
            const char* argv[] = {"hsmctl", "logs", "--some-option"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LOGS);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
        }
    }

    SECTION("erase key")
    {
        SECTION("success")
        {
            SECTION("non-boundary slot")
            {
                const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--slot", "2"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.device == Device::TROPIC01);
                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.options["slot"] == "2");
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.requires_help == false);
            }

            SECTION("boundary slots")
            {
                SECTION("lower boundary")
                {
                    std::string min_slot_str = std::to_string(MIN_SLOT);
                    const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--slot",
                                          min_slot_str.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                    REQUIRE(parsed_command.options["slot"] == min_slot_str.c_str());
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }

                SECTION("upper boundary")
                {
                    std::string max_slot_str = std::to_string(MAX_SLOT);
                    const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--slot",
                                          max_slot_str.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                    REQUIRE(parsed_command.options["slot"] == max_slot_str.c_str());
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }
            }
        }

        SECTION("help")
        {
            const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hsmctl", "erase-key", "--slot", "2"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("missing option")
        {
            const char* argv[] = {"hsmctl", "tropic01", "erase-key"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--invalid-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
            SECTION("trailing garbage after valid command")
            {
                const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--slot", "0", "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("duplicated slot")
            {
                const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--slot",
                                      "3",      "--slot",   "14"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
        }

        SECTION("missing slot value")
        {
            const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--slot"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
        }

        SECTION("invalid slot value")
        {
            SECTION("not a number")
            {
                const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--slot", "NaN"};

                auto result = parser.parseCommand(std::size(argv), argv);

                REQUIRE(result.operation == Operation::ERASE_KEY);
                REQUIRE(result.error == ParseError::INVALID_VALUE);
            }

            SECTION("out of boundaries")
            {
                const char* argv[] = {"hsmctl", "tropic01", "erase-key", "--slot", "32"};

                auto result = parser.parseCommand(std::size(argv), argv);

                REQUIRE(result.operation == Operation::ERASE_KEY);
                REQUIRE(result.error == ParseError::INVALID_VALUE);
            }
        }
    }

    SECTION("generate key")
    {
        SECTION("success")
        {
            SECTION("unspecified curve")
            {
                const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot", "10"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.device == Device::TROPIC01);
                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.options["slot"] == "10");
                REQUIRE(parsed_command.options["curve"] == "ed25519");
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.requires_help == false);
            }

            SECTION("specified curve")
            {
                SECTION("Ed25519")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot",
                                          "12",     "--curve",  "ed25519"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.options["slot"] == "12");
                    REQUIRE(parsed_command.options["curve"] == "ed25519");
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }

                SECTION("P-256")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot",
                                          "18",     "--curve",  "p256"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.options["slot"] == "18");
                    REQUIRE(parsed_command.options["curve"] == "p256");
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }

                SECTION("reversed operation order")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--curve", "p256",
                                          "--slot", "16"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.options["slot"] == "16");
                    REQUIRE(parsed_command.options["curve"] == "p256");
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }
            }

            SECTION("boundary slots")
            {
                SECTION("lower boundary")
                {
                    std::string min_slot_str = std::to_string(MIN_SLOT);
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot",
                                          min_slot_str.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.options["slot"] == min_slot_str.c_str());
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }

                SECTION("upper boundary")
                {
                    std::string max_slot_str = std::to_string(MAX_SLOT);
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot",
                                          max_slot_str.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.options["slot"] == max_slot_str.c_str());
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }
            }
        }

        SECTION("help")
        {
            const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hsmctl", "generate-key", "--slot", "10"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("missing option")
        {
            SECTION("unspecified curve")
            {
                const char* argv[] = {"hsmctl", "tropic01", "generate-key"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("specified curve")
            {
                const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--curve", "p256"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--invalid-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("trailing garbage after valid command")
            {
                const char* argv[] = {"hsmctl", "tropic01", "generate-key",
                                      "--slot", "0",        "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("duplicated options")
            {
                SECTION("slot")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot", "3",
                                          "--slot", "14"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }

                SECTION("curve")
                {
                    const char* argv[] = {"hsmctl",  "tropic01", "generate-key", "--slot", "3",
                                          "--curve", "ed25519",  "--curve",      "p256"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }
            }
        }

        SECTION("missing value")
        {
            SECTION("slot")
            {
                SECTION("unspecified curve")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                }

                SECTION("slot specified first")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key",
                                          "--slot", "--curve",  "p256"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                }

                SECTION("curve specified first")
                {
                    const char* argv[] = {"hsmctl",  "tropic01", "generate-key",
                                          "--curve", "p256",     "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                }
            }

            SECTION("curve")
            {
                SECTION("slot specified first")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key",
                                          "--slot", "13",       "--curve"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                }

                SECTION("curve specified first")
                {
                    const char* argv[] = {"hsmctl",  "tropic01", "generate-key",
                                          "--curve", "--slot",   "26"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                }
            }
        }

        SECTION("invalid value")
        {
            SECTION("slot")
            {
                SECTION("not a number")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot", "NaN"};

                    auto result = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(result.operation == Operation::GENERATE_KEY);
                    REQUIRE(result.error == ParseError::INVALID_VALUE);
                }

                SECTION("out of boundaries")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot", "32"};

                    auto result = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(result.operation == Operation::GENERATE_KEY);
                    REQUIRE(result.error == ParseError::INVALID_VALUE);
                }
            }

            SECTION("curve")
            {
                const char* argv[] = {"hsmctl", "tropic01", "generate-key", "--slot",
                                      "27",     "--curve",  "invalid_curve"};

                auto result = parser.parseCommand(std::size(argv), argv);

                REQUIRE(result.operation == Operation::GENERATE_KEY);
                REQUIRE(result.error == ParseError::INVALID_VALUE);
            }
        }
    }

    SECTION("read key")
    {
        SECTION("success")
        {
            SECTION("non-boundary slot")
            {
                const char* argv[] = {"hsmctl", "tropic01", "read-key", "--slot", "2"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.device == Device::TROPIC01);
                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.options["slot"] == "2");
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.requires_help == false);
            }

            SECTION("boundary slots")
            {
                SECTION("lower boundary")
                {
                    std::string min_slot_str = std::to_string(MIN_SLOT);
                    const char* argv[] = {"hsmctl", "tropic01", "read-key", "--slot",
                                          min_slot_str.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::READ_KEY);
                    REQUIRE(parsed_command.options["slot"] == min_slot_str.c_str());
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }

                SECTION("upper boundary")
                {
                    std::string max_slot_str = std::to_string(MAX_SLOT);
                    const char* argv[] = {"hsmctl", "tropic01", "read-key", "--slot",
                                          max_slot_str.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::READ_KEY);
                    REQUIRE(parsed_command.options["slot"] == max_slot_str.c_str());
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }
            }
        }

        SECTION("help")
        {
            const char* argv[] = {"hsmctl", "tropic01", "read-key", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::READ_KEY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hsmctl", "read-key", "--slot", "2"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::READ_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("missing option")
        {
            const char* argv[] = {"hsmctl", "tropic01", "read-key"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::READ_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hsmctl", "tropic01", "read-key", "--invalid-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("trailing garbage after valid command")
            {
                const char* argv[] = {"hsmctl", "tropic01", "read-key", "--slot", "0", "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("duplicated slot")
            {
                const char* argv[] = {"hsmctl", "tropic01", "read-key", "--slot",
                                      "3",      "--slot",   "14"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
        }

        SECTION("missing slot value")
        {
            const char* argv[] = {"hsmctl", "tropic01", "read-key", "--slot"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::READ_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
        }

        SECTION("invalid slot value")
        {
            SECTION("not a number")
            {
                const char* argv[] = {"hsmctl", "tropic01", "read-key", "--slot", "NaN"};

                auto result = parser.parseCommand(std::size(argv), argv);

                REQUIRE(result.operation == Operation::READ_KEY);
                REQUIRE(result.error == ParseError::INVALID_VALUE);
            }

            SECTION("out of boundaries")
            {
                const char* argv[] = {"hsmctl", "tropic01", "read-key", "--slot", "32"};

                auto result = parser.parseCommand(std::size(argv), argv);

                REQUIRE(result.operation == Operation::READ_KEY);
                REQUIRE(result.error == ParseError::INVALID_VALUE);
            }
        }
    }

    SECTION("list all keys")
    {
        SECTION("success")
        {
            SECTION("unspecified verbosity")
            {
                const char* argv[] = {"hsmctl", "tropic01", "list-keys"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.device == Device::TROPIC01);
                REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.requires_help == false);
            }

            SECTION("specified verbosity")
            {
                const char* argv[] = {"hsmctl", "tropic01", "list-keys", "--verbose"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
                REQUIRE(parsed_command.requires_help == false);
                REQUIRE(parsed_command.options["verbose"] == "true");
                REQUIRE(parsed_command.error == ParseError::NONE);
            }
        }

        SECTION("help")
        {
            const char* argv[] = {"hsmctl", "tropic01", "list-keys", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hsmctl", "list-keys"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hsmctl", "tropic01", "list-keys", "--some-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("trailing garbage after valid command")
            {
                const char* argv[] = {"hsmctl", "tropic01", "list-keys", "--verbose", "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
        }
    }

    SECTION("sign")
    {
        SECTION("success")
        {
            SECTION("data")
            {
                SECTION("slot specified first")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot",
                                          "3",      "--data",   "data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.device == Device::TROPIC01);
                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                    REQUIRE(parsed_command.options["slot"] == "3");
                    REQUIRE(parsed_command.options["data"] == "data");
                    REQUIRE(parsed_command.options["file"].empty());
                }

                SECTION("slot specified last")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--data",
                                          "data",   "--slot",   "3"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                    REQUIRE(parsed_command.options["data"] == "data");
                    REQUIRE(parsed_command.options["slot"] == "3");
                    REQUIRE(parsed_command.options["file"].empty());
                }
            }

            SECTION("file")
            {
                SECTION("slot specified first")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign",    "--slot",
                                          "3",      "--file",   "file.pdf"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                    REQUIRE(parsed_command.options["slot"] == "3");
                    REQUIRE(parsed_command.options["file"] == "file.pdf");
                    REQUIRE(parsed_command.options["data"].empty());
                }

                SECTION("slot specified last")
                {
                    const char* argv[] = {"hsmctl",   "tropic01", "sign", "--file",
                                          "file.pdf", "--slot",   "3"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                    REQUIRE(parsed_command.options["file"] == "file.pdf");
                    REQUIRE(parsed_command.options["slot"] == "3");
                    REQUIRE(parsed_command.options["data"].empty());
                }
            }

            SECTION("boundary slots")
            {
                SECTION("lower boundary")
                {
                    const char* argv[] = {
                        "hsmctl", "tropic01", "sign", "--slot", "0", "--file", "file.pdf",
                    };

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                    REQUIRE(parsed_command.options["slot"] == "0");
                    REQUIRE(parsed_command.options["file"] == "file.pdf");
                    REQUIRE(parsed_command.options["data"].empty());
                }

                SECTION("upper boundary")
                {
                    const char* argv[] = {
                        "hsmctl", "tropic01", "sign", "--slot", "31", "--data", "data",
                    };

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                    REQUIRE(parsed_command.options["slot"] == "31");
                    REQUIRE(parsed_command.options["data"] == "data");
                    REQUIRE(parsed_command.options["file"].empty());
                }
            }
        }

        SECTION("help")
        {
            SECTION("success")
            {
                const char* argv[] = {"hsmctl", "tropic01", "sign", "--help"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.requires_help);
            }

            SECTION("trailing garbage")
            {
                const char* argv[] = {"hsmctl", "tropic01", "sign", "--help", "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                REQUIRE(parsed_command.requires_help == false);
            }
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hsmctl", "sign", "--slot", "3", "--data", "data"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::SIGN);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("missing option")
        {
            SECTION("unspecified slot and data/file")
            {
                const char* argv[] = {"hsmctl", "tropic01", "sign"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("unspecified slot")
            {
                SECTION("specified data")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--data", "data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
                    REQUIRE(parsed_command.options["data"] == "data");
                    REQUIRE(parsed_command.options["slot"].empty());
                }

                SECTION("specified file")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--file", "file.pdf"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
                    REQUIRE(parsed_command.options["file"] == "file.pdf");
                    REQUIRE(parsed_command.options["slot"].empty());
                }
            }

            SECTION("unspecified data/file")
            {
                const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot", "10"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
                REQUIRE(parsed_command.options["slot"] == "10");
            }
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hsmctl", "tropic01", "sign", "--invalid-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("duplicated options")
            {
                SECTION("slot")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot", "26", "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                    REQUIRE(parsed_command.options["slot"] == "26");
                }

                SECTION("data")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign",   "--slot", "26",
                                          "--data", "data1",    "--data", "data2"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                    REQUIRE(parsed_command.options["slot"] == "26");
                    REQUIRE(parsed_command.options["data"] == "data1");
                }

                SECTION("file")
                {
                    const char* argv[] = {"hsmctl", "tropic01",  "sign",   "--slot",   "26",
                                          "--file", "file1.pdf", "--file", "file2.pdf"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                    REQUIRE(parsed_command.options["slot"] == "26");
                    REQUIRE(parsed_command.options["file"] == "file1.pdf");
                }

                SECTION("data and file used in same command")
                {
                    SECTION("data specified first")
                    {
                        const char* argv[] = {"hsmctl", "tropic01", "sign",   "--slot",  "26",
                                              "--data", "data",     "--file", "file.pdf"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["slot"] == "26");
                        REQUIRE(parsed_command.options["data"] == "data");
                        REQUIRE(parsed_command.options["file"].empty());
                    }

                    SECTION("file specified first")
                    {
                        const char* argv[] = {"hsmctl", "tropic01", "sign",   "--slot", "26",
                                              "--file", "file.pdf", "--data", "data"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["slot"] == "26");
                        REQUIRE(parsed_command.options["file"] == "file.pdf");
                        REQUIRE(parsed_command.options["data"].empty());
                    }
                }
            }

            SECTION("trailing garbage after valid command")
            {
                SECTION("data")
                {
                    SECTION("slot specified first")
                    {
                        const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot",
                                              "26",     "--data",   "data", "garbage"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["slot"] == "26");
                        REQUIRE(parsed_command.options["data"] == "data");
                    }

                    SECTION("slot specified last")
                    {
                        const char* argv[] = {"hsmctl", "tropic01", "sign", "--data",
                                              "data",   "--slot",   "26",   "garbage"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["data"] == "data");
                        REQUIRE(parsed_command.options["slot"] == "26");
                    }
                }

                SECTION("file")
                {
                    SECTION("slot specified first")
                    {
                        const char* argv[] = {"hsmctl", "tropic01", "sign",     "--slot",
                                              "26",     "--file",   "file.pdf", "garbage"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["slot"] == "26");
                        REQUIRE(parsed_command.options["file"] == "file.pdf");
                    }

                    SECTION("slot specified last")
                    {
                        const char* argv[] = {"hsmctl",   "tropic01", "sign", "--file",
                                              "file.pdf", "--slot",   "26",   "garbage"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["file"] == "file.pdf");
                        REQUIRE(parsed_command.options["slot"] == "26");
                    }
                }
            }
        }

        SECTION("missing value")
        {
            SECTION("slot")
            {
                SECTION("unspecified data/file")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"].empty());
                }

                SECTION("data specified")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot", "--data", "data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"].empty());
                    REQUIRE(parsed_command.options["data"].empty());
                }

                SECTION("file specified")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign",
                                          "--slot", "--file",   "file.pdf"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"].empty());
                    REQUIRE(parsed_command.options["file"].empty());
                }

                SECTION("data specified first")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--data", "data", "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["data"] == "data");
                    REQUIRE(parsed_command.options["slot"].empty());
                }

                SECTION("file specified first")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign",
                                          "--file", "file.pdf", "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["file"] == "file.pdf");
                    REQUIRE(parsed_command.options["slot"].empty());
                }
            }

            SECTION("data")
            {
                SECTION("unspecified slot")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["data"].empty());
                    REQUIRE(parsed_command.options["slot"].empty());
                }

                SECTION("slot specified first")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot", "12", "--data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"] == "12");
                    REQUIRE(parsed_command.options["data"].empty());
                }

                SECTION("slot specified last")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--data", "--slot", "12"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["data"].empty());
                    REQUIRE(parsed_command.options["slot"].empty());
                }
            }

            SECTION("file")
            {
                SECTION("unspecified slot")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--file"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"].empty());
                    REQUIRE(parsed_command.options["file"].empty());
                }

                SECTION("slot specified first")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot", "15", "--file"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"] == "15");
                    REQUIRE(parsed_command.options["file"].empty());
                }

                SECTION("slot specified last")
                {
                    const char* argv[] = {"hsmctl", "tropic01", "sign", "--file", "--slot", "15"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["file"].empty());
                    REQUIRE(parsed_command.options["slot"].empty());
                }
            }
        }

        SECTION("invalid value")
        {
            SECTION("out of boundaries")
            {
                const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot", "32"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                REQUIRE(parsed_command.options["slot"].empty());
            }

            SECTION("not a number")
            {
                const char* argv[] = {"hsmctl", "tropic01", "sign", "--slot", "NaN"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                REQUIRE(parsed_command.options["slot"].empty());
            }
        }
    }
}

// Possible commands:
//
// hsmctl verify --slot <valid slot> --data <data> --signature <valid signature> ✓
// hsmctl verify --slot <valid slot> --file <valid file> --signature <valid signature> ✓
// hsmctl verify --pubkey <valid key> --data <data> --signature <valid signature> ✓
// hsmctl verify --pubkey <valid key> --file <valid file> --signature <valid signature> ✓
// hsmctl verify --signature <valid signature> --file <valid file> --pubkey <valid key> ✓
// hsmctl verify --data <data> --slot <valid slot> --signature <valid signature> ✓
// hsmctl verify --slot <valid slot> --signature <valid signature> --file <file> ✓
//
// hsmctl verify ✓
// hsmctl verify --unknown-option ✓
//
// hsmctl verify --help ✓
// hsmctl verify --help unexpected-argument ✓
//
// hsmctl verify --slot ✓
// hsmctl verify --pubkey ✓
// hsmctl verify --data ✓
// hsmctl verify --file ✓
// hsmctl verify --signature ✓
//
// hsmctl verify --slot <valid slot> --slot <valid slot> ✓
// hsmctl verify --pubkey <valid key> --pubkey <valid key> ✓
// hsmctl verify --data <data> --data <data> ✓
// hsmctl verify --file <valid file> --file <valid file> ✓
// hsmctl verify --signature <valid signature> --signature <valid signature> ✓
//
// hsmctl verify --slot NaN ✓
// hsmctl verify --slot <out of boundary slot> ✓
// hsmctl verify --slot --data <data> ✓
//
// hsmctl verify --pubkey <non 32 or 64 byte key> ✓
// hsmctl verify --pubkey <invalid 32 byte key> ✓
// hsmctl verify --pubkey <invalid 64 byte key> ✓
// hsmctl verify --pubkey --file <valid file> ✓
//
// hsmctl verify --data --pubkey <valid key> ✓
//
// hsmctl verify --file --pubkey <valid key> ✓
//
// hsmctl verify --signature <non 64 byte signature> ✓
// hsmctl verify --signature <invalid 64 byte signature> ✓
// hsmctl verify --signature --slot <valid slot> ✓
//
// hsmctl verify --slot <valid slot> --pubkey <valid key> ✓
// hsmctl verify --pubkey <valid key> --slot <valid slot> ✓
// hsmctl verify --data <data> --file <valid file> ✓
// hsmctl verify --file <valid file> --data <data> ✓
//
// hsmctl verify --data <data> --signature <valid signature> ✓
// hsmctl verify --file <valid file> --signature <valid signature> ✓
// hsmctl verify --slot <valid slot> --signature <valid signature> ✓
// hsmctl verify --pubkey <valid key> --signature <valid signature> ✓
// hsmctl verify --slot <valid slot> --data <data> ✓
// hsmctl verify --slot <valid slot> --file <valid file> ✓
// hsmctl verify --pubkey <valid key> --data <data> ✓
// hsmctl verify --pubkey <valid key> --file <valid file> ✓
//
// hsmctl verify --slot <valid slot> --data <data> --signature <valid signature> unexpected-argument
// ✓
//
TEST_CASE("verify")
{
    CliParser parser;

    SECTION("success")
    {
        SECTION("slot and data specified")
        {
            const char* argv[] = {"hsmctl", "verify", "--slot",      "25",
                                  "--data", "data",   "--signature", valid_signature.c_str()};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.options["slot"] == "25");
            REQUIRE(parsed_command.options["data"] == "data");
            REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

            REQUIRE(parsed_command.options["pubkey"].empty());
            REQUIRE(parsed_command.options["file"].empty());
        }

        SECTION("slot and file specified")
        {
            const char* argv[] = {"hsmctl", "verify",   "--slot",      "25",
                                  "--file", "file.txt", "--signature", valid_signature.c_str()};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.options["slot"] == "25");
            REQUIRE(parsed_command.options["file"] == "file.txt");
            REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

            REQUIRE(parsed_command.options["pubkey"].empty());
            REQUIRE(parsed_command.options["data"].empty());
        }

        SECTION("public key and data specified")
        {
            const char* argv[] = {"hsmctl", "verify", "--pubkey",    valid_pubkey_32_bytes.c_str(),
                                  "--data", "data",   "--signature", valid_signature.c_str()};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.options["pubkey"] == valid_pubkey_32_bytes.c_str());
            REQUIRE(parsed_command.options["data"] == "data");
            REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

            REQUIRE(parsed_command.options["slot"].empty());
            REQUIRE(parsed_command.options["file"].empty());
        }

        SECTION("public key and file specified")
        {
            const char* argv[] = {
                "hsmctl", "verify",   "--pubkey",    valid_pubkey_32_bytes.c_str(),
                "--file", "file.txt", "--signature", valid_signature.c_str()};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.options["pubkey"] == valid_pubkey_32_bytes.c_str());
            REQUIRE(parsed_command.options["file"] == "file.txt");
            REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

            REQUIRE(parsed_command.options["slot"].empty());
            REQUIRE(parsed_command.options["data"].empty());
        }

        SECTION("interchangeable argument order")
        {
            // Did not test all possible order combinations
            // since it seems irrelevant to do so.
            // 3 should be enough.
            SECTION("signature | file | public key")
            {
                const char* argv[] = {
                    "hsmctl", "verify",   "--signature", valid_signature.c_str(),
                    "--file", "file.txt", "--pubkey",    valid_pubkey_32_bytes.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.options["pubkey"] == valid_pubkey_32_bytes.c_str());
                REQUIRE(parsed_command.options["file"] == "file.txt");
                REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

                REQUIRE(parsed_command.options["slot"].empty());
                REQUIRE(parsed_command.options["data"].empty());
            }

            SECTION("data | slot | signature")
            {
                const char* argv[] = {"hsmctl", "verify", "--data",      "data",
                                      "--slot", "25",     "--signature", valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.options["slot"] == "25");
                REQUIRE(parsed_command.options["data"] == "data");
                REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

                REQUIRE(parsed_command.options["pubkey"].empty());
                REQUIRE(parsed_command.options["file"].empty());
            }

            SECTION("slot | signature | file")
            {
                const char* argv[] = {"hsmctl", "verify",      "--slot",
                                      "25",     "--signature", valid_signature.c_str(),
                                      "--file", "file.txt"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.options["slot"] == "25");
                REQUIRE(parsed_command.options["file"] == "file.txt");
                REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

                REQUIRE(parsed_command.options["pubkey"].empty());
                REQUIRE(parsed_command.options["data"].empty());
            }
        }
    }

    SECTION("help")
    {
        SECTION("success")
        {
            const char* argv[] = {"hsmctl", "verify", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("unexpected argument")
        {
            const char* argv[] = {"hsmctl", "verify", "--help", "unexpected-argument"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            REQUIRE(parsed_command.requires_help == false);
        }
    }

    SECTION("invalid option")
    {
        SECTION("unknown option")
        {
            const char* argv[] = {"hsmctl", "verify", "--unknown-option"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
        }

        SECTION("duplicated option")
        {
            SECTION("slot")
            {
                const char* argv[] = {"hsmctl", "verify", "--slot", "3", "--slot", "4"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("public key")
            {
                const char* argv[] = {"hsmctl",   "verify",
                                      "--pubkey", valid_pubkey_32_bytes.c_str(),
                                      "--pubkey", valid_pubkey_64_bytes.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("data")
            {
                const char* argv[] = {"hsmctl", "verify", "--data", "data", "--data", "more-data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("file")
            {
                const char* argv[] = {"hsmctl",        "verify", "--file",
                                      "some_file.txt", "--file", "another-file.txt"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("signature")
            {
                const char* argv[] = {"hsmctl",      "verify",
                                      "--signature", valid_signature.c_str(),
                                      "--signature", valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
        }

        SECTION("mutually exclusive options")
        {
            SECTION("slot and public key")
            {
                SECTION("slot specified first")
                {
                    const char* argv[] = {"hsmctl", "verify",   "--slot",
                                          "5",      "--pubkey", valid_pubkey_32_bytes.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::VERIFY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }

                SECTION("public key specified first")
                {
                    const char* argv[] = {"hsmctl",   "verify",
                                          "--pubkey", valid_pubkey_32_bytes.c_str(),
                                          "--slot",   "5"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::VERIFY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }
            }

            SECTION("data and file")
            {
                SECTION("data specified first")
                {
                    const char* argv[] = {"hsmctl", "verify", "--data",
                                          "data",   "--file", "file.txt"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::VERIFY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }

                SECTION("file specified first")
                {
                    const char* argv[] = {"hsmctl",   "verify", "--file",
                                          "file.txt", "--data", "data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::VERIFY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }
            }
        }

        SECTION("unexpected argument after valid command")
        {
            const char* argv[] = {"hsmctl",
                                  "verify",
                                  "--slot",
                                  "12",
                                  "--file",
                                  "file.txt",
                                  "--signature",
                                  valid_signature.c_str(),
                                  "unexpected-argument"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
        }
    }

    SECTION("missing option")
    {
        SECTION("no option given")
        {
            const char* argv[] = {"hsmctl", "verify"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
        }

        SECTION("missing slot / public key")
        {
            SECTION("data specified")
            {
                const char* argv[] = {"hsmctl", "verify",      "--data",
                                      "data",   "--signature", valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("file specified")
            {
                const char* argv[] = {"hsmctl",   "verify",      "--file",
                                      "file.txt", "--signature", valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }
        }

        SECTION("missing data / file")
        {
            SECTION("slot specified")
            {
                const char* argv[] = {"hsmctl", "verify",      "--slot",
                                      "18",     "--signature", valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("public key specified")
            {
                const char* argv[] = {"hsmctl",      "verify",
                                      "--pubkey",    valid_pubkey_32_bytes.c_str(),
                                      "--signature", valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }
        }

        SECTION("missing signature")
        {
            SECTION("slot and data specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--slot", "15", "--data", "data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("slot and file specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--slot", "15", "--file", "file.txt"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("public key and data specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--pubkey", valid_pubkey_32_bytes.c_str(),
                                      "--data", "data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("public key and file specified")
            {
                const char* argv[] = {"hsmctl",   "verify",
                                      "--pubkey", valid_pubkey_32_bytes.c_str(),
                                      "--file",   "file.txt"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }
        }
    }

    SECTION("missing value")
    {
        SECTION("slot")
        {
            SECTION("other options not specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--slot"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--slot", "--data", "data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }
        }

        SECTION("public key")
        {
            SECTION("other options not specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--pubkey"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--pubkey", "--file", "file.txt"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }
        }

        SECTION("data")
        {
            SECTION("other options not specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--data", "--pubkey",
                                      valid_pubkey_32_bytes.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }
        }

        SECTION("file")
        {
            SECTION("other options not specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--file"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--file", "--slot", "5"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }
        }

        SECTION("signature")
        {
            SECTION("other options not specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--signature"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hsmctl", "verify", "--signature", "--data", "some-data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }
        }
    }

    SECTION("invalid value")
    {
        SECTION("slot")
        {
            SECTION("not a number")
            {
                const char* argv[] = {"hsmctl", "verify", "--slot", "NaN"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                REQUIRE(parsed_command.options["slot"].empty());
            }

            SECTION("out of boundaries")
            {
                const char* argv[] = {"hsmctl", "verify", "--slot", "32"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                REQUIRE(parsed_command.options["slot"].empty());
            }
        }

        SECTION("public key")
        {
            SECTION("invalid key length")
            {
                const char* argv[] = {"hsmctl", "verify", "--pubkey",
                                      invalid_pubkey_33_bytes.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }

            SECTION("invalid key content")
            {
                SECTION("32 byte key")
                {
                    const char* argv[] = {"hsmctl", "verify", "--pubkey",
                                          invalid_pubkey_32_bytes.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::VERIFY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                }

                SECTION("64 byte key")
                {
                    const char* argv[] = {"hsmctl", "verify", "--pubkey",
                                          invalid_pubkey_64_bytes.c_str()};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::VERIFY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                }
            }
        }

        SECTION("signature")
        {
            SECTION("invalid signature length")
            {
                const char* argv[] = {"hsmctl", "verify", "--signature",
                                      invalid_signature_65_bytes.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }

            SECTION("invalid signature content")
            {
                const char* argv[] = {"hsmctl", "verify", "--signature",
                                      invalid_signature_64_bytes.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }
        }
    }
}
