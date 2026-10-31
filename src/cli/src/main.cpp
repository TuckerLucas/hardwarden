#include <sys/stat.h>

#include <cstdlib>
#include <fstream>

#include "AuditLogger.hpp"
#include "CliDisplay.hpp"
#include "CliParser.hpp"
#include "DeviceManager.hpp"
#include "Pkcs11Device.hpp"

std::string getDbPath();

std::vector<uint8_t> hexToBytes(const std::string& hex)
{
    std::vector<uint8_t> bytes;

    for (size_t i = 0; i < hex.length(); i += 2)
    {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }

    return bytes;
}

int main(int argc, const char* argv[])
{
    CliDisplay display;
    CliParser parser;
    Command command = parser.parseCommand(argc, argv);

    if (command.error != ParseError::NONE)
    {
        display.commandError(command);
        return 1;
    }

    if (command.requires_help)
    {
        display.operationHelpMenu(command.operation, command.device);
        return 0;
    }

    Pkcs11Device se(TROPIC_PKCS11_MODULE_PATH);
    AuditLogger logger(getDbPath());
    DeviceManager device(se, logger);

    SystemStatus result;

    switch (command.operation)
    {
        case Operation::HELP:
        {
            if (command.device == DeviceType::NONE)
            {
                display.helpMenu();
            }
            else
            {
                display.deviceHelpMenu(command.device);
            }

            return 0;
        }
        case Operation::STATUS:
        {
            result = device.status();

            display.statusResult(result);

            break;
        }
        case Operation::LOGS:
        {
            std::vector<AuditEntry> entries;

            result = logger.fetch(entries);

            display.logsResult(result, entries);

            break;
        }
        case Operation::ERASE_KEY:
        {
            uint8_t slot = std::stoi(command.options["slot"]);

            result = device.eraseKey(slot);

            display.eraseKeyResult(result, slot);

            break;
        }
        case Operation::GENERATE_KEY:
        {
            uint8_t slot = std::stoi(command.options["slot"]);
            Curve curve = (command.options["curve"] == "p256") ? Curve::P256 : Curve::Ed25519;

            result = device.generateKey(slot, curve);

            display.generateKeyResult(result, command);

            break;
        }
        case Operation::READ_KEY:
        {
            uint8_t slot = std::stoi(command.options["slot"]);
            std::vector<uint8_t> pubKey;

            result = device.readKey(slot, pubKey);

            display.readKeyResult(result, slot, pubKey);

            break;
        }
        case Operation::LIST_KEYS:
        {
            std::vector<std::vector<uint8_t>> pubKeys;

            result = device.listKeys(pubKeys);

            display.listKeysResult(result, command, pubKeys);

            break;
        }
        case Operation::SIGN:
        {
            uint8_t slot = std::stoi(command.options["slot"]);
            std::vector<uint8_t> payload;
            std::vector<uint8_t> signature;
            DataSource dataSource;
            std::string filepath = "";

            if (command.options.count("data") > 0)
            {
                dataSource = DataSource::DATA;
                std::string data = command.options["data"];
                payload = std::vector<uint8_t>(data.begin(), data.end());
            }
            else
            {
                dataSource = DataSource::FILE;
                filepath = command.options["file"];
                std::ifstream file(filepath, std::ios::binary);

                if (!file.is_open())
                {
                    display.signResult(SystemStatus::ERROR_FILE_NOT_FOUND, {});
                    return 1;
                }

                payload = std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                               std::istreambuf_iterator<char>());
            }

            result = device.sign(slot, payload, signature, dataSource, filepath);

            display.signResult(result, signature);

            break;
        }
        case Operation::VERIFY:
        {
            std::vector<uint8_t> signature = hexToBytes(command.options["signature"]);

            DataSource dataSource;
            std::vector<uint8_t> payload;
            std::string filepath = "";

            if (!command.options["data"].empty())
            {
                dataSource = DataSource::DATA;
                std::string data = command.options["data"];
                payload = std::vector<uint8_t>(data.begin(), data.end());
            }
            else
            {
                dataSource = DataSource::FILE;
                filepath = command.options["file"];
                std::ifstream file(filepath, std::ios::binary);

                if (!file.is_open())
                {
                    display.verifyResult(SystemStatus::ERROR_FILE_NOT_FOUND);
                    return 1;
                }

                payload = std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                               std::istreambuf_iterator<char>());
            }

            uint8_t slot = std::stoi(command.options["slot"]);

            result = device.verify(slot, payload, signature, dataSource, filepath);

            display.verifyResult(result);

            break;
        }
        case Operation::NONE:
        default:
            display.commandError(command);
            return 1;
    }

    return (result == SystemStatus::OK) ? 0 : 1;
}

std::string getDbPath()
{
    const char* home = getenv("HOME");
    std::string dir = std::string(home) + "/.hardwarden";
    mkdir(dir.c_str(), 0700);
    return dir + "/audit.db";
}