#include "CliDisplay.hpp"

#include <cassert>
#include <iomanip>

namespace
{
std::string slotRangeText(DeviceType device)
{
    SlotRange range = slotRange(device);

    return "<" + std::to_string(range.min) + "-" + std::to_string(range.max) + ">";
}
// The secrets a device asks for, and for which operations, or "none".
std::string secretsText(DeviceType device)
{
    const Operation operations[] = {
        Operation::STATUS,    Operation::ERASE_KEY, Operation::GENERATE_KEY, Operation::READ_KEY,
        Operation::LIST_KEYS, Operation::SIGN,      Operation::VERIFY};
    std::string pin_ops;
    std::string key_ops;

    for (Operation operation : operations)
    {
        std::string& list = (requiredSecret(device, operation) == Secret::PIN) ? pin_ops : key_ops;

        if (requiredSecret(device, operation) == Secret::NONE)
        {
            continue;
        }

        list += (list.empty() ? "" : ", ") + operationToString(operation);
    }

    std::string text;

    if (!pin_ops.empty())
    {
        text += "PIN (" + pin_ops + ")";
    }

    if (!key_ops.empty())
    {
        text += (text.empty() ? "" : ", ") + std::string("management key (") + key_ops + ")";
    }

    return text.empty() ? "none" : text;
}

// One description line saying which secret the operation asks for, on devices that need one.
void printSecretNote(DeviceType device, Operation operation)
{
    Secret secret = requiredSecret(device, operation);

    if (secret == Secret::NONE)
    {
        return;
    }

    std::cout << "        Asks for the device's "
              << (secret == Secret::PIN ? "PIN" : "management key") << ".";

    if (defaultSecret(device, secret))
    {
        std::cout << " Leave it blank to use the factory default.";
    }

    std::cout << "\n";
}
}  // namespace

void CliDisplay::helpMenu()
{
    std::cout << "hwarden <device> <operation> [options]\n";
    std::cout << "hwarden <operation> [options]\n";
    std::cout << "\n";
    std::cout << "Key management and audit logging for hardware security devices.\n";
    std::cout << "\n";
    std::cout << "Usage:\n";
    std::cout << "\n";
    std::cout << "        Global flags:\n";
    std::cout << "                     --help         Show this help menu.\n";
    std::cout << "\n";
    std::cout << "        Devices:\n";
    std::cout << "                     tropic01-spi   TROPIC01 secure element over SPI (Raspberry "
                 "Pi shield).\n";
    std::cout
        << "                     tropic01-usb   TROPIC01 secure element over USB (USB devkit).\n";
    std::cout << "                     yubikey        YubiKey 5 Series security key.\n";
    std::cout << "\n";
    std::cout << "        Device operations:\n";
    std::cout << "                     status         Check if the device is connected.\n";
    std::cout << "                     erase-key      Erase a key.\n";
    std::cout << "                     generate-key   Generate an ECC key pair.\n";
    std::cout << "                     read-key       Read back a public key.\n";
    std::cout << "                     list-keys      List all public keys stored on the device.\n";
    std::cout
        << "                     sign           Sign data/file using a hardware backed key.\n";
    std::cout << "                     verify         Verify a signature using a hardware backed "
                 "key.\n";
    std::cout << "\n";
    std::cout << "        Deviceless operations:\n";
    std::cout << "                     logs           Display audit log.\n";
    std::cout << "\n";
    std::cout << "Hints:\n";
    std::cout << "\n";
    std::cout << "        Run 'hwarden <device> --help' for device specific information.\n";
    std::cout << "        Run 'hwarden <device> <operation> --help' for operation specific "
                 "options.\n";
    std::cout
        << "        Run 'hwarden <operation> --help' for deviceless operation specific options.\n";
}

void CliDisplay::deviceHelpMenu(DeviceType device)
{
    switch (device)
    {
        case DeviceType::TROPIC01_SPI:
        case DeviceType::TROPIC01_USB:
            std::cout << "hwarden " << deviceTypeToString(device) << " <operation> [options]\n";
            std::cout << "\n";
            std::cout << (device == DeviceType::TROPIC01_SPI
                              ? "TROPIC01 secure element over SPI (Raspberry Pi shield).\n"
                              : "TROPIC01 secure element over USB (USB devkit).\n");
            std::cout << "\n";
            std::cout << "        Key slots:   " << slotRange(device).min << "-"
                      << slotRange(device).max << "\n";
            std::cout << "        Curves:      ed25519, p256\n";
            std::cout << "        Secrets:     " << secretsText(device) << "\n";
            std::cout << "        Operations:  status, erase-key, generate-key, read-key, "
                         "list-keys, sign, verify\n";
            std::cout << "\n";
            std::cout << "Hint:\n";
            std::cout << "\n";
            std::cout << "        Run 'hwarden " << deviceTypeToString(device)
                      << " <operation> --help' for operation specific options.\n";
            break;
        case DeviceType::YUBIKEY:
            std::cout << "hwarden yubikey <operation> [options]\n";
            std::cout << "\n";
            std::cout << "YubiKey 5 Series security key.\n";
            std::cout << "\n";
            std::cout << "        Key slots:   " << slotRange(device).min << "-"
                      << slotRange(device).max << "\n";
            std::cout << "        Curves:      ed25519, p256\n";
            std::cout << "        Secrets:     " << secretsText(device) << "\n";
            std::cout << "        Operations:  status, erase-key, generate-key, read-key, "
                         "list-keys, sign, verify\n";
            std::cout << "\n";
            std::cout << "Hint:\n";
            std::cout << "\n";
            std::cout << "        Run 'hwarden yubikey <operation> --help' for operation "
                         "specific options.\n";
            break;
        default:
            assert(false && "Unhandled device value in deviceHelpMenu");
    }
}

void CliDisplay::commandError(Command command)
{
    switch (command.error)
    {
        case ParseError::MISSING_OPERATION:
        {
            std::cout << "No operation specified. Run 'hwarden --help' for available operations.\n";
            break;
        }
        case ParseError::INVALID_OPERATION:
        {
            if (command.device == DeviceType::NONE)
            {
                std::cout << "Unknown device or operation. Run 'hwarden --help' for available "
                             "devices and operations.\n";
            }
            else
            {
                std::cout << "Unknown operation. Run 'hwarden "
                          << deviceTypeToString(command.device)
                          << " --help' for available operations.\n";
            }
            break;
        }
        case ParseError::MISSING_DEVICE:
        {
            std::cout << operationToString(command.operation)
                      << ": missing device. Run 'hwarden --help' for available devices.\n";
            break;
        }
        case ParseError::UNEXPECTED_DEVICE:
        {
            std::cout << operationToString(command.operation)
                      << ": does not take a device. Run 'hwarden "
                      << operationToString(command.operation) << " --help' for usage.\n";
            break;
        }
        case ParseError::MISSING_OPTION:
        {
            std::cout << operationToString(command.operation) << ": missing an option.\n";
            break;
        }
        case ParseError::INVALID_OPTION:
        {
            std::cout << operationToString(command.operation) << ": invalid option.\n";
            break;
        }
        case ParseError::MISSING_VALUE:
        {
            std::cout << operationToString(command.operation) << ": missing option value.\n";
            break;
        }
        case ParseError::INVALID_VALUE:
        {
            std::cout << operationToString(command.operation) << ": invalid value.\n";
            break;
        }
    }
}

void CliDisplay::secretCancelled(Secret secret)
{
    std::cout << "Cancelled: no " << (secret == Secret::PIN ? "PIN" : "management key")
              << " entered.\n";
}

// Prints the message for a failed login and returns true, or returns false for any other result.
bool CliDisplay::loginError(SystemStatus result)
{
    switch (result)
    {
        case SystemStatus::DEVICE_ERROR_PIN_INCORRECT:
            std::cout << "incorrect PIN. The PIN is blocked after too many wrong attempts.\n";
            return true;
        case SystemStatus::DEVICE_ERROR_PIN_LOCKED:
            std::cout << "the PIN is blocked after too many wrong attempts. Unblock it with the "
                         "device's PUK before trying again.\n";
            return true;
        case SystemStatus::DEVICE_ERROR_MANAGEMENT_KEY_INCORRECT:
            std::cout << "incorrect management key.\n";
            return true;
        case SystemStatus::DEVICE_ERROR_LOGIN:
            std::cout << "could not log in to the device.\n";
            return true;
        default:
            return false;
    }
}

void CliDisplay::statusResult(SystemStatus result)
{
    if (result == SystemStatus::OK)
    {
        std::cout << "Hardware detected!\n";
    }
    else
    {
        std::cout << "Could not detect hardware: ";

        if (result == SystemStatus::DEVICE_ERROR_INIT)
            std::cout
                << "initialisation error. Check that the hardware security device is connected.\n";
        else if (result == SystemStatus::DEVICE_ERROR_STATUS)
            std::cout << "ping failed. Check that the hardware security device is connected.\n";
        else if (result == SystemStatus::DEVICE_ERROR_DEINIT)
            std::cout << "deinitialisation error.\n";
    }
}

void CliDisplay::logsResult(SystemStatus result, std::vector<AuditEntry> entries)
{
    if (result == SystemStatus::OK)
    {
        std::cout << "\n";
        std::cout << "Audit Log\n";
        std::cout << std::string(89, '-') << "\n";
        std::cout << std::left << std::setw(22) << "Timestamp" << std::setw(14) << "Device"
                  << std::setw(15) << "Operation" << std::setw(10) << "Result" << "Options\n";
        std::cout << std::string(89, '-') << "\n";

        for (const auto& entry : entries)
        {
            std::cout << std::left << std::setw(22) << entry.timestamp << std::setw(14)
                      << entry.device << std::setw(15) << entry.operation << std::setw(10)
                      << entry.auditResult << entry.options << "\n";
        }

        std::cout << std::string(89, '-') << "\n";
        std::cout << entries.size() << " operations logged\n\n";
    }
    else
    {
        std::cout << "Failed to retrieve logs\n";
    }
}

void CliDisplay::eraseKeyResult(SystemStatus result, uint8_t slot)
{
    if (result == SystemStatus::OK)
    {
        std::cout << "Key erased from slot " << static_cast<int>(slot) << "!\n";
    }
    else
    {
        std::cout << "Operation failed: ";

        if (loginError(result))
        {
            return;
        }

        if (result == SystemStatus::DEVICE_ERROR_INIT)
            std::cout
                << "initialisation error. Check that the hardware security device is connected.\n";
        else if (result == SystemStatus::DEVICE_ERROR_ERASE_KEY)
            std::cout << "could not erase key.\n";
        else if (result == SystemStatus::DEVICE_ERROR_DEINIT)
            std::cout << "deinitialisation error.\n";
    }
}

void CliDisplay::generateKeyResult(SystemStatus result, Command command)
{
    if (result == SystemStatus::OK)
    {
        std::cout << "Key generated in slot " << command.options["slot"] << " with "
                  << command.options["curve"] << " curve!\n";
    }
    else
    {
        std::cout << "Operation failed: ";

        if (loginError(result))
        {
            return;
        }

        if (result == SystemStatus::DEVICE_ERROR_INIT)
        {
            std::cout
                << "initialisation error. Check that the hardware security device is connected.\n";
        }
        else if (result == SystemStatus::DEVICE_ERROR_GENERATE_KEY_SLOT_OCCUPIED)
        {
            std::cout << "slot " << command.options["slot"] << " already contains a key.\n";
            std::cout << "Run 'hwarden " << deviceTypeToString(command.device)
                      << " erase-key --slot " << command.options["slot"]
                      << "' to erase it first.\n";
        }
        else if (result == SystemStatus::DEVICE_ERROR_GENERATE_KEY_HW_ERROR)
        {
            std::cout << "hardware error generating key in slot " << command.options["slot"]
                      << ".\n";
        }
        else if (result == SystemStatus::DEVICE_ERROR_DEINIT)
        {
            std::cout << "deinitialisation error.\n";
        }
    }
}

void CliDisplay::readKeyResult(SystemStatus result, DeviceType device, uint8_t slot,
                               std::vector<uint8_t> pubKey)
{
    if (result == SystemStatus::OK)
    {
        std::string curve_name = (pubKey.size() == 32) ? "Ed25519" : "P-256";

        std::cout << "\n";
        std::cout << "Public key from slot " << static_cast<int>(slot) << " (" << curve_name
                  << "):\n";
        std::cout << std::string(60, '-') << "\n";

        for (size_t i = 0; i < pubKey.size(); i++)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(pubKey[i]);

            if ((i + 1) % 16 == 0)
            {
                std::cout << "\n";
            }
            else
            {
                std::cout << " ";
            }
        }

        std::cout << std::string(60, '-') << "\n";
        std::cout << "Key length: " << std::dec << pubKey.size() << " bytes\n\n";
    }
    else
    {
        std::cout << "Operation failed: ";

        if (result == SystemStatus::DEVICE_ERROR_INIT)
        {
            std::cout
                << "initialisation error. Check that the hardware security device is connected.\n";
        }
        else if (result == SystemStatus::DEVICE_ERROR_READ_KEY_EMPTY_SLOT)
        {
            std::cout << "slot " << static_cast<int>(slot) << " is empty.";
            std::cout << "\n";
            std::cout << "Run 'hwarden " << deviceTypeToString(device) << " generate-key --slot "
                      << static_cast<int>(slot) << "' to generate a key first.\n";
        }
        else if (result == SystemStatus::DEVICE_ERROR_READ_KEY_HW_ERROR)
        {
            std::cout << "hardware error reading key from slot " << static_cast<int>(slot) << ".\n";
        }
        else if (result == SystemStatus::DEVICE_ERROR_DEINIT)
        {
            std::cout << "deinitialisation error.\n";
        }
    }
}

void CliDisplay::listKeysResult(SystemStatus result, Command command,
                                std::vector<std::vector<uint8_t>> pubKeys)
{
    // TODO: Any point having different SystemStatus error codes
    // for list keys if this is how we deal with them?
    if (result != SystemStatus::OK)
    {
        std::cout << "Error reading keys\n";
        return;
    }

    int occupied = 0;
    for (size_t i = 0; i < pubKeys.size(); i++)
    {
        if (!pubKeys[i].empty())
        {
            occupied++;
        }
    }

    if (occupied == 0)
    {
        std::cout << "\nNo keys found.\n\n";
        return;
    }

    // TODO: Decide if 60 char separator is enough when verbose is used
    std::cout << "\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::left << std::setw(8) << "Slot" << std::setw(10) << "Curve" << "Public Key\n";
    std::cout << std::string(60, '-') << "\n";

    for (size_t i = 0; i < pubKeys.size(); i++)
    {
        if (pubKeys[i].empty())
        {
            continue;
        }

        std::string curve = (pubKeys[i].size() == 32) ? "Ed25519" : "P-256";

        std::cout << std::left << std::setw(8) << i << std::setw(10) << curve << std::right;

        if (command.options["verbose"] == "true")
        {
            for (size_t j = 0; j < pubKeys[i].size(); j++)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<int>(pubKeys[i][j]);
            }
        }
        else
        {
            std::ostringstream truncated{};

            for (size_t j = 0; j < 8 && j < pubKeys[i].size(); j++)
                truncated << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<int>(pubKeys[i][j]);

            truncated << "...";

            for (size_t j = pubKeys[i].size() - 4; j < pubKeys[i].size(); j++)
                truncated << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<int>(pubKeys[i][j]);

            std::cout << truncated.str();
        }

        std::cout << std::dec << std::setfill(' ') << "\n";
    }

    std::cout << std::string(60, '-') << "\n";
    std::cout << occupied << " key(s) found\n\n";
}

void CliDisplay::signResult(SystemStatus result, DeviceType device, std::vector<uint8_t> signature)
{
    if (result == SystemStatus::OK)
    {
        std::cout << "\nSignature:\n";
        std::cout << std::string(60, '-') << "\n";

        for (size_t i = 0; i < signature.size(); i++)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(signature[i]);
        }

        std::cout << "\n";
        std::cout << std::dec << std::setfill(' ');
        std::cout << std::string(60, '-') << "\n";
        std::cout << "Signature length: " << signature.size() << " bytes\n\n";
    }
    else
    {
        std::cout << "Operation failed: ";

        if (loginError(result))
        {
            return;
        }

        if (result == SystemStatus::ERROR_FILE_NOT_FOUND)
            std::cout << "file not found. Check the path and try again.\n";
        else if (result == SystemStatus::DEVICE_ERROR_INIT)
            std::cout
                << "initialisation error. Check that the hardware security device is connected.\n";
        else if (result == SystemStatus::DEVICE_ERROR_READ_KEY_EMPTY_SLOT)
            std::cout << "slot is empty. Generate a key first using 'hwarden "
                      << deviceTypeToString(device) << " generate-key --slot <slot>'.\n";
        else if (result == SystemStatus::DEVICE_ERROR_READ_KEY_HW_ERROR)
            std::cout << "hardware error reading key from slot. Check that the hardware security "
                         "device is "
                         "connected.\n";
        else if (result == SystemStatus::DEVICE_ERROR_SIGN)
            std::cout << "signing failed. Check that a key exists in the specified slot.\n";
        else if (result == SystemStatus::DEVICE_ERROR_SIGN_PAYLOAD_TOO_LARGE)
            std::cout << "payload too large for an Ed25519 key on this device (maximum "
                      << ed25519MaxMessageSize(device)
                      << " bytes). Use a P-256 key for larger payloads.\n";
        else if (result == SystemStatus::DEVICE_ERROR_DEINIT)
            std::cout << "deinitialisation error.\n";
    }
}

void CliDisplay::verifyResult(SystemStatus result, DeviceType device)
{
    if (result == SystemStatus::OK)
    {
        std::cout << "\nThe signature is valid.\n";
    }
    else
    {
        std::cout << "Signature verification failed: ";

        if (result == SystemStatus::ERROR_FILE_NOT_FOUND)
            std::cout << "file not found. Check the path and try again.\n";
        else if (result == SystemStatus::DEVICE_ERROR_INIT)
            std::cout
                << "initialisation error. Check that the hardware security device is connected.\n";
        else if (result == SystemStatus::DEVICE_ERROR_READ_KEY_EMPTY_SLOT)
            std::cout << "slot is empty. Generate a key first using 'hwarden "
                      << deviceTypeToString(device) << " generate-key --slot <slot>'.\n";
        else if (result == SystemStatus::DEVICE_ERROR_READ_KEY_HW_ERROR)
            std::cout << "hardware error reading key from slot. Check that the hardware security "
                         "device is "
                         "connected.\n";
        else if (result == SystemStatus::DEVICE_ERROR_VERIFY)
            std::cout << "invalid public key size. Public keys must be 32 or 64 bytes.\n";
        else if (result == SystemStatus::DEVICE_ERROR_VERIFY_CRYPTO_ERROR)
            std::cout
                << "cryptographic error during verification. Check your inputs and try again.\n";
        else if (result == SystemStatus::DEVICE_ERROR_VERIFY_INVALID_SIGNATURE)
            std::cout << "the signature is invalid.\n";
        else if (result == SystemStatus::DEVICE_ERROR_DEINIT)
            std::cout << "deinitialisation error.\n";
    }
}

// TODO: Improve help menus for individual operations

void CliDisplay::status_help(DeviceType device)
{
    std::cout << "Usage:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " status\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "        Check if the device is connected and responding.\n";
}

void CliDisplay::logs_help()
{
    std::cout << "Usage:\n";
    std::cout << "        hwarden logs\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "        Display the audit log of all operations performed on devices.\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "        hwarden logs\n";
}

void CliDisplay::eraseKey_help(DeviceType device)
{
    const SlotRange range = slotRange(device);
    const unsigned int low = range.min;
    const unsigned int high = range.max;

    std::cout << "Usage:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " erase-key --slot <slot>\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "        Erase the key stored in the specified slot on the device.\n";
    printSecretNote(device, Operation::ERASE_KEY);
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "        --slot " << slotRangeText(device) << "   Slot to erase the key from.\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " erase-key --slot " << low
              << "\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " erase-key --slot " << high
              << "\n";
}

void CliDisplay::generateKey_help(DeviceType device)
{
    const SlotRange range = slotRange(device);
    const unsigned int mid = (range.min + range.max) / 2;

    std::cout << "Usage:\n";
    std::cout << "        hwarden " << deviceTypeToString(device)
              << " generate-key --slot <slot> [--curve <curve>]\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "        Generate an ECC key pair in the specified slot on the device.\n";
    printSecretNote(device, Operation::GENERATE_KEY);
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "        --slot  " << slotRangeText(device)
              << "    Slot to generate the key in.\n";
    std::cout << "        --curve <curve>   Curve to use for key generation.\n";
    std::cout << "                          ed25519   Edwards curve (default)\n";
    std::cout << "                          p256      NIST P-256 curve\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " generate-key --slot " << mid
              << "\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " generate-key --slot " << mid
              << " --curve ed25519\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " generate-key --slot " << mid
              << " --curve p256\n";
    std::cout << "        hwarden " << deviceTypeToString(device)
              << " generate-key --curve p256 --slot " << mid << "\n";
}

void CliDisplay::readKey_help(DeviceType device)
{
    const SlotRange range = slotRange(device);
    const unsigned int low = range.min;
    const unsigned int high = range.max;

    std::cout << "Usage:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " read-key --slot <slot>\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "        Read the public key stored in the specified slot on the device.\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "        --slot " << slotRangeText(device)
              << "   Slot to read the public key from.\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " read-key --slot " << low
              << "\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " read-key --slot " << high
              << "\n";
}

void CliDisplay::listKeys_help(DeviceType device)
{
    std::cout << "Usage:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " list-keys [--verbose]\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "        List all public keys stored on the device.\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "        --verbose   Show complete key values.\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " list-keys\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " list-keys --verbose\n";
}

void CliDisplay::sign_help(DeviceType device)
{
    const SlotRange range = slotRange(device);
    const unsigned int low = range.min;
    const unsigned int mid = (range.min + range.max) / 2;
    const unsigned int high = range.max;

    std::cout << "Usage:\n";
    std::cout << "        hwarden " << deviceTypeToString(device)
              << " sign --slot <slot> --data <data>\n";
    std::cout << "        hwarden " << deviceTypeToString(device)
              << " sign --slot <slot> --file <path>\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "        Sign data or a file using a hardware-backed key stored in the specified "
                 "slot.\n";
    std::cout << "        The private key never leaves the device.\n";
    printSecretNote(device, Operation::SIGN);
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "        --slot " << slotRangeText(device) << "   Slot to sign with.\n";
    std::cout << "        --data <data>   Data string to sign (mutually exclusive with --file).\n";
    std::cout << "        --file <path>   Path to file to sign (mutually exclusive with --data).\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " sign --slot " << low
              << " --data \"hello\"\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " sign --slot " << mid
              << " --file document.pdf\n";
    std::cout << "        hwarden " << deviceTypeToString(device)
              << " sign --data \"hello\" --slot " << high << "\n";
    std::cout << "        hwarden " << deviceTypeToString(device)
              << " sign --file document.pdf --slot " << mid << "\n";
}

void CliDisplay::verify_help(DeviceType device)
{
    const SlotRange range = slotRange(device);
    const unsigned int low = range.min;
    const unsigned int mid = (range.min + range.max) / 2;

    std::cout << "Usage:\n";
    std::cout << "        hwarden " << deviceTypeToString(device)
              << " verify --slot <slot> --data <data> --signature <hex>\n";
    std::cout << "        hwarden " << deviceTypeToString(device)
              << " verify --slot <slot> --file <path> --signature <hex>\n";
    std::cout << "\n";
    std::cout << "Description:\n";
    std::cout << "        Verify a signature over data or a file using the public key stored in\n";
    std::cout << "        the specified slot.\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "        --slot " << slotRangeText(device) << "      Slot to verify with.\n";
    std::cout
        << "        --data <data>      Data that was signed (mutually exclusive with --file).\n";
    std::cout << "        --file <path>      Path to file that was signed (mutually exclusive with "
                 "--data).\n";
    std::cout << "        --signature <hex>  Signature in hex format (required).\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " verify --slot " << low
              << " --data \"hello\" --signature "
                 "ab12cd34...\n";
    std::cout << "        hwarden " << deviceTypeToString(device) << " verify --slot " << mid
              << " --file document.pdf --signature "
                 "ab12cd34...\n";
}

void CliDisplay::operationHelpMenu(Operation op, DeviceType device)
{
    switch (op)
    {
        case Operation::STATUS:
            status_help(device);
            break;
        case Operation::LOGS:
            logs_help();
            break;
        case Operation::ERASE_KEY:
            eraseKey_help(device);
            break;
        case Operation::GENERATE_KEY:
            generateKey_help(device);
            break;
        case Operation::READ_KEY:
            readKey_help(device);
            break;
        case Operation::LIST_KEYS:
            listKeys_help(device);
            break;
        case Operation::SIGN:
            sign_help(device);
            break;
        case Operation::VERIFY:
            verify_help(device);
            break;
        default:
            assert(false && "Unhandled operation value in operationHelpMenu");
    }
}