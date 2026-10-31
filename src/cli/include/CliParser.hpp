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
    void parseEraseKey(Command& command, const std::vector<std::string>& options);
    bool parseSlotValue(Command& command, const std::string& value);
};