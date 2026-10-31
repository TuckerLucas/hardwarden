#pragma once

#include <string>
#include <vector>

#include "CliTypes.hpp"

class CliParser
{
public:
    Command parseCommand(int argc, const char* argv[]);

private:
    // TODO: Consider refactoring this function somewhere else.
    static bool isValidHex(const std::string& str)
    {
        return str.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
    }

    void parseStatus(Command& command, const std::vector<std::string>& options);
    void parseLogs(Command& command, const std::vector<std::string>& options);
    void parseSlotOperation(Command& command, const std::vector<std::string>& options);
    bool parseSlotValue(Command& command, const std::string& value);
    void parseListKeys(Command& command, const std::vector<std::string>& options);
    void parseGenerateKey(Command& command, const std::vector<std::string>& options);
    static bool hasValue(const std::vector<std::string>& options, size_t i);
};