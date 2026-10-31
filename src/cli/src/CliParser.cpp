#include "CliParser.hpp"

#include <iostream>

Command CliParser::parseCommand(int argc, const char* argv[])
{
    Command command;

    if (argc >= 2 && std::string(argv[1]) == "tropic01")
    {
        command.device = DeviceType::TROPIC01;
        argc--;
        argv++;
    }

    if (argc < 2)
    {
        command.error = ParseError::MISSING_OPERATION;
        return command;
    }

    std::string operation = std::string(argv[1]);
    std::vector<std::string> options(argv + 2, argv + argc);

    if (operation == "--help")
    {
        command.operation = Operation::HELP;

        if (argc > 2)
        {
            command.error = ParseError::INVALID_OPTION;
        }

        return command;
    }

    if (operation == "status")
    {
        parseStatus(command, options);
        return command;
    }

    if (operation == "logs")
    {
        parseLogs(command, options);
        return command;
    }

    if (operation == "erase-key")
    {
        command.operation = Operation::ERASE_KEY;
        parseSlotOperation(command, options);
        return command;
    }

    if (operation == "generate-key")
    {
        parseGenerateKey(command, options);
        return command;
    }

    if (operation == "read-key")
    {
        command.operation = Operation::READ_KEY;
        parseSlotOperation(command, options);
        return command;
    }

    if (operation == "list-keys")
    {
        parseListKeys(command, options);
        return command;
    }

    if (operation == "sign")
    {
        command.operation = Operation::SIGN;

        if (command.device == DeviceType::NONE)
        {
            command.error = ParseError::MISSING_DEVICE;
            return command;
        }

        bool slot_option_used = false;
        bool data_option_used = false;
        bool file_option_used = false;

        if (argc < 3)
        {
            command.error = ParseError::MISSING_OPTION;
            return command;
        }

        for (int i = 2; i < argc; i++)
        {
            std::string arg = argv[i];

            if (arg == "--help")
            {
                if (argc != 3)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }

                command.error = ParseError::NONE;
                command.requires_help = true;

                return command;
            }
            else if (arg == "--slot")
            {
                if (slot_option_used)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }

                slot_option_used = true;

                if (i + 1 >= argc || std::string(argv[i + 1]).substr(0, 2) == "--")
                {
                    command.error = ParseError::MISSING_VALUE;
                    return command;
                }

                i++;

                try
                {
                    u_int8_t slot = std::stoi(argv[i]);

                    if (slot < MIN_SLOT || slot > MAX_SLOT)
                    {
                        command.error = ParseError::INVALID_VALUE;
                        return command;
                    }

                    command.options["slot"] = std::string(argv[i]);
                }
                catch (const std::invalid_argument&)
                {
                    command.error = ParseError::INVALID_VALUE;
                    return command;
                }
            }
            else if (arg == "--data")
            {
                if (data_option_used || file_option_used)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }

                data_option_used = true;

                // TODO: note down that signing data starting with "--" cannot be done
                if (i + 1 >= argc || std::string(argv[i + 1]).substr(0, 2) == "--")
                {
                    command.error = ParseError::MISSING_VALUE;
                    return command;
                }

                i++;

                command.options["data"] = std::string(argv[i]);
            }
            else if (arg == "--file")
            {
                if (file_option_used || data_option_used)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }

                file_option_used = true;

                // TODO: note down that signing a file whose name starts with "--" cannot be done
                if (i + 1 >= argc || std::string(argv[i + 1]).substr(0, 2) == "--")
                {
                    command.error = ParseError::MISSING_VALUE;
                    return command;
                }

                i++;

                command.options["file"] = std::string(argv[i]);
            }
            else
            {
                command.error = ParseError::INVALID_OPTION;
                return command;
            }
        }

        if (slot_option_used == false || (data_option_used == false && file_option_used == false))
        {
            command.error = ParseError::MISSING_OPTION;
        }

        return command;
    }

    if (operation == "verify")
    {
        command.operation = Operation::VERIFY;

        if (command.device == DeviceType::NONE)
        {
            command.error = ParseError::MISSING_DEVICE;
            return command;
        }

        bool slot_option_used = false;
        bool data_option_used = false;
        bool file_option_used = false;
        bool signature_option_used = false;

        for (int i = 2; i < argc; i++)
        {
            std::string arg = argv[i];

            if (arg == "--help")
            {
                if (argc != 3)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }
                command.requires_help = true;
                return command;
            }
            else if (arg == "--slot")
            {
                if (slot_option_used)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }

                slot_option_used = true;

                i++;

                if (i >= argc || std::string(argv[i]).substr(0, 2) == "--")
                {
                    command.error = ParseError::MISSING_VALUE;
                    return command;
                }

                try
                {
                    u_int8_t slot = std::stoi(argv[i]);

                    if (slot < MIN_SLOT || slot > MAX_SLOT)
                    {
                        command.error = ParseError::INVALID_VALUE;
                        return command;
                    }

                    command.options["slot"] = std::string(argv[i]);
                }
                catch (const std::invalid_argument&)
                {
                    command.error = ParseError::INVALID_VALUE;
                    return command;
                }
            }
            else if (arg == "--data")
            {
                if (data_option_used || file_option_used)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }

                data_option_used = true;

                i++;

                if (i >= argc || std::string(argv[i]).substr(0, 2) == "--")
                {
                    command.error = ParseError::MISSING_VALUE;
                    return command;
                }

                command.options["data"] = std::string(argv[i]);
            }
            else if (arg == "--file")
            {
                if (file_option_used || data_option_used)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }

                file_option_used = true;

                i++;

                if (i >= argc || std::string(argv[i]).substr(0, 2) == "--")
                {
                    command.error = ParseError::MISSING_VALUE;
                    return command;
                }

                command.options["file"] = std::string(argv[i]);
            }
            else if (arg == "--signature")
            {
                if (signature_option_used)
                {
                    command.error = ParseError::INVALID_OPTION;
                    return command;
                }

                signature_option_used = true;

                i++;

                if (i >= argc || std::string(argv[i]).substr(0, 2) == "--")
                {
                    command.error = ParseError::MISSING_VALUE;
                    return command;
                }

                std::string signature = std::string(argv[i]);

                if (signature.size() != 128)
                {
                    command.error = ParseError::INVALID_VALUE;
                    return command;
                }

                if (!isValidHex(signature))
                {
                    command.error = ParseError::INVALID_VALUE;
                    return command;
                }

                command.options["signature"] = std::string(argv[i]);
            }
            else
            {
                command.error = ParseError::INVALID_OPTION;
                return command;
            }
        }

        if (!slot_option_used || (!data_option_used && !file_option_used) || !signature_option_used)
        {
            command.error = ParseError::MISSING_OPTION;
        }

        return command;
    }

    command.error = ParseError::INVALID_OPERATION;

    return command;
}

void CliParser::parseStatus(Command& command, const std::vector<std::string>& options)
{
    command.operation = Operation::STATUS;

    if (command.device == DeviceType::NONE)
    {
        command.error = ParseError::MISSING_DEVICE;
        return;
    }

    if (options == std::vector<std::string>{"--help"})
    {
        command.requires_help = true;
        return;
    }

    if (!options.empty())
    {
        command.error = ParseError::INVALID_OPTION;
    }
}

void CliParser::parseLogs(Command& command, const std::vector<std::string>& options)
{
    command.operation = Operation::LOGS;

    if (command.device != DeviceType::NONE)
    {
        command.error = ParseError::UNEXPECTED_DEVICE;
        return;
    }

    if (options == std::vector<std::string>{"--help"})
    {
        command.requires_help = true;
        return;
    }

    if (!options.empty())
    {
        command.error = ParseError::INVALID_OPTION;
    }
}

void CliParser::parseSlotOperation(Command& command, const std::vector<std::string>& options)
{
    if (command.device == DeviceType::NONE)
    {
        command.error = ParseError::MISSING_DEVICE;
        return;
    }

    if (options.empty())
    {
        command.error = ParseError::MISSING_OPTION;
        return;
    }

    bool slot_option_used = false;

    for (size_t i = 0; i < options.size(); i++)
    {
        const std::string& arg = options[i];

        if (arg == "--help")
        {
            if (options.size() != 1)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            command.requires_help = true;
            return;
        }
        else if (arg == "--slot")
        {
            if (slot_option_used)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            slot_option_used = true;

            if (i + 1 >= options.size())
            {
                command.error = ParseError::MISSING_VALUE;
                return;
            }

            i++;

            if (!parseSlotValue(command, options[i]))
            {
                return;
            }
        }
        else
        {
            command.error = ParseError::INVALID_OPTION;
            return;
        }
    }
}

bool CliParser::parseSlotValue(Command& command, const std::string& value)
{
    try
    {
        u_int8_t slot = std::stoi(value);

        if (slot < MIN_SLOT || slot > MAX_SLOT)
        {
            command.error = ParseError::INVALID_VALUE;
            return false;
        }

        command.options["slot"] = value;
        return true;
    }
    catch (const std::invalid_argument&)
    {
        command.error = ParseError::INVALID_VALUE;
        return false;
    }
}

void CliParser::parseListKeys(Command& command, const std::vector<std::string>& options)
{
    command.operation = Operation::LIST_KEYS;

    if (command.device == DeviceType::NONE)
    {
        command.error = ParseError::MISSING_DEVICE;
        return;
    }

    if (options == std::vector<std::string>{"--help"})
    {
        command.requires_help = true;
        return;
    }

    if (options == std::vector<std::string>{"--verbose"})
    {
        command.options["verbose"] = "true";
        return;
    }

    if (!options.empty())
    {
        command.error = ParseError::INVALID_OPTION;
    }
}

// True if the option at index i is followed by a value (an argument not starting with "--").
bool CliParser::hasValue(const std::vector<std::string>& options, size_t i)
{
    return i + 1 < options.size() && options[i + 1].substr(0, 2) != "--";
}

void CliParser::parseGenerateKey(Command& command, const std::vector<std::string>& options)
{
    command.operation = Operation::GENERATE_KEY;

    if (command.device == DeviceType::NONE)
    {
        command.error = ParseError::MISSING_DEVICE;
        return;
    }

    bool slot_option_used = false;
    bool curve_option_used = false;
    command.options["curve"] = "ed25519";

    if (options.empty())
    {
        command.error = ParseError::MISSING_OPTION;
        return;
    }

    for (size_t i = 0; i < options.size(); i++)
    {
        const std::string& arg = options[i];

        if (arg == "--help")
        {
            if (options.size() != 1)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            command.requires_help = true;
            return;
        }
        else if (arg == "--slot")
        {
            if (slot_option_used)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            slot_option_used = true;

            if (!hasValue(options, i))
            {
                command.error = ParseError::MISSING_VALUE;
                return;
            }

            i++;

            if (!parseSlotValue(command, options[i]))
            {
                return;
            }
        }
        else if (arg == "--curve")
        {
            if (curve_option_used)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            curve_option_used = true;

            if (!hasValue(options, i))
            {
                command.error = ParseError::MISSING_VALUE;
                return;
            }

            i++;

            if (options[i] == "ed25519" || options[i] == "p256")
            {
                command.options["curve"] = options[i];
            }
            else
            {
                command.error = ParseError::INVALID_VALUE;
                return;
            }
        }
        else
        {
            command.error = ParseError::INVALID_OPTION;
            return;
        }
    }

    if (!slot_option_used)
    {
        command.error = ParseError::MISSING_OPTION;
    }
}