#include "AuditLogger.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("audit logger")
{
    SECTION("valid database")
    {
        AuditLogger logger(":memory:");

        SECTION("reading empty database")
        {
            std::vector<AuditEntry> entries;

            auto result = logger.fetch(entries);

            REQUIRE(result == SystemStatus::OK);
            REQUIRE(entries.size() == 0);
        }

        SECTION("write entries and read them back")
        {
            SECTION("single entries")
            {
                SECTION("status")
                {
                    auto write_result = logger.log(Operation::STATUS, SystemStatus::OK, "");

                    REQUIRE(write_result == SystemStatus::OK);

                    std::vector<AuditEntry> entries;
                    auto read_result = logger.fetch(entries);

                    REQUIRE(read_result == SystemStatus::OK);
                    REQUIRE(entries.size() == 1);
                    REQUIRE(entries[0].timestamp != "");
                    REQUIRE(entries[0].operation == "status");
                    REQUIRE(entries[0].auditResult == "SUCCESS");
                    REQUIRE(entries[0].options == "");
                }

                SECTION("erase key")
                {
                    auto write_result =
                        logger.log(Operation::ERASE_KEY, SystemStatus::OK, "slot=2");

                    REQUIRE(write_result == SystemStatus::OK);

                    std::vector<AuditEntry> entries;
                    auto read_result = logger.fetch(entries);

                    REQUIRE(read_result == SystemStatus::OK);
                    REQUIRE(entries.size() == 1);
                    REQUIRE(entries[0].timestamp != "");
                    REQUIRE(entries[0].operation == "erase-key");
                    REQUIRE(entries[0].auditResult == "SUCCESS");
                    REQUIRE(entries[0].options == "slot=2");
                }

                SECTION("generate key")
                {
                    auto write_result = logger.log(
                        Operation::GENERATE_KEY,
                        SystemStatus::DEVICE_ERROR_GENERATE_KEY_SLOT_OCCUPIED, "slot=2 curve=p256");

                    REQUIRE(write_result == SystemStatus::OK);

                    std::vector<AuditEntry> entries;
                    auto read_result = logger.fetch(entries);

                    REQUIRE(read_result == SystemStatus::OK);
                    REQUIRE(entries.size() == 1);
                    REQUIRE(entries[0].timestamp != "");
                    REQUIRE(entries[0].operation == "generate-key");
                    REQUIRE(entries[0].auditResult == "FAILED");
                    REQUIRE(entries[0].options == "slot=2 curve=p256");
                }

                SECTION("read key")
                {
                    auto write_result = logger.log(Operation::READ_KEY,
                                                   SystemStatus::DEVICE_ERROR_DEINIT, "slot=2");

                    REQUIRE(write_result == SystemStatus::OK);

                    std::vector<AuditEntry> entries;
                    auto read_result = logger.fetch(entries);

                    REQUIRE(read_result == SystemStatus::OK);
                    REQUIRE(entries.size() == 1);
                    REQUIRE(entries[0].timestamp != "");
                    REQUIRE(entries[0].operation == "read-key");
                    REQUIRE(entries[0].auditResult == "FAILED");
                    REQUIRE(entries[0].options == "slot=2");
                }

                SECTION("list all keys")
                {
                    auto write_result =
                        logger.log(Operation::LIST_KEYS, SystemStatus::DEVICE_ERROR_DEINIT, "");

                    REQUIRE(write_result == SystemStatus::OK);

                    std::vector<AuditEntry> entries;
                    auto read_result = logger.fetch(entries);

                    REQUIRE(read_result == SystemStatus::OK);
                    REQUIRE(entries.size() == 1);
                    REQUIRE(entries[0].timestamp != "");
                    REQUIRE(entries[0].operation == "list-keys");
                    REQUIRE(entries[0].auditResult == "FAILED");
                    REQUIRE(entries[0].options == "");
                }
            }

            SECTION("multiple entries preserve order")
            {
                SystemStatus write_result;
                int n_entries = 3;

                write_result = logger.log(Operation::STATUS, SystemStatus::OK, "");
                REQUIRE(write_result == SystemStatus::OK);
                write_result = logger.log(Operation::ERASE_KEY, SystemStatus::OK, "slot=6");
                REQUIRE(write_result == SystemStatus::OK);
                write_result =
                    logger.log(Operation::GENERATE_KEY, SystemStatus::OK, "slot=15 curve=ed25519");
                REQUIRE(write_result == SystemStatus::OK);

                std::vector<AuditEntry> entries;
                auto read_result = logger.fetch(entries);

                REQUIRE(read_result == SystemStatus::OK);
                REQUIRE(entries.size() == n_entries);

                REQUIRE(entries[0].timestamp != "");
                REQUIRE(entries[0].operation == "status");
                REQUIRE(entries[0].auditResult == "SUCCESS");
                REQUIRE(entries[0].options == "");

                REQUIRE(entries[1].timestamp != "");
                REQUIRE(entries[1].operation == "erase-key");
                REQUIRE(entries[1].auditResult == "SUCCESS");
                REQUIRE(entries[1].options == "slot=6");

                REQUIRE(entries[2].timestamp != "");
                REQUIRE(entries[2].operation == "generate-key");
                REQUIRE(entries[2].auditResult == "SUCCESS");
                REQUIRE(entries[2].options == "slot=15 curve=ed25519");
            }
        }
    }

    SECTION("invalid database")
    {
        AuditLogger logger("/invalid/path/hardwarden.db");

        SECTION("reading from database fails")
        {
            std::vector<AuditEntry> entries;

            auto result = logger.fetch(entries);

            REQUIRE(result == SystemStatus::AUDIT_ERROR_DB_OPEN);
        }

        SECTION("writing to database fails")
        {
            auto result = logger.log(Operation::ERASE_KEY, SystemStatus::OK, "");

            REQUIRE(result == SystemStatus::AUDIT_ERROR_DB_OPEN);
        }
    }
}