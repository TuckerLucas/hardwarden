#include "CliParser.hpp"

#include <iostream>
#include <unordered_map>

namespace
{
const std::unordered_map<std::string, DeviceType> DEVICES = {
    {"tropic01", DeviceType::TROPIC01},
};
}

Command CliParser::parseCommand(int argc, const char* argv[])
{
    Command command;

    std::vector<std::string> args;

    if (argc > 1)
    {
        // Skip the program name.
        args.assign(argv + 1, argv + argc);
    }

    size_t next = 0;

    if (next < args.size())
    {
        auto device = DEVICES.find(args[next]);

        if (device != DEVICES.end())
        {
            command.device = device->second;
            next++;
        }
    }

    if (next >= args.size())
    {
        command.error = ParseError::MISSING_OPERATION;
        return command;
    }

    const std::string& operation = args[next];
    std::vector<std::string> options(args.begin() + next + 1, args.end());

    if (operation == "--help")
    {
        command.operation = Operation::HELP;

        if (!options.empty())
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
        parseSign(command, options);
        return command;
    }

    if (operation == "verify")
    {
        parseVerify(command, options);
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

void CliParser::parseSign(Command& command, const std::vector<std::string>& options)
{
    command.operation = Operation::SIGN;

    if (command.device == DeviceType::NONE)
    {
        command.error = ParseError::MISSING_DEVICE;
        return;
    }

    bool slot_option_used = false;
    bool data_option_used = false;
    bool file_option_used = false;

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
        else if (arg == "--data")
        {
            if (data_option_used || file_option_used)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            data_option_used = true;

            // TODO: note down that signing data starting with "--" cannot be done
            if (!hasValue(options, i))
            {
                command.error = ParseError::MISSING_VALUE;
                return;
            }

            i++;

            command.options["data"] = options[i];
        }
        else if (arg == "--file")
        {
            if (file_option_used || data_option_used)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            file_option_used = true;

            // TODO: note down that signing a file whose name starts with "--" cannot be done
            if (!hasValue(options, i))
            {
                command.error = ParseError::MISSING_VALUE;
                return;
            }

            i++;

            command.options["file"] = options[i];
        }
        else
        {
            command.error = ParseError::INVALID_OPTION;
            return;
        }
    }

    if (!slot_option_used || (!data_option_used && !file_option_used))
    {
        command.error = ParseError::MISSING_OPTION;
    }
}

void CliParser::parseVerify(Command& command, const std::vector<std::string>& options)
{
    command.operation = Operation::VERIFY;

    if (command.device == DeviceType::NONE)
    {
        command.error = ParseError::MISSING_DEVICE;
        return;
    }

    bool slot_option_used = false;
    bool data_option_used = false;
    bool file_option_used = false;
    bool signature_option_used = false;

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
        else if (arg == "--data")
        {
            if (data_option_used || file_option_used)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            data_option_used = true;

            if (!hasValue(options, i))
            {
                command.error = ParseError::MISSING_VALUE;
                return;
            }

            i++;

            command.options["data"] = options[i];
        }
        else if (arg == "--file")
        {
            if (file_option_used || data_option_used)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            file_option_used = true;

            if (!hasValue(options, i))
            {
                command.error = ParseError::MISSING_VALUE;
                return;
            }

            i++;

            command.options["file"] = options[i];
        }
        else if (arg == "--signature")
        {
            if (signature_option_used)
            {
                command.error = ParseError::INVALID_OPTION;
                return;
            }

            signature_option_used = true;

            if (!hasValue(options, i))
            {
                command.error = ParseError::MISSING_VALUE;
                return;
            }

            i++;

            if (options[i].size() != 128 || !isValidHex(options[i]))
            {
                command.error = ParseError::INVALID_VALUE;
                return;
            }

            command.options["signature"] = options[i];
        }
        else
        {
            command.error = ParseError::INVALID_OPTION;
            return;
        }
    }

    if (!slot_option_used || (!data_option_used && !file_option_used) || !signature_option_used)
    {
        command.error = ParseError::MISSING_OPTION;
    }
}