#include "AuditLogger.hpp"

#include <cassert>

AuditLogger::AuditLogger(const std::string& connection_string)
{
    if (sqlite3_open(connection_string.c_str(), &m_db) != SQLITE_OK)
    {
        sqlite3_close(m_db);
        m_db = nullptr;
        return;
    }

    const char* create_table_sql =
        "CREATE TABLE IF NOT EXISTS audit_log ("
        "timestamp DATETIME DEFAULT (datetime('now', 'localtime')),"
        "operation TEXT NOT NULL,"
        "auditResult TEXT NOT NULL,"
        "options TEXT NOT NULL"
        ");";

    if (sqlite3_exec(m_db, create_table_sql, nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

AuditLogger::~AuditLogger()
{
    if (m_db)
    {
        sqlite3_close(m_db);
    }
}

SystemStatus AuditLogger::fetch(std::vector<AuditEntry>& entries)
{
    if (!m_db)
    {
        return SystemStatus::AUDIT_ERROR_DB_OPEN;
    }

    const char* sql = "SELECT timestamp, operation, auditResult, options FROM audit_log;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        // TODO: Implement database mock to test this line
        return SystemStatus::AUDIT_ERROR_DB_READ;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        AuditEntry entry;
        entry.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        entry.operation = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        entry.auditResult = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        entry.options = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        entries.push_back(entry);
    }

    sqlite3_finalize(stmt);

    return SystemStatus::OK;
}

SystemStatus AuditLogger::log(Operation operation, SystemStatus auditResult, std::string options)
{
    if (!m_db)
    {
        return SystemStatus::AUDIT_ERROR_DB_OPEN;
    }

    std::string op_str = operationToString(operation);
    std::string audit_res_str = (auditResult == SystemStatus::OK) ? "SUCCESS" : "FAILED";

    // Values are bound, never concatenated into the SQL: user input (e.g. file paths) cannot
    // break or inject statements.
    const char* sql = "INSERT INTO audit_log (operation, auditResult, options) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return SystemStatus::AUDIT_ERROR_DB_WRITE;
    }

    bool bound =
        sqlite3_bind_text(stmt, 1, op_str.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_text(stmt, 2, audit_res_str.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_text(stmt, 3, options.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK;

    int rc = bound ? sqlite3_step(stmt) : SQLITE_ERROR;
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        // TODO: Implement database mock to test this line
        return SystemStatus::AUDIT_ERROR_DB_WRITE;
    }

    return SystemStatus::OK;
}