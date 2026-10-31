#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "AuditTypes.hpp"
#include "CliTypes.hpp"
#include "IDevice.hpp"

class CliDisplay
{
public:
    void helpMenu();
    void deviceHelpMenu(DeviceType device);
    void commandError(Command command);

    void statusResult(SystemStatus result);
    void logsResult(SystemStatus result, std::vector<AuditEntry> entries);
    void eraseKeyResult(SystemStatus result, uint8_t slot);
    void generateKeyResult(SystemStatus result, Command command);
    void readKeyResult(SystemStatus result, uint8_t slot, std::vector<uint8_t> pubKey);
    void listKeysResult(SystemStatus result, Command command,
                        std::vector<std::vector<uint8_t>> pubKeys);
    void signResult(SystemStatus result, std::vector<uint8_t> signature);
    void verifyResult(SystemStatus result);

    // operation specific help menu functions
    void operationHelpMenu(Operation op);

    void status_help();
    void logs_help();
    void eraseKey_help();
    void generateKey_help();
    void readKey_help();
    void listKeys_help();
    void sign_help();
    void verify_help();
};