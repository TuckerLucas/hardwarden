#include "CliParser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

#include "CliTestConstants.hpp"

TEST_CASE("CliParser parseCommand")
{
    CliParser parser;

    SECTION("no operation")
    {
        const char* argv[] = {"hwarden"};

        auto parsed_command = parser.parseCommand(std::size(argv), argv);

        REQUIRE(parsed_command.operation == Operation::NONE);
        REQUIRE(parsed_command.error == ParseError::MISSING_OPERATION);
    }

    SECTION("device without operation")
    {
        const char* argv[] = {"hwarden", "tropic01"};

        auto parsed_command = parser.parseCommand(std::size(argv), argv);

        REQUIRE(parsed_command.device == DeviceType::TROPIC01);
        REQUIRE(parsed_command.operation == Operation::NONE);
        REQUIRE(parsed_command.error == ParseError::MISSING_OPERATION);
    }

    SECTION("device help")
    {
        const char* argv[] = {"hwarden", "tropic01", "--help"};

        auto parsed_command = parser.parseCommand(std::size(argv), argv);

        REQUIRE(parsed_command.device == DeviceType::TROPIC01);
        REQUIRE(parsed_command.operation == Operation::HELP);
        REQUIRE(parsed_command.error == ParseError::NONE);
    }

    SECTION("unknown device")
    {
        const char* argv[] = {"hwarden", "tropic1", "status"};

        auto parsed_command = parser.parseCommand(std::size(argv), argv);

        REQUIRE(parsed_command.device == DeviceType::NONE);
        REQUIRE(parsed_command.error == ParseError::INVALID_OPERATION);
    }

    SECTION("device with invalid operation")
    {
        const char* argv[] = {"hwarden", "tropic01", "invalid-operation"};

        auto parsed_command = parser.parseCommand(std::size(argv), argv);

        REQUIRE(parsed_command.device == DeviceType::TROPIC01);
        REQUIRE(parsed_command.operation == Operation::NONE);
        REQUIRE(parsed_command.error == ParseError::INVALID_OPERATION);
    }

    SECTION("invalid operation")
    {
        const char* argv[] = {"hwarden", "invalid-operation"};

        auto parsed_command = parser.parseCommand(std::size(argv), argv);

        REQUIRE(parsed_command.operation == Operation::NONE);
        REQUIRE(parsed_command.error == ParseError::INVALID_OPERATION);
    }

    SECTION("help")
    {
        SECTION("success")
        {
            const char* argv[] = {"hwarden", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::HELP);
            REQUIRE(parsed_command.error == ParseError::NONE);
        }

        SECTION("invalid option")
        {
            const char* argv[] = {"hwarden", "--help", "--some-option"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::HELP);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
        }
    }

    SECTION("status")
    {
        SECTION("success")
        {
            const char* argv[] = {"hwarden", "tropic01", "status"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.device == DeviceType::TROPIC01);
            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help == false);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hwarden", "status"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.device == DeviceType::NONE);
            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("help")
        {
            const char* argv[] = {"hwarden", "tropic01", "status", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help == true);
        }

        SECTION("help without device")
        {
            const char* argv[] = {"hwarden", "status", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
            REQUIRE(parsed_command.requires_help == false);
        }

        SECTION("invalid option")
        {
            const char* argv[] = {"hwarden", "tropic01", "status", "--some-option"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::STATUS);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
        }
    }

    SECTION("logs")
    {
        SECTION("success")
        {
            const char* argv[] = {"hwarden", "logs"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.device == DeviceType::NONE);
            REQUIRE(parsed_command.operation == Operation::LOGS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help == false);
        }

        SECTION("help")
        {
            const char* argv[] = {"hwarden", "logs", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LOGS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help == true);
        }

        SECTION("unexpected device")
        {
            const char* argv[] = {"hwarden", "tropic01", "logs"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LOGS);
            REQUIRE(parsed_command.error == ParseError::UNEXPECTED_DEVICE);
        }

        SECTION("invalid option")
        {
            const char* argv[] = {"hwarden", "logs", "--some-option"};

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
                const char* argv[] = {"hwarden", "tropic01", "erase-key", "--slot", "2"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.device == DeviceType::TROPIC01);
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
                    const char* argv[] = {"hwarden", "tropic01", "erase-key", "--slot",
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
                    const char* argv[] = {"hwarden", "tropic01", "erase-key", "--slot",
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
            const char* argv[] = {"hwarden", "tropic01", "erase-key", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hwarden", "erase-key", "--slot", "2"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("missing option")
        {
            const char* argv[] = {"hwarden", "tropic01", "erase-key"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hwarden", "tropic01", "erase-key", "--invalid-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
            SECTION("trailing garbage after valid command")
            {
                const char* argv[] = {"hwarden", "tropic01", "erase-key", "--slot", "0", "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("duplicated slot")
            {
                const char* argv[] = {"hwarden", "tropic01", "erase-key", "--slot",
                                      "3",       "--slot",   "14"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
        }

        SECTION("missing slot value")
        {
            const char* argv[] = {"hwarden", "tropic01", "erase-key", "--slot"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
        }

        SECTION("invalid slot value")
        {
            SECTION("not a number")
            {
                const char* argv[] = {"hwarden", "tropic01", "erase-key", "--slot", "NaN"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }

            SECTION("out of boundaries")
            {
                const char* argv[] = {"hwarden", "tropic01", "erase-key", "--slot", "32"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::ERASE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }
        }
    }

    SECTION("generate key")
    {
        SECTION("success")
        {
            SECTION("unspecified curve")
            {
                const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot", "10"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.device == DeviceType::TROPIC01);
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
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot",
                                          "12",      "--curve",  "ed25519"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.options["slot"] == "12");
                    REQUIRE(parsed_command.options["curve"] == "ed25519");
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }

                SECTION("P-256")
                {
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot",
                                          "18",      "--curve",  "p256"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.options["slot"] == "18");
                    REQUIRE(parsed_command.options["curve"] == "p256");
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                }

                SECTION("reversed operation order")
                {
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--curve", "p256",
                                          "--slot",  "16"};

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
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot",
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
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot",
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
            const char* argv[] = {"hwarden", "tropic01", "generate-key", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hwarden", "generate-key", "--slot", "10"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("missing option")
        {
            SECTION("unspecified curve")
            {
                const char* argv[] = {"hwarden", "tropic01", "generate-key"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("specified curve")
            {
                const char* argv[] = {"hwarden", "tropic01", "generate-key", "--curve", "p256"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hwarden", "tropic01", "generate-key", "--invalid-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("trailing garbage after valid command")
            {
                const char* argv[] = {"hwarden", "tropic01", "generate-key",
                                      "--slot",  "0",        "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("duplicated options")
            {
                SECTION("slot")
                {
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot", "3",
                                          "--slot",  "14"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }

                SECTION("curve")
                {
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot", "3",
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
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                }

                SECTION("slot specified first")
                {
                    const char* argv[] = {"hwarden", "tropic01", "generate-key",
                                          "--slot",  "--curve",  "p256"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                }

                SECTION("curve specified first")
                {
                    const char* argv[] = {"hwarden", "tropic01", "generate-key",
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
                    const char* argv[] = {"hwarden", "tropic01", "generate-key",
                                          "--slot",  "13",       "--curve"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                }

                SECTION("curve specified first")
                {
                    const char* argv[] = {"hwarden", "tropic01", "generate-key",
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
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot", "NaN"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                }

                SECTION("out of boundaries")
                {
                    const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot", "32"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                }
            }

            SECTION("curve")
            {
                const char* argv[] = {"hwarden", "tropic01", "generate-key", "--slot",
                                      "27",      "--curve",  "invalid_curve"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::GENERATE_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }
        }
    }

    SECTION("read key")
    {
        SECTION("success")
        {
            SECTION("non-boundary slot")
            {
                const char* argv[] = {"hwarden", "tropic01", "read-key", "--slot", "2"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.device == DeviceType::TROPIC01);
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
                    const char* argv[] = {"hwarden", "tropic01", "read-key", "--slot",
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
                    const char* argv[] = {"hwarden", "tropic01", "read-key", "--slot",
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
            const char* argv[] = {"hwarden", "tropic01", "read-key", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::READ_KEY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hwarden", "read-key", "--slot", "2"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::READ_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("missing option")
        {
            const char* argv[] = {"hwarden", "tropic01", "read-key"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::READ_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hwarden", "tropic01", "read-key", "--invalid-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("trailing garbage after valid command")
            {
                const char* argv[] = {"hwarden", "tropic01", "read-key", "--slot", "0", "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("duplicated slot")
            {
                const char* argv[] = {"hwarden", "tropic01", "read-key", "--slot",
                                      "3",       "--slot",   "14"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
        }

        SECTION("missing slot value")
        {
            const char* argv[] = {"hwarden", "tropic01", "read-key", "--slot"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::READ_KEY);
            REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
        }

        SECTION("invalid slot value")
        {
            SECTION("not a number")
            {
                const char* argv[] = {"hwarden", "tropic01", "read-key", "--slot", "NaN"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }

            SECTION("out of boundaries")
            {
                const char* argv[] = {"hwarden", "tropic01", "read-key", "--slot", "32"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::READ_KEY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }
        }
    }

    SECTION("list all keys")
    {
        SECTION("success")
        {
            SECTION("unspecified verbosity")
            {
                const char* argv[] = {"hwarden", "tropic01", "list-keys"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.device == DeviceType::TROPIC01);
                REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.requires_help == false);
            }

            SECTION("specified verbosity")
            {
                const char* argv[] = {"hwarden", "tropic01", "list-keys", "--verbose"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
                REQUIRE(parsed_command.requires_help == false);
                REQUIRE(parsed_command.options["verbose"] == "true");
                REQUIRE(parsed_command.error == ParseError::NONE);
            }
        }

        SECTION("help")
        {
            const char* argv[] = {"hwarden", "tropic01", "list-keys", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hwarden", "list-keys"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("invalid option")
        {
            SECTION("unknown option")
            {
                const char* argv[] = {"hwarden", "tropic01", "list-keys", "--some-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::LIST_KEYS);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("trailing garbage after valid command")
            {
                const char* argv[] = {"hwarden", "tropic01", "list-keys", "--verbose", "garbage"};

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
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--slot",
                                          "3",       "--data",   "data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.device == DeviceType::TROPIC01);
                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::NONE);
                    REQUIRE(parsed_command.requires_help == false);
                    REQUIRE(parsed_command.options["slot"] == "3");
                    REQUIRE(parsed_command.options["data"] == "data");
                    REQUIRE(parsed_command.options["file"].empty());
                }

                SECTION("slot specified last")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--data",
                                          "data",    "--slot",   "3"};

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
                    const char* argv[] = {"hwarden", "tropic01", "sign",    "--slot",
                                          "3",       "--file",   "file.pdf"};

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
                    const char* argv[] = {"hwarden",  "tropic01", "sign", "--file",
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
                        "hwarden", "tropic01", "sign", "--slot", "0", "--file", "file.pdf",
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
                        "hwarden", "tropic01", "sign", "--slot", "31", "--data", "data",
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
                const char* argv[] = {"hwarden", "tropic01", "sign", "--help"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.requires_help);
            }

            SECTION("trailing garbage")
            {
                const char* argv[] = {"hwarden", "tropic01", "sign", "--help", "garbage"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                REQUIRE(parsed_command.requires_help == false);
            }
        }

        SECTION("missing device")
        {
            const char* argv[] = {"hwarden", "sign", "--slot", "3", "--data", "data"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::SIGN);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
        }

        SECTION("missing option")
        {
            SECTION("unspecified slot and data/file")
            {
                const char* argv[] = {"hwarden", "tropic01", "sign"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("unspecified slot")
            {
                SECTION("specified data")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--data", "data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
                    REQUIRE(parsed_command.options["data"] == "data");
                    REQUIRE(parsed_command.options["slot"].empty());
                }

                SECTION("specified file")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--file", "file.pdf"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
                    REQUIRE(parsed_command.options["file"] == "file.pdf");
                    REQUIRE(parsed_command.options["slot"].empty());
                }
            }

            SECTION("unspecified data/file")
            {
                const char* argv[] = {"hwarden", "tropic01", "sign", "--slot", "10"};

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
                const char* argv[] = {"hwarden", "tropic01", "sign", "--invalid-option"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("duplicated options")
            {
                SECTION("slot")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--slot", "26", "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                    REQUIRE(parsed_command.options["slot"] == "26");
                }

                SECTION("data")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign",   "--slot", "26",
                                          "--data",  "data1",    "--data", "data2"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                    REQUIRE(parsed_command.options["slot"] == "26");
                    REQUIRE(parsed_command.options["data"] == "data1");
                }

                SECTION("file")
                {
                    const char* argv[] = {"hwarden", "tropic01",  "sign",   "--slot",   "26",
                                          "--file",  "file1.pdf", "--file", "file2.pdf"};

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
                        const char* argv[] = {"hwarden", "tropic01", "sign",   "--slot",  "26",
                                              "--data",  "data",     "--file", "file.pdf"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["slot"] == "26");
                        REQUIRE(parsed_command.options["data"] == "data");
                        REQUIRE(parsed_command.options["file"].empty());
                    }

                    SECTION("file specified first")
                    {
                        const char* argv[] = {"hwarden", "tropic01", "sign",   "--slot", "26",
                                              "--file",  "file.pdf", "--data", "data"};

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
                        const char* argv[] = {"hwarden", "tropic01", "sign", "--slot",
                                              "26",      "--data",   "data", "garbage"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["slot"] == "26");
                        REQUIRE(parsed_command.options["data"] == "data");
                    }

                    SECTION("slot specified last")
                    {
                        const char* argv[] = {"hwarden", "tropic01", "sign", "--data",
                                              "data",    "--slot",   "26",   "garbage"};

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
                        const char* argv[] = {"hwarden", "tropic01", "sign",     "--slot",
                                              "26",      "--file",   "file.pdf", "garbage"};

                        auto parsed_command = parser.parseCommand(std::size(argv), argv);

                        REQUIRE(parsed_command.operation == Operation::SIGN);
                        REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                        REQUIRE(parsed_command.options["slot"] == "26");
                        REQUIRE(parsed_command.options["file"] == "file.pdf");
                    }

                    SECTION("slot specified last")
                    {
                        const char* argv[] = {"hwarden",  "tropic01", "sign", "--file",
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
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"].empty());
                }

                SECTION("data specified")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign",
                                          "--slot",  "--data",   "data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"].empty());
                    REQUIRE(parsed_command.options["data"].empty());
                }

                SECTION("file specified")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign",
                                          "--slot",  "--file",   "file.pdf"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"].empty());
                    REQUIRE(parsed_command.options["file"].empty());
                }

                SECTION("data specified first")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign",
                                          "--data",  "data",     "--slot"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["data"] == "data");
                    REQUIRE(parsed_command.options["slot"].empty());
                }

                SECTION("file specified first")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign",
                                          "--file",  "file.pdf", "--slot"};

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
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["data"].empty());
                    REQUIRE(parsed_command.options["slot"].empty());
                }

                SECTION("slot specified first")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--slot", "12", "--data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"] == "12");
                    REQUIRE(parsed_command.options["data"].empty());
                }

                SECTION("slot specified last")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--data", "--slot", "12"};

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
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--file"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"].empty());
                    REQUIRE(parsed_command.options["file"].empty());
                }

                SECTION("slot specified first")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--slot", "15", "--file"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::SIGN);
                    REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
                    REQUIRE(parsed_command.options["slot"] == "15");
                    REQUIRE(parsed_command.options["file"].empty());
                }

                SECTION("slot specified last")
                {
                    const char* argv[] = {"hwarden", "tropic01", "sign", "--file", "--slot", "15"};

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
                const char* argv[] = {"hwarden", "tropic01", "sign", "--slot", "32"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                REQUIRE(parsed_command.options["slot"].empty());
            }

            SECTION("not a number")
            {
                const char* argv[] = {"hwarden", "tropic01", "sign", "--slot", "NaN"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::SIGN);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                REQUIRE(parsed_command.options["slot"].empty());
            }
        }
    }
}

TEST_CASE("verify")
{
    CliParser parser;

    SECTION("success")
    {
        SECTION("slot and data specified")
        {
            const char* argv[] = {"hwarden", "tropic01",    "verify",
                                  "--slot",  "25",          "--data",
                                  "data",    "--signature", valid_signature.c_str()};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.device == DeviceType::TROPIC01);
            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.options["slot"] == "25");
            REQUIRE(parsed_command.options["data"] == "data");
            REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

            REQUIRE(parsed_command.options["file"].empty());
        }

        SECTION("slot and file specified")
        {
            const char* argv[] = {"hwarden",  "tropic01",    "verify",
                                  "--slot",   "25",          "--file",
                                  "file.txt", "--signature", valid_signature.c_str()};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.options["slot"] == "25");
            REQUIRE(parsed_command.options["file"] == "file.txt");
            REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

            REQUIRE(parsed_command.options["data"].empty());
        }

        SECTION("interchangeable argument order")
        {
            // Did not test all possible order combinations
            // since it seems irrelevant to do so.
            // 3 should be enough.
            SECTION("signature | file | slot")
            {
                const char* argv[] = {
                    "hwarden", "tropic01", "verify", "--signature", valid_signature.c_str(),
                    "--file",  "file.txt", "--slot", "25"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.options["slot"] == "25");
                REQUIRE(parsed_command.options["file"] == "file.txt");
                REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

                REQUIRE(parsed_command.options["data"].empty());
            }

            SECTION("data | slot | signature")
            {
                const char* argv[] = {"hwarden", "tropic01",    "verify",
                                      "--data",  "data",        "--slot",
                                      "25",      "--signature", valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.options["slot"] == "25");
                REQUIRE(parsed_command.options["data"] == "data");
                REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

                REQUIRE(parsed_command.options["file"].empty());
            }

            SECTION("slot | signature | file")
            {
                const char* argv[] = {"hwarden",
                                      "tropic01",
                                      "verify",
                                      "--slot",
                                      "25",
                                      "--signature",
                                      valid_signature.c_str(),
                                      "--file",
                                      "file.txt"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::NONE);
                REQUIRE(parsed_command.options["slot"] == "25");
                REQUIRE(parsed_command.options["file"] == "file.txt");
                REQUIRE(parsed_command.options["signature"] == valid_signature.c_str());

                REQUIRE(parsed_command.options["data"].empty());
            }
        }
    }

    SECTION("help")
    {
        SECTION("success")
        {
            const char* argv[] = {"hwarden", "tropic01", "verify", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::NONE);
            REQUIRE(parsed_command.requires_help);
        }

        SECTION("help without device")
        {
            const char* argv[] = {"hwarden", "verify", "--help"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
            REQUIRE(parsed_command.requires_help == false);
        }

        SECTION("unexpected argument")
        {
            const char* argv[] = {"hwarden", "tropic01", "verify", "--help", "unexpected-argument"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            REQUIRE(parsed_command.requires_help == false);
        }
    }

    SECTION("slot without device")
    {
        const char* argv[] = {"hwarden", "verify", "--slot",      "25",
                              "--data",  "data",   "--signature", valid_signature.c_str()};

        auto parsed_command = parser.parseCommand(std::size(argv), argv);

        REQUIRE(parsed_command.operation == Operation::VERIFY);
        REQUIRE(parsed_command.error == ParseError::MISSING_DEVICE);
    }

    SECTION("invalid option")
    {
        SECTION("unknown option")
        {
            const char* argv[] = {"hwarden", "tropic01", "verify", "--unknown-option"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
        }

        SECTION("duplicated option")
        {
            SECTION("slot")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--slot",
                                      "3",       "--slot",   "4"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("data")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify",   "--data",
                                      "data",    "--data",   "more-data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("file")
            {
                const char* argv[] = {"hwarden",       "tropic01", "verify",          "--file",
                                      "some_file.txt", "--file",   "another-file.txt"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }

            SECTION("signature")
            {
                const char* argv[] = {"hwarden",
                                      "tropic01",
                                      "verify",
                                      "--signature",
                                      valid_signature.c_str(),
                                      "--signature",
                                      valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
            }
        }

        SECTION("mutually exclusive options")
        {
            SECTION("data and file")
            {
                SECTION("data specified first")
                {
                    const char* argv[] = {"hwarden", "tropic01", "verify",  "--data",
                                          "data",    "--file",   "file.txt"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::VERIFY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }

                SECTION("file specified first")
                {
                    const char* argv[] = {"hwarden",  "tropic01", "verify", "--file",
                                          "file.txt", "--data",   "data"};

                    auto parsed_command = parser.parseCommand(std::size(argv), argv);

                    REQUIRE(parsed_command.operation == Operation::VERIFY);
                    REQUIRE(parsed_command.error == ParseError::INVALID_OPTION);
                }
            }
        }

        SECTION("unexpected argument after valid command")
        {
            const char* argv[] = {"hwarden",
                                  "tropic01",
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
            const char* argv[] = {"hwarden", "tropic01", "verify"};

            auto parsed_command = parser.parseCommand(std::size(argv), argv);

            REQUIRE(parsed_command.operation == Operation::VERIFY);
            REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
        }

        SECTION("missing slot")
        {
            SECTION("data specified")
            {
                const char* argv[] = {"hwarden",
                                      "tropic01",
                                      "verify",
                                      "--data",
                                      "data",
                                      "--signature",
                                      valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("file specified")
            {
                const char* argv[] = {"hwarden",
                                      "tropic01",
                                      "verify",
                                      "--file",
                                      "file.txt",
                                      "--signature",
                                      valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }
        }

        SECTION("missing data / file")
        {
            SECTION("slot specified")
            {
                const char* argv[] = {"hwarden",
                                      "tropic01",
                                      "verify",
                                      "--slot",
                                      "18",
                                      "--signature",
                                      valid_signature.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }
        }

        SECTION("missing signature")
        {
            SECTION("slot and data specified")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--slot",
                                      "15",      "--data",   "data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_OPTION);
            }

            SECTION("slot and file specified")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify",  "--slot",
                                      "15",      "--file",   "file.txt"};

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
                const char* argv[] = {"hwarden", "tropic01", "verify", "--slot"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--slot", "--data", "data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }
        }

        SECTION("data")
        {
            SECTION("other options not specified")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--data"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--data", "--slot", "5"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }
        }

        SECTION("file")
        {
            SECTION("other options not specified")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--file"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--file", "--slot", "5"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }
        }

        SECTION("signature")
        {
            SECTION("other options not specified")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--signature"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::MISSING_VALUE);
            }

            SECTION("other options specified")
            {
                const char* argv[] = {"hwarden",     "tropic01", "verify",
                                      "--signature", "--data",   "some-data"};

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
                const char* argv[] = {"hwarden", "tropic01", "verify", "--slot", "NaN"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                REQUIRE(parsed_command.options["slot"].empty());
            }

            SECTION("out of boundaries")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--slot", "32"};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
                REQUIRE(parsed_command.options["slot"].empty());
            }
        }

        SECTION("signature")
        {
            SECTION("invalid signature length")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--signature",
                                      invalid_signature_65_bytes.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }

            SECTION("invalid signature content")
            {
                const char* argv[] = {"hwarden", "tropic01", "verify", "--signature",
                                      invalid_signature_64_bytes.c_str()};

                auto parsed_command = parser.parseCommand(std::size(argv), argv);

                REQUIRE(parsed_command.operation == Operation::VERIFY);
                REQUIRE(parsed_command.error == ParseError::INVALID_VALUE);
            }
        }
    }
}
