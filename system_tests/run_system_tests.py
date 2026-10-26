import subprocess

BINARY = "../build/src/cli/hsmctl"

ED25519_PUBKEY = "6c6078d631fed56cbf94920cb7ddc8bc8b3203f1bd261a2445d75ce65f760758"
ED25519_PUBKEY_INVALID = "0000000000000000000000000000000000000000000000000000000000000000"
ED25519_VALID_SIGNATURE_DATA = "7e5f36d0eb98c67019dee0ee37f808582c872f6e2bbcbd66ff8eafcb732bcc56869f44db916c83a045fc06f70fd0d52510ffda476755a345ca9b9989679df207"
ED25519_VALID_SIGNATURE_FILE = "c414bbda6715eb5dfe7418620845923b44ac73c355d0093b3a733299bcf3c581d2c7a63e78d1ee8647bbcaff3626ecdd86877c9f881d6aa0e44ca32bed7fad0c"
ED25519_VALID_DATA = "hello"

P256_PUBKEY = "a8dbccb2160c3b3c155413211068563d084331e2e8d1516a4c68800c12e171f235c7985e4d6a9b7fb36f8daba7dab5dcc2a43767fe17f55fd6914bf5a6b5b913"
P256_PUBKEY_INVALID = "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
P256_VALID_SIGNATURE_DATA = "64869bae63735f4f5f9b537ba88a1a25d0d49b6e54cf485aa76291ab3f5daae7b3ab1146d05b7c4baf02c9c503aefec34e906999ab0380ec949a163c58f88422"
P256_VALID_SIGNATURE_FILE = "cdbc567050471775c41fad6f6535c9fd80ca85197b3052f4dd732e290ddee1804d1cf7e00d06df4c1f5db4f084ae2cfe20d336280a7ab4fcc5007b7c78b95ba2"
P256_VALID_DATA = "hsmctl"

INVALID_SIGNATURE = "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

def extract_signature_from_output(output):
    """Extract hex signature from hsmctl sign command output.
    
    WARNING: This parsing depends on CliDisplay::signResult() format.
    If that format changes, this function must be updated.
    """
    lines = output.split('\n')
    signature = ""
    in_sig_block = False
    
    for line in lines:
        # Enter signature block after "Signature:" line
        if "Signature:" in line:
            in_sig_block = True
            continue
        
        # Skip dash lines
        if line.startswith('-') or line.strip() == "":
            if in_sig_block and signature:  # Exit if we've already collected sig
                break
            continue
        
        # Collect hex lines
        if in_sig_block and line.strip():
            if all(c in '0123456789abcdefABCDEF' for c in line.strip()):
                signature += line.strip()
    
    return signature if signature else None

def make_get_signature(sign_command):
    """Factory function that creates a signature extractor for a specific sign command"""
    def get_signature():
        sign_result = run_command(sign_command)
        return extract_signature_from_output(sign_result.stdout)
    return get_signature

def run_command(command):
    return subprocess.run(command, capture_output=True, text=True)

def run_test(description, command, expected_exit_code=0, setup=None, cleanup=None):
    setup_result = None
    
    if setup:
        if isinstance(setup[0], list) or callable(setup[0]):
            # List of commands/callables
            for s in setup:
                if callable(s):
                    setup_result = s()
                else:
                    run_command(s)
        else:
            # Single command
            run_command(setup)

    # If command is callable, pass setup_result to it
    if callable(command):
        actual_command = command(setup_result)
    else:
        actual_command = command

    result = run_command(actual_command)

    if cleanup:
        if isinstance(cleanup[0], list):
            for c in cleanup:
                run_command(c)
        else:
            run_command(cleanup)

    if result.returncode == expected_exit_code:
        print(f"\033[92mPASS\033[0m: {description}")
    else:
        print(f"\033[91mFAIL\033[0m: {description} (expected {expected_exit_code}, got {result.returncode})")



print(f"\033[93m{'=' * 50}\033[0m")
print(f"\033[93mWARNING: System tests will erase ALL keys from\033[0m")
print(f"\033[93mthe HSM before running.\033[0m")
print()
print(f"\033[93mDo NOT run on a production device.\033[0m")
print(f"\033[93m{'=' * 50}\033[0m")
print()
response = input("Continue? (yes/no): ")
if response.lower() != "yes":
    print("Aborted.")
    exit(0)



print("\n" + "="*50)
print("Running System Tests")
print("="*50 + "\n")

print("Preparing HSM...")
for slot in range(32):
    run_command([BINARY, "tropic01", "erase-key", "--slot", str(slot)])
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
run_test("status - help", [BINARY, "tropic01", "status", "--help"])
run_test("status", [BINARY, "tropic01", "status"])

# Logs
run_test("logs - help", [BINARY, "logs", "--help"])
run_test("logs", [BINARY, "logs"])

# Erase key
run_test("erase key - help", [BINARY, "tropic01", "erase-key", "--help"])
run_test("erase key", [BINARY, "tropic01", "erase-key", "--slot", "1"])

# Generate key
run_test("generate key - help", [BINARY, "tropic01", "generate-key", "--help"])

run_test("generate key - unspecified curve",
         [BINARY, "tropic01", "generate-key", "--slot", "1"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "1"])

run_test("generate key - Ed25519 curve",
         [BINARY, "tropic01", "generate-key", "--slot", "2", "--curve", "ed25519"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "2"])

run_test("generate key - NIST P-256 curve",
         [BINARY, "tropic01", "generate-key", "--slot", "3", "--curve", "p256"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "3"])

run_test("generate key - curve specified first",
         [BINARY, "tropic01", "generate-key", "--curve", "p256", "--slot", "3"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "3"])   

run_test("generate key - occupied slot",
         [BINARY, "tropic01", "generate-key", "--slot", "10"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "10"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "10"],
         expected_exit_code=1)

# Read key
run_test("read key - help", [BINARY, "tropic01", "read-key", "--help"])

run_test("read key - Ed25519",
         [BINARY, "tropic01", "read-key", "--slot", "17"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "17"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "17"])

run_test("read key - NIST P-256",
         [BINARY, "tropic01", "read-key", "--slot", "25"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "25", "--curve", "p256"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "25"])

run_test("read key - empty slot",
         [BINARY, "tropic01", "read-key", "--slot", "10"],
         expected_exit_code=1)

# List all keys
run_test("list all keys - help", [BINARY, "tropic01", "list-keys", "--help"])

run_test("list all keys - no keys", [BINARY, "tropic01", "list-keys"])

run_test("list all keys - one key",
         [BINARY, "tropic01", "list-keys"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "5"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "5"])

run_test("list all keys - multiple keys",
         [BINARY, "tropic01", "list-keys"],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "5"],
             [BINARY, "tropic01", "generate-key", "--slot", "6"],
             [BINARY, "tropic01", "generate-key", "--slot", "7"],
         ],
         cleanup=[
             [BINARY, "tropic01", "erase-key", "--slot", "5"],
             [BINARY, "tropic01", "erase-key", "--slot", "6"],
             [BINARY, "tropic01", "erase-key", "--slot", "7"],
         ])

run_test("list all keys - verbose",
         [BINARY, "tropic01", "list-keys", "--verbose"],
                  setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "6"],
             [BINARY, "tropic01", "generate-key", "--slot", "7"],
         ],
         cleanup=[
             [BINARY, "tropic01", "erase-key", "--slot", "6"],
             [BINARY, "tropic01", "erase-key", "--slot", "7"],
         ])

# Sign
run_test("sign - help", [BINARY, "tropic01", "sign", "--help"])

run_test("sign - data (Ed25519)",
         [BINARY, "tropic01", "sign", "--slot", "18", "--data", "hello"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "18"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "18"])

run_test("sign - data (P-256)",
         [BINARY, "tropic01", "sign", "--slot", "18", "--data", "hello"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "18", "--curve", "p256"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "18"])

run_test("sign - file (Ed25519)",
         [BINARY, "tropic01", "sign", "--slot", "19", "--file", "test_file.txt"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "19"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "19"])

run_test("sign - file (P-256)",
         [BINARY, "tropic01", "sign", "--slot", "19", "--file", "test_file.txt"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "19", "--curve", "p256"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "19"]) 

run_test("sign - large file (P-256)",
         [BINARY, "tropic01", "sign", "--slot", "19", "--file", "large_test_file.txt"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "19", "--curve", "p256"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "19"])      

run_test("sign - file not found",
         [BINARY, "tropic01", "sign", "--slot", "23", "--file", "nonexistent.txt"],
         setup=[BINARY, "tropic01", "generate-key", "--slot", "23"],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "23"],
         expected_exit_code=1)

run_test("sign - key not found (data)",
         [BINARY, "tropic01", "sign", "--slot", "23", "--data", "hello"],
         expected_exit_code=1)

run_test("sign - key not found (file)",
         [BINARY, "tropic01", "sign", "--slot", "23", "--file", "test_file.txt"],
         expected_exit_code=1) 

# Verify
run_test("verify - help", [BINARY, "verify", "--help"])  

run_test("verify - Ed25519 - HSM key - valid data",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "0", "--data", ED25519_VALID_DATA, "--signature", sig],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "0"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "0", "--data", ED25519_VALID_DATA])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "0"])

run_test("verify - Ed25519 - HSM key - valid file",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "0", "--file", "test_file.txt", "--signature", sig],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "0"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "0", "--file", "test_file.txt"])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "0"])     

run_test("verify - Ed25519 - HSM key - invalid data",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "2", "--data", "wrong_data", "--signature", sig],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "2"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "2", "--data", ED25519_VALID_DATA])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - Ed25519 - HSM key - invalid file",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "2", "--file", "large_test_file.txt", "--signature", sig],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "2"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "2", "--file", "test_file.txt"])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - Ed25519 - HSM key - invalid signature + valid data",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "2", "--data", ED25519_VALID_DATA, "--signature", INVALID_SIGNATURE],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "2"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "2", "--data", ED25519_VALID_DATA])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - Ed25519 - HSM key - invalid signature + valid file",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "2", "--file", "test_file.txt", "--signature", INVALID_SIGNATURE],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "2"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "2", "--file", "test_file.txt"])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - Ed25519 - user provided key - valid data",
         [BINARY, "verify", "--pubkey", ED25519_PUBKEY, "--data", ED25519_VALID_DATA, "--signature", ED25519_VALID_SIGNATURE_DATA])

run_test("verify - Ed25519 - user provided key - valid file",
         [BINARY, "verify", "--pubkey", ED25519_PUBKEY, "--file", "test_file.txt", "--signature", ED25519_VALID_SIGNATURE_FILE])

run_test("verify - Ed25519 - user provided key - invalid key",
         [BINARY, "verify", "--pubkey", ED25519_PUBKEY_INVALID, "--data", ED25519_VALID_DATA, "--signature", ED25519_VALID_SIGNATURE_DATA],
         expected_exit_code=1)

run_test("verify - Ed25519 - user provided key - invalid data",
         [BINARY, "verify", "--pubkey", ED25519_PUBKEY, "--data", "invalid_data", "--signature", ED25519_VALID_SIGNATURE_DATA],
         expected_exit_code=1)

run_test("verify - Ed25519 - user provided key - invalid file",
         [BINARY, "verify", "--pubkey", ED25519_PUBKEY, "--file", "large_test_file.txt", "--signature", ED25519_VALID_SIGNATURE_FILE],
         expected_exit_code=1)

run_test("verify - Ed25519 - user provided key - invalid signature + valid data",
         [BINARY, "verify", "--pubkey", ED25519_PUBKEY, "--data", ED25519_VALID_DATA, "--signature", INVALID_SIGNATURE],
         expected_exit_code=1)

run_test("verify - Ed25519 - user provided key - invalid signature + valid file",
         [BINARY, "verify", "--pubkey", ED25519_PUBKEY, "--file", "test_file.txt", "--signature", INVALID_SIGNATURE],
         expected_exit_code=1)

run_test("verify - P-256 - HSM key - valid data",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "1", "--data", P256_VALID_DATA, "--signature", sig],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "1", "--curve", "p256"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "1", "--data", P256_VALID_DATA])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "1"])

run_test("verify - P-256 - HSM key - valid file",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "1", "--file", "large_test_file.txt", "--signature", sig],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "1", "--curve", "p256"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "1", "--file", "large_test_file.txt"])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "1"])

run_test("verify - P-256 - HSM key - invalid data",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "3", "--data", "wrong_data", "--signature", sig],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "3", "--curve", "p256"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "3", "--data", P256_VALID_DATA])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "3"],
         expected_exit_code=1)

run_test("verify - P-256 - HSM key - invalid file",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "3", "--file", "test_file.txt", "--signature", sig],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "3", "--curve", "p256"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "3", "--file", "large_test_file.txt"])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "3"],
         expected_exit_code=1)

run_test("verify - P-256 - HSM key - invalid signature + valid data",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "2", "--data", P256_VALID_DATA, "--signature", INVALID_SIGNATURE],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "2", "--curve", "p256"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "2", "--data", P256_VALID_DATA])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - P-256 - HSM key - invalid signature + valid file",
         lambda sig: [BINARY, "tropic01", "verify", "--slot", "2", "--file", "large_test_file.txt", "--signature", INVALID_SIGNATURE],
         setup=[
             [BINARY, "tropic01", "generate-key", "--slot", "2", "--curve", "p256"],
             make_get_signature([BINARY, "tropic01", "sign", "--slot", "2", "--file", "large_test_file.txt"])
         ],
         cleanup=[BINARY, "tropic01", "erase-key", "--slot", "2"],
         expected_exit_code=1)

run_test("verify - P-256 - user provided key - valid data",
         [BINARY, "verify", "--pubkey", P256_PUBKEY, "--data", P256_VALID_DATA, "--signature", P256_VALID_SIGNATURE_DATA])

run_test("verify - P-256 - user provided key - valid file",
         [BINARY, "verify", "--pubkey", P256_PUBKEY, "--file", "large_test_file.txt", "--signature", P256_VALID_SIGNATURE_FILE])

run_test("verify - P-256 - user provided key - invalid key",
         [BINARY, "verify", "--pubkey", P256_PUBKEY_INVALID, "--data", P256_VALID_DATA, "--signature", P256_VALID_SIGNATURE_DATA],
         expected_exit_code=1)

run_test("verify - P-256 - user provided key - invalid data",
         [BINARY, "verify", "--pubkey", P256_PUBKEY, "--data", "invalid_data", "--signature", P256_VALID_SIGNATURE_DATA],
         expected_exit_code=1)

run_test("verify - P-256 - user provided key - invalid file",
         [BINARY, "verify", "--pubkey", P256_PUBKEY, "--file", "test_file.txt", "--signature", P256_VALID_SIGNATURE_FILE],
         expected_exit_code=1)

run_test("verify - P-256 - user provided key - invalid signature + valid data",
         [BINARY, "verify", "--pubkey", P256_PUBKEY, "--data", P256_VALID_DATA, "--signature", INVALID_SIGNATURE],
         expected_exit_code=1)

run_test("verify - P-256 - user provided key - invalid signature + valid file",
         [BINARY, "verify", "--pubkey", P256_PUBKEY, "--file", "large_test_file.txt", "--signature", INVALID_SIGNATURE],
         expected_exit_code=1)

run_test("verify - file not found",
         [BINARY, "verify", "--pubkey", P256_PUBKEY, "--file", "invalid_file.txt", "--signature", P256_VALID_SIGNATURE_FILE],
         expected_exit_code=1)

# Device handling
run_test("status - missing device", [BINARY, "status"], expected_exit_code=1)

run_test("logs - unexpected device", [BINARY, "tropic01", "logs"], expected_exit_code=1)

run_test("verify pubkey - unexpected device",
         [BINARY, "tropic01", "verify", "--pubkey", ED25519_PUBKEY, "--data", ED25519_VALID_DATA,
          "--signature", ED25519_VALID_SIGNATURE_DATA],
         expected_exit_code=1)

print("")    