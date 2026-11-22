"""System tests for TROPIC01 devices. Usage: python3 run_tropic01_tests.py <device>"""

import sys

from helpers import (BINARY, ED25519_VALID_DATA, INVALID_SIGNATURE, P256_VALID_DATA,
                     confirm_erase, finish, make_get_signature, print_header, run_command,
                     run_test)

DEVICES = ["tropic01-spi", "tropic01-usb"]

if len(sys.argv) != 2 or sys.argv[1] not in DEVICES:
    print(f"Usage: python3 {sys.argv[0]} <device>")
    print(f"Devices: {', '.join(DEVICES)}")
    sys.exit(1)

DEVICE = sys.argv[1]

confirm_erase(DEVICE)
print_header()

print("Preparing the device...")
for slot in range(32):
    run_command([BINARY, DEVICE, "erase-key", "--slot", str(slot)])
print("Ready.\n")

# TODO: Setting up and cleaning up should only be done once
# at the beggining of the tests and at the end of the tests,
# respectively. This avoids overhead.

# TODO: Find a way to run tests in the same session without
# needing to initialize and deinitialize hardware between 
# each test. This avoids overhead.

# Help
run_test("help menu", [BINARY, "--help"])

# Status
run_test("status - help", [BINARY, DEVICE, "status", "--help"])
run_test("status", [BINARY, DEVICE, "status"])

# Logs
run_test("logs - help", [BINARY, "logs", "--help"])
run_test("logs", [BINARY, "logs"])

# Erase key
run_test("erase key - help", [BINARY, DEVICE, "erase-key", "--help"])
run_test("erase key", [BINARY, DEVICE, "erase-key", "--slot", "1"])

# Generate key
run_test("generate key - help", [BINARY, DEVICE, "generate-key", "--help"])

run_test("generate key - unspecified curve",
         [BINARY, DEVICE, "generate-key", "--slot", "1"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "1"])

run_test("generate key - Ed25519 curve",
         [BINARY, DEVICE, "generate-key", "--slot", "2", "--curve", "ed25519"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "2"])

run_test("generate key - NIST P-256 curve",
         [BINARY, DEVICE, "generate-key", "--slot", "3", "--curve", "p256"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "3"])

run_test("generate key - curve specified first",
         [BINARY, DEVICE, "generate-key", "--curve", "p256", "--slot", "3"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "3"])   

run_test("generate key - occupied slot",
         [BINARY, DEVICE, "generate-key", "--slot", "10"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "10"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "10"],
         expected_exit_code=1)

# Read key
run_test("read key - help", [BINARY, DEVICE, "read-key", "--help"])

run_test("read key - Ed25519",
         [BINARY, DEVICE, "read-key", "--slot", "17"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "17"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "17"])

run_test("read key - NIST P-256",
         [BINARY, DEVICE, "read-key", "--slot", "25"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "25", "--curve", "p256"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "25"])

run_test("read key - empty slot",
         [BINARY, DEVICE, "read-key", "--slot", "10"],
         expected_exit_code=1)

# List all keys
run_test("list all keys - help", [BINARY, DEVICE, "list-keys", "--help"])

run_test("list all keys - no keys", [BINARY, DEVICE, "list-keys"])

run_test("list all keys - one key",
         [BINARY, DEVICE, "list-keys"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "5"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "5"])

run_test("list all keys - multiple keys",
         [BINARY, DEVICE, "list-keys"],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "5"],
             [BINARY, DEVICE, "generate-key", "--slot", "6"],
             [BINARY, DEVICE, "generate-key", "--slot", "7"],
         ],
         cleanup=[
             [BINARY, DEVICE, "erase-key", "--slot", "5"],
             [BINARY, DEVICE, "erase-key", "--slot", "6"],
             [BINARY, DEVICE, "erase-key", "--slot", "7"],
         ])

run_test("list all keys - verbose",
         [BINARY, DEVICE, "list-keys", "--verbose"],
                  setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "6"],
             [BINARY, DEVICE, "generate-key", "--slot", "7"],
         ],
         cleanup=[
             [BINARY, DEVICE, "erase-key", "--slot", "6"],
             [BINARY, DEVICE, "erase-key", "--slot", "7"],
         ])

# Sign
run_test("sign - help", [BINARY, DEVICE, "sign", "--help"])

run_test("sign - data (Ed25519)",
         [BINARY, DEVICE, "sign", "--slot", "18", "--data", "hello"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "18"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "18"])

run_test("sign - data (P-256)",
         [BINARY, DEVICE, "sign", "--slot", "18", "--data", "hello"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "18", "--curve", "p256"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "18"])

run_test("sign - file (Ed25519)",
         [BINARY, DEVICE, "sign", "--slot", "19", "--file", "test_file.txt"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "19"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "19"])

run_test("sign - file (P-256)",
         [BINARY, DEVICE, "sign", "--slot", "19", "--file", "test_file.txt"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "19", "--curve", "p256"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "19"]) 

run_test("sign - large file (P-256)",
         [BINARY, DEVICE, "sign", "--slot", "19", "--file", "large_test_file.txt"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "19", "--curve", "p256"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "19"])      

run_test("sign - file not found",
         [BINARY, DEVICE, "sign", "--slot", "23", "--file", "nonexistent.txt"],
         setup=[BINARY, DEVICE, "generate-key", "--slot", "23"],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "23"],
         expected_exit_code=1)

run_test("sign - key not found (data)",
         [BINARY, DEVICE, "sign", "--slot", "23", "--data", "hello"],
         expected_exit_code=1)

run_test("sign - key not found (file)",
         [BINARY, DEVICE, "sign", "--slot", "23", "--file", "test_file.txt"],
         expected_exit_code=1) 

# Verify
run_test("verify - help", [BINARY, DEVICE, "verify", "--help"])  

run_test("verify - Ed25519 - valid data",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "0", "--data", ED25519_VALID_DATA, "--signature", sig],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "0"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "0", "--data", ED25519_VALID_DATA])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "0"])

run_test("verify - Ed25519 - valid file",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "0", "--file", "test_file.txt", "--signature", sig],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "0"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "0", "--file", "test_file.txt"])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "0"])     

run_test("verify - Ed25519 - invalid data",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "2", "--data", "wrong_data", "--signature", sig],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "2"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "2", "--data", ED25519_VALID_DATA])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - Ed25519 - invalid file",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "2", "--file", "large_test_file.txt", "--signature", sig],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "2"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "2", "--file", "test_file.txt"])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - Ed25519 - invalid signature + valid data",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "2", "--data", ED25519_VALID_DATA, "--signature", INVALID_SIGNATURE],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "2"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "2", "--data", ED25519_VALID_DATA])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - Ed25519 - invalid signature + valid file",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "2", "--file", "test_file.txt", "--signature", INVALID_SIGNATURE],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "2"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "2", "--file", "test_file.txt"])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - P-256 - valid data",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "1", "--data", P256_VALID_DATA, "--signature", sig],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "1", "--curve", "p256"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "1", "--data", P256_VALID_DATA])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "1"])

run_test("verify - P-256 - valid file",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "1", "--file", "large_test_file.txt", "--signature", sig],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "1", "--curve", "p256"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "1", "--file", "large_test_file.txt"])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "1"])

run_test("verify - P-256 - invalid data",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "3", "--data", "wrong_data", "--signature", sig],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "3", "--curve", "p256"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "3", "--data", P256_VALID_DATA])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "3"],
         expected_exit_code=1)

run_test("verify - P-256 - invalid file",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "3", "--file", "test_file.txt", "--signature", sig],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "3", "--curve", "p256"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "3", "--file", "large_test_file.txt"])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "3"],
         expected_exit_code=1)

run_test("verify - P-256 - invalid signature + valid data",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "2", "--data", P256_VALID_DATA, "--signature", INVALID_SIGNATURE],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "2", "--curve", "p256"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "2", "--data", P256_VALID_DATA])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - P-256 - invalid signature + valid file",
         lambda sig: [BINARY, DEVICE, "verify", "--slot", "2", "--file", "large_test_file.txt", "--signature", INVALID_SIGNATURE],
         setup=[
             [BINARY, DEVICE, "generate-key", "--slot", "2", "--curve", "p256"],
             make_get_signature([BINARY, DEVICE, "sign", "--slot", "2", "--file", "large_test_file.txt"])
         ],
         cleanup=[BINARY, DEVICE, "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - file not found",
         [BINARY, DEVICE, "verify", "--slot", "0", "--file", "invalid_file.txt",
          "--signature", INVALID_SIGNATURE],
         expected_exit_code=1)

# Device handling
run_test("status - missing device", [BINARY, "status"], expected_exit_code=1)

run_test("logs - unexpected device", [BINARY, DEVICE, "logs"], expected_exit_code=1)

run_test("verify - missing device",
         [BINARY, "verify", "--slot", "0", "--data", ED25519_VALID_DATA,
          "--signature", INVALID_SIGNATURE],
         expected_exit_code=1)

finish()
