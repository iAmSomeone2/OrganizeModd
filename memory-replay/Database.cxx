#include <iostream>

#include "Database.hxx"

using namespace memory_replay;

Database::Database(fs::path dbPath) {
    int result = sqlite3_open(dbPath.c_str(), &this->m_dbHandle);
    if (result != SQLITE_OK) {
        string errStr = sqlite3_errmsg(this->m_dbHandle);
        throw std::runtime_error(errStr);
    }

    // Set up any missing tables if they don't already exist.
    // Modd table
    result = this->execStatement(string(PREPARE_MODD_TABLE), 0);
    if (result != 0) {
        string errMsg = sqlite3_errmsg(this->m_dbHandle);
        throw std::runtime_error(errMsg);
    }
    // Video table
    this->execStatement(string(PREPARE_VIDEO_TABLE), 0);
    if (result != 0) {
        string errMsg = sqlite3_errmsg(this->m_dbHandle);
        throw std::runtime_error(errMsg);
    }
}

Database::~Database() {
    int result = sqlite3_close(this->m_dbHandle);
    if (result != SQLITE_OK) {
        string errStr = sqlite3_errmsg(this->m_dbHandle);
        std::cerr << errStr << std::endl;
    }
}

/**
 * Sends a one-off statement to the database. Useful for creating tables and updating
 * individual entries.
 * 
 * @param statement string version of SQL statement
 * @param flags flags to pass to the db during statement execution.
 * @return 0 on success, SQL error code otherwise. 
*/
int Database::execStatement(string statement, unsigned int flags) {
    sqlite3_stmt *sqlStmt;
    int result = sqlite3_prepare_v3(this->m_dbHandle, statement.c_str(),
        statement.length(), flags, &sqlStmt, nullptr);
    if (result != SQLITE_OK) {
        return result;
    }

    result = sqlite3_step(sqlStmt);
    if (result != SQLITE_DONE) {
        return result;
    }

    result = sqlite3_reset(sqlStmt);
    if (result != SQLITE_OK) {
        return result;
    }

    result = sqlite3_finalize(sqlStmt);
    if (result != SQLITE_OK) {
        return result;
    }

    return 0;
}