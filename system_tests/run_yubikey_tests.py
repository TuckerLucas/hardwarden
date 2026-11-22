"""System tests for the YubiKey. Usage: python3 run_yubikey_tests.py

Needs ykman: the device's PKCS#11 module does not really delete keys, so slots are emptied
with 'ykman piv keys delete' between tests.
"""

import getpass
import shutil
import sys

from helpers import (BINARY, ED25519_VALID_DATA, INVALID_SIGNATURE, P256_VALID_DATA,
                     confirm_erase, finish, make_get_signature, print_header, run_command,
                     run_test)

DEVICE = "yubikey"
LOWEST_SLOT = 5
HIGHEST_SLOT = 24
DEFAULT_MANAGEMENT_KEY = "010203040506070801020304050607080102030405060708"
WRONG_PIN = "000000"
WRONG_MANAGEMENT_KEY = "0" * 48

if shutil.which("ykman") is None:
    print("ykman not found. Install it with: sudo apt install yubikey-manager")
    sys.exit(1)

confirm_erase(DEVICE)

# Asked once and piped into every command that needs it. Blank uses the factory default.
PIN = getpass.getpass("PIN [blank to use default]: ")
MANAGEMENT_KEY = getpass.getpass("Management key [blank to use default]: ")
YKMAN_MANAGEMENT_KEY = MANAGEMENT_KEY or DEFAULT_MANAGEMENT_KEY


def secret_for(args):
    """The input to pipe into a command, matching the secret its operation asks for."""
    operation = args[0]
    if operation == "sign" and "--help" not in args:
        return PIN + "\n"
    if operation in ("generate-key", "erase-key") and "--help" not in args:
        return MANAGEMENT_KEY + "\n"
    return None


def hw(*args):
    """A hardwarden command on the YubiKey."""
    return [BINARY, DEVICE, *args]


def step(*args):
    """A setup or cleanup step running hardwarden with the matching secret."""
    return lambda: run_command(hw(*args), secret_for(args))


def delete(slot):
    """A setup or cleanup step that really deletes the key in a slot (PIV slots 82-95)."""
    piv_slot = f"{0x82 + slot - LOWEST_SLOT:02x}"
    return lambda: run_command(
        ["ykman", "piv", "keys", "delete", piv_slot, "-m", YKMAN_MANAGEMENT_KEY])


def test(description, args, expected_exit_code=0, setup=None, cleanup=None, input=None):
    """run_test for a hardwarden command on the YubiKey, piping the matching secret."""
    run_test(description, hw(*args), expected_exit_code, setup, cleanup,
             input if input is not None else secret_for(args))


def verify_test(description, slot, curve, sign_args, verify_args, expected_exit_code,
                signature=None):
    """Generates a key, signs, then verifies. signature replaces the real one if given."""
    run_test(description,
             lambda sig: hw("verify", "--slot", str(slot), *verify_args,
                            "--signature", signature or sig),
             expected_exit_code,
             setup=[step("generate-key", "--slot", str(slot), "--curve", curve),
                    make_get_signature(hw("sign", "--slot", str(slot), *sign_args), PIN + "\n")],
             cleanup=delete(slot))


print_header()

print("Preparing the device...")
for slot in range(LOWEST_SLOT, HIGHEST_SLOT + 1):
    delete(slot)()
print("Ready.\n")

# Help
run_test("help menu", [BINARY, "--help"])
run_test("device help", hw("--help"))

# Status
test("status - help", ["status", "--help"])
test("status", ["status"])

# Logs
run_test("logs - help", [BINARY, "logs", "--help"])
run_test("logs", [BINARY, "logs"])

# Erase key
test("erase key - help", ["erase-key", "--help"])

# The device's PKCS#11 module reports success but keeps the key, so only the exit code is checked.
test("erase key - reports success",
     ["erase-key", "--slot", "5"],
     setup=step("generate-key", "--slot", "5"),
     cleanup=delete(5))

test("erase key - wrong management key",
     ["erase-key", "--slot", "5"],
     setup=step("generate-key", "--slot", "5"),
     cleanup=delete(5),
     input=WRONG_MANAGEMENT_KEY + "\n",
     expected_exit_code=1)

# Generate key
test("generate key - help", ["generate-key", "--help"])

test("generate key - unspecified curve",
     ["generate-key", "--slot", "5"],
     cleanup=delete(5))

test("generate key - Ed25519 curve",
     ["generate-key", "--slot", "6", "--curve", "ed25519"],
     cleanup=delete(6))

test("generate key - NIST P-256 curve",
     ["generate-key", "--slot", "7", "--curve", "p256"],
     cleanup=delete(7))

test("generate key - curve specified first",
     ["generate-key", "--curve", "p256", "--slot", "7"],
     cleanup=delete(7))

test("generate key - highest slot",
     ["generate-key", "--slot", str(HIGHEST_SLOT)],
     cleanup=delete(HIGHEST_SLOT))

test("generate key - below lowest slot",
     ["generate-key", "--slot", str(LOWEST_SLOT - 1)],
     expected_exit_code=1)

test("generate key - above highest slot",
     ["generate-key", "--slot", str(HIGHEST_SLOT + 1)],
     expected_exit_code=1)

test("generate key - occupied slot",
     ["generate-key", "--slot", "10"],
     setup=step("generate-key", "--slot", "10"),
     cleanup=delete(10),
     expected_exit_code=1)

test("generate key - wrong management key",
     ["generate-key", "--slot", "10"],
     cleanup=delete(10),
     input=WRONG_MANAGEMENT_KEY + "\n",
     expected_exit_code=1)

# Read key
test("read key - help", ["read-key", "--help"])

test("read key - Ed25519",
     ["read-key", "--slot", "17"],
     setup=step("generate-key", "--slot", "17"),
     cleanup=delete(17))

test("read key - NIST P-256",
     ["read-key", "--slot", "18", ],
     setup=step("generate-key", "--slot", "18", "--curve", "p256"),
     cleanup=delete(18))

test("read key - empty slot",
     ["read-key", "--slot", "10"],
     expected_exit_code=1)

# List all keys
test("list all keys - help", ["list-keys", "--help"])

test("list all keys - no keys", ["list-keys"])

test("list all keys - one key",
     ["list-keys"],
     setup=step("generate-key", "--slot", "5"),
     cleanup=delete(5))

test("list all keys - multiple keys",
     ["list-keys"],
     setup=[step("generate-key", "--slot", "5"),
            step("generate-key", "--slot", "6", "--curve", "p256"),
            step("generate-key", "--slot", "7")],
     cleanup=[delete(5), delete(6), delete(7)])

test("list all keys - verbose",
     ["list-keys", "--verbose"],
     setup=[step("generate-key", "--slot", "6"),
            step("generate-key", "--slot", "7", "--curve", "p256")],
     cleanup=[delete(6), delete(7)])

# Sign
test("sign - help", ["sign", "--help"])

test("sign - data (Ed25519)",
     ["sign", "--slot", "18", "--data", "hello"],
     setup=step("generate-key", "--slot", "18"),
     cleanup=delete(18))

test("sign - data (P-256)",
     ["sign", "--slot", "18", "--data", "hello"],
     setup=step("generate-key", "--slot", "18", "--curve", "p256"),
     cleanup=delete(18))

test("sign - file (Ed25519)",
     ["sign", "--slot", "19", "--file", "test_file.txt"],
     setup=step("generate-key", "--slot", "19"),
     cleanup=delete(19))

test("sign - file (P-256)",
     ["sign", "--slot", "19", "--file", "test_file.txt"],
     setup=step("generate-key", "--slot", "19", "--curve", "p256"),
     cleanup=delete(19))

test("sign - large file (P-256)",
     ["sign", "--slot", "19", "--file", "large_test_file.txt"],
     setup=step("generate-key", "--slot", "19", "--curve", "p256"),
     cleanup=delete(19))

test("sign - file not found",
     ["sign", "--slot", "23", "--file", "nonexistent.txt"],
     setup=step("generate-key", "--slot", "23"),
     cleanup=delete(23),
     expected_exit_code=1)

test("sign - key not found (data)",
     ["sign", "--slot", "23", "--data", "hello"],
     expected_exit_code=1)

test("sign - key not found (file)",
     ["sign", "--slot", "23", "--file", "test_file.txt"],
     expected_exit_code=1)

# A wrong PIN uses up one attempt. The correct PIN right after resets the counter.
test("sign - wrong PIN",
     ["sign", "--slot", "20", "--data", "hello"],
     setup=step("generate-key", "--slot", "20"),
     input=WRONG_PIN + "\n",
     expected_exit_code=1)

test("sign - correct PIN after a wrong one",
     ["sign", "--slot", "20", "--data", "hello"],
     cleanup=delete(20))

# Verify
test("verify - help", ["verify", "--help"])

verify_test("verify - Ed25519 - valid data", 5, "ed25519",
            ["--data", ED25519_VALID_DATA], ["--data", ED25519_VALID_DATA], 0)

verify_test("verify - Ed25519 - valid file", 5, "ed25519",
            ["--file", "test_file.txt"], ["--file", "test_file.txt"], 0)

verify_test("verify - Ed25519 - invalid data", 6, "ed25519",
            ["--data", ED25519_VALID_DATA], ["--data", "wrong_data"], 1)

verify_test("verify - Ed25519 - invalid file", 6, "ed25519",
            ["--file", "test_file.txt"], ["--file", "large_test_file.txt"], 1)

verify_test("verify - Ed25519 - invalid signature + valid data", 6, "ed25519",
            ["--data", ED25519_VALID_DATA], ["--data", ED25519_VALID_DATA], 1,
            signature=INVALID_SIGNATURE)

verify_test("verify - Ed25519 - invalid signature + valid file", 6, "ed25519",
            ["--file", "test_file.txt"], ["--file", "test_file.txt"], 1,
            signature=INVALID_SIGNATURE)

verify_test("verify - P-256 - valid data", 7, "p256",
            ["--data", P256_VALID_DATA], ["--data", P256_VALID_DATA], 0)

verify_test("verify - P-256 - valid file", 7, "p256",
            ["--file", "large_test_file.txt"], ["--file", "large_test_file.txt"], 0)

verify_test("verify - P-256 - invalid data", 8, "p256",
            ["--data", P256_VALID_DATA], ["--data", "wrong_data"], 1)

verify_test("verify - P-256 - invalid file", 8, "p256",
            ["--file", "large_test_file.txt"], ["--file", "test_file.txt"], 1)

verify_test("verify - P-256 - invalid signature + valid data", 8, "p256",
            ["--data", P256_VALID_DATA], ["--data", P256_VALID_DATA], 1,
            signature=INVALID_SIGNATURE)

verify_test("verify - P-256 - invalid signature + valid file", 8, "p256",
            ["--file", "large_test_file.txt"], ["--file", "large_test_file.txt"], 1,
            signature=INVALID_SIGNATURE)

test("verify - file not found",
     ["verify", "--slot", "5", "--file", "invalid_file.txt", "--signature", INVALID_SIGNATURE],
     expected_exit_code=1)

# Device handling
run_test("status - missing device", [BINARY, "status"], expected_exit_code=1)

run_test("logs - unexpected device", hw("logs"), expected_exit_code=1)

run_test("verify - missing device",
         [BINARY, "verify", "--slot", "5", "--data", ED25519_VALID_DATA,
          "--signature", INVALID_SIGNATURE],
         expected_exit_code=1)

finish()
