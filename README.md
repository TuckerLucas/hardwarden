# Hardwarden

A command line tool for managing keys on hardware security devices.

Hardwarden gives different devices one simple, consistent set of commands: generate, list, read and erase keys, and sign and verify data. Private keys are generated inside the hardware and never leave it, and every operation is recorded in a persistent audit log.

Currently supported: the [TROPIC01](https://www.tropicsquare.com/tropic01) secure element.

## Hardware Requirements

The following hardware is required in order to use Hardwarden: 

- Raspberry Pi - available at [Raspberry Pi Foundation](https://www.raspberrypi.com/)

- TROPIC01 Raspberry Pi Shield - available at [Tropic Square](https://www.tropicsquare.com/order-devboard-form)

The TROPIC01 shield mounts directly onto the Raspberry Pi GPIO header and communicates over SPI. You can get started with the TROPIC01 with the tutorials provided [here](https://tropicsquare.github.io/libtropic/latest/tutorials/linux/spi/).

## Dependencies

```bash
sudo apt install git
sudo apt install build-essential
sudo apt install cmake
sudo apt install libsqlite3-dev
sudo apt install libssl-dev
```

## Building and installing

The following steps build and install the `hwarden` command: 

```bash
git clone https://github.com/TuckerLucas/hardwarden
cd hardwarden
mkdir build && cd build
cmake ..
make
sudo make install
```

## Supported operations

| Operation | Description | Options |
|---------|-------------|---------|
| `status` | Check if the hardware device is connected | — |
| `logs` | Display the full audit log (deviceless) | — |
| `generate-key` | Generate an ECC key pair in a hardware slot | `<--slot <0-31>>` `[--curve <ed25519 \| p256>]` |
| `erase-key` | Erase the key stored in a slot | `<--slot <0-31>>` |
| `read-key` | Read back the public key from a slot | `<--slot <0-31>>` |
| `list-keys` | List all public keys stored on the hardware device | `[--verbose]` |
| `sign` | Sign data or a file using a hardware backed key | `<--slot <0-31>>` `<--data <data> \| --file <path>>` |
| `verify` | Verify a signature using a hardware backed key | `<--slot <0-31>>` `<--data <data> \| --file <path>>` `<--signature <signature>>` |

## Quick start

```bash
# Confirm the hardware device is connected
hwarden tropic01 status

# Generate an ECC key pair in slot 0 (Ed25519 by default)
hwarden tropic01 generate-key --slot 0

# Generate a NIST P-256 ECC key pair in slot 1
hwarden tropic01 generate-key --slot 1 --curve p256

# Generate an Ed25519 ECC key pair in slot 29
hwarden tropic01 generate-key --slot 29 --curve ed25519

# Read back the public key from slot 0
hwarden tropic01 read-key --slot 0

# Erase the key from slot 0
hwarden tropic01 erase-key --slot 0

# List all public keys stored on the hardware
hwarden tropic01 list-keys

# Sign a message using the key in slot 1 (outputs 1st signature)
hwarden tropic01 sign --slot 1 --data "hello from hardwarden"

# Create a sample file to sign 
echo "test file content" > sample.txt

# Sign the file using the key in slot 29 (outputs 2nd signature)
hwarden tropic01 sign --slot 29 --file sample.txt

# Verify the signed data
hwarden tropic01 verify --slot 1 --data "hello from hardwarden" --signature <1st signature>

# Verify the signed file
hwarden tropic01 verify --slot 29 --file sample.txt --signature <2nd signature>

# View the audit log
hwarden logs
```

The `list-keys` operation presents a list of all public keys stored on the hardware, as shown below:

```
------------------------------------------------------------
Slot    Curve     Public Key
------------------------------------------------------------
1       P-256     bf98aad487c19154...f3b8a20a
29      Ed25519   f11fe3f0c136961f...b720f27a
------------------------------------------------------------
2 key(s) found
```

All operations are logged automatically to `~/.hardwarden/audit.db` and can be retrieved using the `logs` operation. The quick start commands above produce the following:

```
Audit Log
---------------------------------------------------------------------------
Timestamp             Operation      Result    Options             
---------------------------------------------------------------------------
2026-09-19 16:37:01   status         SUCCESS                       
2026-09-19 16:37:10   generate-key   SUCCESS   slot=0  curve=ed25519
2026-09-19 16:37:18   generate-key   SUCCESS   slot=1  curve=p256  
2026-09-19 16:37:27   generate-key   SUCCESS   slot=29 curve=ed25519
2026-09-19 16:37:35   read-key       SUCCESS   slot=0              
2026-09-19 16:37:48   erase-key      SUCCESS   slot=0              
2026-09-19 16:37:59   list-keys      SUCCESS                       
2026-09-19 16:38:43   sign           SUCCESS   slot=1  type=data   
2026-09-19 16:39:07   sign           SUCCESS   slot=29 type=file path=sample.txt
2026-09-19 16:39:37   verify         SUCCESS   slot=1  type=data   
2026-09-19 16:39:59   verify         SUCCESS   slot=29 type=file path=sample.txt
---------------------------------------------------------------------------
11 operations logged
```

## Roadmap

Multi-device support for additional hardware security devices.
