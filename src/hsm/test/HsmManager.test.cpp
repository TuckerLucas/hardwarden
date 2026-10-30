#include "HsmManager.hpp"

#include <catch2/catch_test_macros.hpp>

#include "MockAuditLogger.hpp"
#include "MockLibtropicSecureElement.hpp"

// TODO: add empty payload test for sign. Should fail

TEST_CASE("hardware status")
{
    MockLibtropicSecureElement mock;
    MockAuditLogger mock_logger;
    HsmManager hsm(mock, mock_logger);

    SECTION("success")
    {
        auto result = hsm.status();

        REQUIRE(result == SystemStatus::OK);
    }

    SECTION("hardware initialization fails")
    {
        mock.initResult = SystemStatus::HSM_ERROR_INIT;

        auto result = hsm.status();

        REQUIRE(result == SystemStatus::HSM_ERROR_INIT);
    }

    SECTION("ping fails")
    {
        mock.statusResult = SystemStatus::HSM_ERROR_STATUS;

        auto result = hsm.status();

        REQUIRE(result == SystemStatus::HSM_ERROR_STATUS);
    }

    SECTION("hardware deinitialization fails")
    {
        mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

        auto result = hsm.status();

        REQUIRE(result == SystemStatus::HSM_ERROR_DEINIT);
    }

    SECTION("logging")
    {
        SECTION("success")
        {
            hsm.status();

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::STATUS);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
            REQUIRE(mock_logger.lastOptions == "");
        }

        SECTION("hardware initialization fails")
        {
            mock.initResult = SystemStatus::HSM_ERROR_INIT;

            hsm.status();

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::STATUS);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
            REQUIRE(mock_logger.lastOptions == "");
        }

        SECTION("ping fails")
        {
            mock.statusResult = SystemStatus::HSM_ERROR_STATUS;

            hsm.status();

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::STATUS);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_STATUS);
            REQUIRE(mock_logger.lastOptions == "");
        }

        SECTION("hardware deinitialization fails")
        {
            mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

            hsm.status();

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::STATUS);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
            REQUIRE(mock_logger.lastOptions == "");
        }
    }
}

TEST_CASE("erase key")
{
    MockLibtropicSecureElement mock;
    MockAuditLogger mock_logger;
    HsmManager hsm(mock, mock_logger);
    uint8_t slot = 29;

    SECTION("success")
    {
        auto result = hsm.eraseKey(slot);

        REQUIRE(result == SystemStatus::OK);
    }

    SECTION("hardware initialization fails")
    {
        mock.initResult = SystemStatus::HSM_ERROR_INIT;

        auto result = hsm.eraseKey(slot);

        REQUIRE(result == SystemStatus::HSM_ERROR_INIT);
    }

    SECTION("erasing key fails")
    {
        mock.eraseKeyResult = SystemStatus::HSM_ERROR_ERASE_KEY;

        auto result = hsm.eraseKey(slot);

        REQUIRE(result == SystemStatus::HSM_ERROR_ERASE_KEY);
    }

    SECTION("hardware deinitialization fails")
    {
        mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

        auto result = hsm.eraseKey(slot);

        REQUIRE(result == SystemStatus::HSM_ERROR_DEINIT);
    }

    SECTION("logging")
    {
        SECTION("success")
        {
            hsm.eraseKey(slot);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::ERASE_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
            REQUIRE(mock_logger.lastOptions == "slot=" + std::to_string(slot));
        }

        SECTION("hardware initialization fails")
        {
            mock.initResult = SystemStatus::HSM_ERROR_INIT;

            hsm.eraseKey(slot);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::ERASE_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
            REQUIRE(mock_logger.lastOptions == "slot=" + std::to_string(slot));
        }

        SECTION("erasing key fails")
        {
            mock.eraseKeyResult = SystemStatus::HSM_ERROR_ERASE_KEY;

            hsm.eraseKey(slot);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::ERASE_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_ERASE_KEY);
            REQUIRE(mock_logger.lastOptions == "slot=" + std::to_string(slot));
        }

        SECTION("hardware deinitialization fails")
        {
            mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

            hsm.eraseKey(slot);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::ERASE_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
            REQUIRE(mock_logger.lastOptions == "slot=" + std::to_string(slot));
        }
    }
}

TEST_CASE("generate key")
{
    MockLibtropicSecureElement mock;
    MockAuditLogger mock_logger;
    HsmManager hsm(mock, mock_logger);
    uint8_t slot = 6;
    Curve curve = Curve::P256;

    SECTION("success")
    {
        SECTION("Ed25519 curve")
        {
            auto result = hsm.generateKey(slot, Curve::Ed25519);

            REQUIRE(mock.lastCurve == Curve::Ed25519);
            REQUIRE(result == SystemStatus::OK);
        }

        SECTION("P-256 curve")
        {
            auto result = hsm.generateKey(slot, Curve::P256);

            REQUIRE(mock.lastCurve == Curve::P256);
            REQUIRE(result == SystemStatus::OK);
        }
    }

    SECTION("hardware initialization fails")
    {
        mock.initResult = SystemStatus::HSM_ERROR_INIT;

        auto result = hsm.generateKey(slot, curve);

        REQUIRE(result == SystemStatus::HSM_ERROR_INIT);
    }

    SECTION("generating key fails")
    {
        mock.generateKeyResult = SystemStatus::HSM_ERROR_GENERATE_KEY_SLOT_OCCUPIED;

        auto result = hsm.generateKey(slot, curve);

        REQUIRE(result == SystemStatus::HSM_ERROR_GENERATE_KEY_SLOT_OCCUPIED);
    }

    SECTION("hardware deinitialization fails")
    {
        mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

        auto result = hsm.generateKey(slot, curve);

        REQUIRE(result == SystemStatus::HSM_ERROR_DEINIT);
    }

    SECTION("logging")
    {
        SECTION("success")
        {
            hsm.generateKey(slot, curve);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::GENERATE_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
            REQUIRE(mock_logger.lastOptions == "slot=6  curve=p256");
        }

        SECTION("hardware initialization fails")
        {
            mock.initResult = SystemStatus::HSM_ERROR_INIT;

            hsm.generateKey(slot, curve);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::GENERATE_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
            REQUIRE(mock_logger.lastOptions == "slot=6  curve=p256");
        }

        SECTION("generating key fails")
        {
            mock.generateKeyResult = SystemStatus::HSM_ERROR_GENERATE_KEY_HW_ERROR;

            hsm.generateKey(slot, curve);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::GENERATE_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_GENERATE_KEY_HW_ERROR);
            REQUIRE(mock_logger.lastOptions == "slot=6  curve=p256");
        }

        SECTION("hardware deinitialization fails")
        {
            mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

            hsm.generateKey(slot, curve);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::GENERATE_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
            REQUIRE(mock_logger.lastOptions == "slot=6  curve=p256");
        }
    }
}

TEST_CASE("read key")
{
    MockLibtropicSecureElement mock;
    MockAuditLogger mock_logger;
    HsmManager hsm(mock, mock_logger);
    uint8_t slot = 8;
    std::vector<uint8_t> pubKey;

    SECTION("success")
    {
        auto result = hsm.readKey(slot, pubKey);

        REQUIRE(pubKey == mock_ed25519_public_key);
        REQUIRE(result == SystemStatus::OK);
    }

    SECTION("hardware initialization fails")
    {
        mock.initResult = SystemStatus::HSM_ERROR_INIT;

        auto result = hsm.readKey(slot, pubKey);

        REQUIRE(result == SystemStatus::HSM_ERROR_INIT);
    }

    SECTION("reading key fails")
    {
        mock.readKeyResult = SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT;

        auto result = hsm.readKey(slot, pubKey);

        REQUIRE(result == SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT);
    }

    SECTION("hardware deinitialization fails")
    {
        mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

        auto result = hsm.readKey(slot, pubKey);

        REQUIRE(result == SystemStatus::HSM_ERROR_DEINIT);
    }

    SECTION("logging")
    {
        SECTION("success")
        {
            hsm.readKey(slot, pubKey);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::READ_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
            REQUIRE(mock_logger.lastOptions == "slot=" + std::to_string(slot));
        }

        SECTION("hardware initialization fails")
        {
            mock.initResult = SystemStatus::HSM_ERROR_INIT;

            hsm.readKey(slot, pubKey);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::READ_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
            REQUIRE(mock_logger.lastOptions == "slot=" + std::to_string(slot));
        }

        SECTION("reading key fails")
        {
            mock.readKeyResult = SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;

            hsm.readKey(slot, pubKey);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::READ_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR);
            REQUIRE(mock_logger.lastOptions == "slot=" + std::to_string(slot));
        }

        SECTION("hardware deinitialization fails")
        {
            mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

            hsm.readKey(slot, pubKey);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::READ_KEY);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
            REQUIRE(mock_logger.lastOptions == "slot=" + std::to_string(slot));
        }
    }
}

TEST_CASE("list all keys")
{
    MockLibtropicSecureElement mock;
    MockAuditLogger mock_logger;
    HsmManager hsm(mock, mock_logger);
    std::vector<std::vector<uint8_t>> pubKeys;

    SECTION("success")
    {
        auto result = hsm.listKeys(pubKeys);

        REQUIRE(result == SystemStatus::OK);
    }

    SECTION("hardware initialization fails")
    {
        mock.initResult = SystemStatus::HSM_ERROR_INIT;

        auto result = hsm.listKeys(pubKeys);

        REQUIRE(result == SystemStatus::HSM_ERROR_INIT);
    }

    SECTION("listing keys fails")
    {
        mock.listKeysResult = SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT;

        auto result = hsm.listKeys(pubKeys);

        REQUIRE(result == SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT);
    }

    SECTION("hardware deinitialization fails")
    {
        mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

        auto result = hsm.listKeys(pubKeys);

        REQUIRE(result == SystemStatus::HSM_ERROR_DEINIT);
    }

    SECTION("logging")
    {
        SECTION("success")
        {
            hsm.listKeys(pubKeys);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::LIST_KEYS);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
            REQUIRE(mock_logger.lastOptions == "");
        }

        SECTION("hardware initialization fails")
        {
            mock.initResult = SystemStatus::HSM_ERROR_INIT;

            hsm.listKeys(pubKeys);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::LIST_KEYS);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
            REQUIRE(mock_logger.lastOptions == "");
        }

        SECTION("listing keys fails")
        {
            mock.listKeysResult = SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;

            hsm.listKeys(pubKeys);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::LIST_KEYS);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR);
            REQUIRE(mock_logger.lastOptions == "");
        }

        SECTION("hardware deinitialization fails")
        {
            mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

            hsm.listKeys(pubKeys);

            REQUIRE(mock_logger.logCalled == true);
            REQUIRE(mock_logger.lastOperation == Operation::LIST_KEYS);
            REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
            REQUIRE(mock_logger.lastOptions == "");
        }
    }
}

TEST_CASE("sign")
{
    MockLibtropicSecureElement mock;
    MockAuditLogger mock_logger;
    HsmManager hsm(mock, mock_logger);
    uint8_t slot = 15;
    std::vector<uint8_t> payload;
    std::vector<uint8_t> signature;
    DataSource dataSource = DataSource::DATA;
    std::string filepath;

    SECTION("success")
    {
        SECTION("data")
        {
            dataSource = DataSource::DATA;

            SECTION("Ed25519")
            {
                mock.readKeyPubKey = mock_ed25519_public_key;
                payload = std::vector<uint8_t>(ED25519_MAX_MSG_SIZE, 0x00);

                auto result = hsm.sign(slot, payload, signature, dataSource);

                REQUIRE(signature == mock_signature);
                REQUIRE(mock.lastCurve == Curve::Ed25519);
                REQUIRE(result == SystemStatus::OK);
            }

            SECTION("P-256")
            {
                mock.readKeyPubKey = mock_p256_public_key;
                payload = std::vector<uint8_t>(ED25519_MAX_MSG_SIZE + 1, 0x00);

                auto result = hsm.sign(slot, payload, signature, dataSource);

                REQUIRE(signature == mock_signature);
                REQUIRE(mock.lastCurve == Curve::P256);
                REQUIRE(result == SystemStatus::OK);
            }
        }

        SECTION("file")
        {
            dataSource = DataSource::FILE;

            SECTION("Ed25519")
            {
                mock.readKeyPubKey = mock_ed25519_public_key;
                payload = std::vector<uint8_t>(ED25519_MAX_MSG_SIZE, 0x00);

                auto result = hsm.sign(slot, payload, signature, dataSource, filepath);

                REQUIRE(signature == mock_signature);
                REQUIRE(mock.lastCurve == Curve::Ed25519);
                REQUIRE(result == SystemStatus::OK);
            }

            SECTION("P-256")
            {
                mock.readKeyPubKey = mock_p256_public_key;
                payload = std::vector<uint8_t>(ED25519_MAX_MSG_SIZE + 1, 0x00);

                auto result = hsm.sign(slot, payload, signature, dataSource, filepath);

                REQUIRE(signature == mock_signature);
                REQUIRE(mock.lastCurve == Curve::P256);
                REQUIRE(result == SystemStatus::OK);
            }
        }
    }

    SECTION("hardware initialization fails")
    {
        mock.initResult = SystemStatus::HSM_ERROR_INIT;

        auto result = hsm.sign(slot, payload, signature, dataSource);

        REQUIRE(result == SystemStatus::HSM_ERROR_INIT);
    }

    SECTION("reading key fails")
    {
        mock.readKeyResult = SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT;

        auto result = hsm.sign(slot, payload, signature, dataSource);

        REQUIRE(result == SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT);
    }

    SECTION("payload too large for Ed25519")
    {
        payload = std::vector<uint8_t>(ED25519_MAX_MSG_SIZE + 1, 0x00);

        auto result = hsm.sign(slot, payload, signature, dataSource);

        REQUIRE(result == SystemStatus::HSM_ERROR_SIGN_PAYLOAD_TOO_LARGE);
    }

    SECTION("signing fails")
    {
        mock.signResult = SystemStatus::HSM_ERROR_SIGN;

        auto result = hsm.sign(slot, payload, signature, dataSource);

        REQUIRE(result == SystemStatus::HSM_ERROR_SIGN);
    }

    SECTION("hardware deinitialization fails")
    {
        mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

        auto result = hsm.sign(slot, payload, signature, dataSource);

        REQUIRE(result == SystemStatus::HSM_ERROR_DEINIT);
    }

    SECTION("logging")
    {
        SECTION("data")
        {
            dataSource = DataSource::DATA;

            SECTION("success")
            {
                hsm.sign(slot, payload, signature, dataSource);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=data");
            }

            SECTION("hardware initialization fails")
            {
                mock.initResult = SystemStatus::HSM_ERROR_INIT;

                hsm.sign(slot, payload, signature, dataSource);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=data");
            }

            SECTION("reading key fails")
            {
                mock.readKeyResult = SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;

                hsm.sign(slot, payload, signature, dataSource);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=data");
            }

            SECTION("payload too large for Ed25519")
            {
                payload = std::vector<uint8_t>(ED25519_MAX_MSG_SIZE + 1, 0x00);

                auto result = hsm.sign(slot, payload, signature, dataSource);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus ==
                        SystemStatus::HSM_ERROR_SIGN_PAYLOAD_TOO_LARGE);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=data");

                REQUIRE(result == SystemStatus::HSM_ERROR_SIGN_PAYLOAD_TOO_LARGE);
            }

            SECTION("signing fails")
            {
                mock.signResult = SystemStatus::HSM_ERROR_SIGN;

                hsm.sign(slot, payload, signature, dataSource);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_SIGN);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=data");
            }

            SECTION("hardware deinitialization fails")
            {
                mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

                hsm.sign(slot, payload, signature, dataSource);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=data");
            }
        }

        SECTION("file")
        {
            dataSource = DataSource::FILE;
            filepath = "filepath/file.txt";

            SECTION("success")
            {
                hsm.sign(slot, payload, signature, dataSource, filepath);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=file path=filepath/file.txt");
            }

            SECTION("hardware initialization fails")
            {
                mock.initResult = SystemStatus::HSM_ERROR_INIT;

                hsm.sign(slot, payload, signature, dataSource, filepath);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=file path=filepath/file.txt");
            }

            SECTION("reading key fails")
            {
                mock.readKeyResult = SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT;

                hsm.sign(slot, payload, signature, dataSource, filepath);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus ==
                        SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=file path=filepath/file.txt");
            }

            SECTION("payload too large for Ed25519")
            {
                payload = std::vector<uint8_t>(ED25519_MAX_MSG_SIZE + 1, 0x00);

                auto result = hsm.sign(slot, payload, signature, dataSource, filepath);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus ==
                        SystemStatus::HSM_ERROR_SIGN_PAYLOAD_TOO_LARGE);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=file path=filepath/file.txt");

                REQUIRE(result == SystemStatus::HSM_ERROR_SIGN_PAYLOAD_TOO_LARGE);
            }

            SECTION("signing fails")
            {
                mock.signResult = SystemStatus::HSM_ERROR_SIGN;

                hsm.sign(slot, payload, signature, dataSource, filepath);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_SIGN);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=file path=filepath/file.txt");
            }

            SECTION("hardware deinitialization fails")
            {
                mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

                hsm.sign(slot, payload, signature, dataSource, filepath);

                REQUIRE(mock_logger.logCalled == true);
                REQUIRE(mock_logger.lastOperation == Operation::SIGN);
                REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
                REQUIRE(mock_logger.lastOptions == "slot=15 type=file path=filepath/file.txt");
            }
        }
    }
}

TEST_CASE("verify")
{
    MockLibtropicSecureElement mock;
    MockAuditLogger mock_logger;
    HsmManager hsm(mock, mock_logger);
    std::vector<uint8_t> payload;
    std::vector<uint8_t> signature;
    DataSource dataSource = DataSource::DATA;
    std::string filepath;

    SECTION("verify with HSM key")
    {
        uint8_t slot = 17;

        SECTION("success")
        {
            auto result = hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

            REQUIRE(result == SystemStatus::OK);
        }

        SECTION("hardware initialization fails")
        {
            mock.initResult = SystemStatus::HSM_ERROR_INIT;

            auto result = hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

            REQUIRE(result == SystemStatus::HSM_ERROR_INIT);
        }

        SECTION("reading key fails")
        {
            mock.readKeyResult = SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;

            auto result = hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

            REQUIRE(result == SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR);
        }

        SECTION("verification fails")
        {
            mock.verifyResult = SystemStatus::HSM_ERROR_VERIFY_INVALID_SIGNATURE;

            auto result = hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

            REQUIRE(result == SystemStatus::HSM_ERROR_VERIFY_INVALID_SIGNATURE);
        }

        SECTION("hardware deinitialization fails")
        {
            mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

            auto result = hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

            REQUIRE(result == SystemStatus::HSM_ERROR_DEINIT);
        }

        SECTION("logging")
        {
            SECTION("data")
            {
                dataSource = DataSource::DATA;

                SECTION("success")
                {
                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=data");
                }

                SECTION("hardware initialization fails")
                {
                    mock.initResult = SystemStatus::HSM_ERROR_INIT;

                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=data");
                }

                SECTION("reading key fails")
                {
                    mock.readKeyResult = SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR;

                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus ==
                            SystemStatus::HSM_ERROR_READ_KEY_HW_ERROR);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=data");
                }

                SECTION("verification fails")
                {
                    mock.verifyResult = SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR;

                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus ==
                            SystemStatus::HSM_ERROR_VERIFY_CRYPTO_ERROR);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=data");
                }

                SECTION("hardware deinitialization fails")
                {
                    mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=data");
                }
            }

            SECTION("file")
            {
                dataSource = DataSource::FILE;
                filepath = "filepath/file.txt";

                SECTION("success")
                {
                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource, filepath);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus == SystemStatus::OK);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=file path=filepath/file.txt");
                }

                SECTION("hardware initialization fails")
                {
                    mock.initResult = SystemStatus::HSM_ERROR_INIT;

                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource, filepath);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_INIT);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=file path=filepath/file.txt");
                }

                SECTION("reading key fails")
                {
                    mock.readKeyResult = SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT;

                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource, filepath);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus ==
                            SystemStatus::HSM_ERROR_READ_KEY_EMPTY_SLOT);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=file path=filepath/file.txt");
                }

                SECTION("verification fails")
                {
                    mock.verifyResult = SystemStatus::HSM_ERROR_VERIFY_INVALID_SIGNATURE;

                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource, filepath);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus ==
                            SystemStatus::HSM_ERROR_VERIFY_INVALID_SIGNATURE);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=file path=filepath/file.txt");
                }

                SECTION("hardware deinitialization fails")
                {
                    mock.deinitResult = SystemStatus::HSM_ERROR_DEINIT;

                    hsm.verifyWithHsmKey(slot, payload, signature, dataSource, filepath);

                    REQUIRE(mock_logger.logCalled == true);
                    REQUIRE(mock_logger.lastOperation == Operation::VERIFY);
                    REQUIRE(mock_logger.lastSystemStatus == SystemStatus::HSM_ERROR_DEINIT);
                    REQUIRE(mock_logger.lastOptions == "slot=17 type=file path=filepath/file.txt");
                }
            }
        }
    }
}