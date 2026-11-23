"""Plumbing shared by the device test scripts."""

import os
import subprocess
import sys

# Run from this folder, so the binary and the test files are found wherever the script is started.
os.chdir(os.path.dirname(os.path.abspath(__file__)))

BINARY = "../build/src/cli/hwarden"

ED25519_VALID_DATA = "hello"
P256_VALID_DATA = "hardwarden"
INVALID_SIGNATURE = "0" * 128

_results = {"passed": 0, "failed": 0}


def run_command(command, input=None):
    """Runs a command. Without input, stdin is empty, so a secret prompt never waits for a person."""
    if input is None:
        return subprocess.run(command, capture_output=True, text=True, stdin=subprocess.DEVNULL)
    return subprocess.run(command, capture_output=True, text=True, input=input)


def _run_step(step):
    """Runs one setup or cleanup step: a command list or a callable. Returns the callable's result."""
    if callable(step):
        return step()
    run_command(step)
    return None


def _steps(steps):
    """Accepts a single step or a list of steps."""
    if steps is None:
        return []
    if callable(steps) or (isinstance(steps, list) and steps and isinstance(steps[0], str)):
        return [steps]
    return steps


def run_test(description, command, expected_exit_code=0, setup=None, cleanup=None, input=None):
    """Runs setup, the command and cleanup, and checks the command's exit code.

    If command is callable, it receives the result of the last callable setup step
    (for example a signature) and returns the command to run.
    """
    setup_result = None
    for step in _steps(setup):
        result = _run_step(step)
        if callable(step):
            setup_result = result

    actual_command = command(setup_result) if callable(command) else command

    if any(part is None for part in actual_command):
        for step in _steps(cleanup):
            _run_step(step)
        _results["failed"] += 1
        print(f"\033[91mFAIL\033[0m: {description} (setup failed)")
        return

    result = run_command(actual_command, input)

    for step in _steps(cleanup):
        _run_step(step)

    if result.returncode == expected_exit_code:
        _results["passed"] += 1
        print(f"\033[92mPASS\033[0m: {description}")
    else:
        _results["failed"] += 1
        print(f"\033[91mFAIL\033[0m: {description} "
              f"(expected {expected_exit_code}, got {result.returncode})")


def extract_signature_from_output(output):
    """Extracts the hex signature from the sign command's output.

    WARNING: This parsing depends on CliDisplay::signResult() format.
    If that format changes, this function must be updated.
    """
    signature = ""
    in_sig_block = False

    for line in output.split("\n"):
        if "Signature:" in line:
            in_sig_block = True
            continue

        if line.startswith("-") or line.strip() == "":
            if in_sig_block and signature:
                break
            continue

        if in_sig_block and all(c in "0123456789abcdefABCDEF" for c in line.strip()):
            signature += line.strip()

    return signature if signature else None


def make_get_signature(sign_command, input=None):
    """Returns a callable that signs and returns the signature, for use as a setup step."""
    def get_signature():
        return extract_signature_from_output(run_command(sign_command, input).stdout)
    return get_signature


def confirm_erase(device):
    """Warns that all keys will be erased and asks for confirmation. Exits if not confirmed."""
    print(f"\033[93m{'=' * 50}\033[0m")
    print("\033[93mWARNING: System tests will erase ALL keys from\033[0m")
    print(f"\033[93mthe {device} device before running.\033[0m")
    print()
    print("\033[93mDo NOT run on a production device.\033[0m")
    print(f"\033[93m{'=' * 50}\033[0m")
    print()
    if input("Continue? (yes/no): ").lower() != "yes":
        print("Aborted.")
        sys.exit(0)


def print_header():
    print("\n" + "=" * 50)
    print("Running System Tests")
    print("=" * 50 + "\n")


def finish():
    """Prints the totals and exits with 1 if any test failed."""
    total = _results["passed"] + _results["failed"]
    print(f"\n{_results['passed']}/{total} passed, {_results['failed']} failed")
    sys.exit(1 if _results["failed"] else 0)
