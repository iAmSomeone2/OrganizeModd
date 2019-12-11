#include <iostream>
#include <sstream>

#include <boost/format.hpp>

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
 * Finds Rows matching the provided statement.
 * 
 * @param stmt SQL select statement.
 * @return a vector containing all matched rows.
*/
Rows Database::query(const string& stmt) {
    Rows rows;

    int result = sqlite3_exec(
        this->m_dbHandle,
        stmt.c_str(),
        this->handleQuery,
        &rows,
        nullptr
    );

    return rows;
}

/**
 * Callback for handling db queries.
 * Appends a row (map<string, string>) to a Rows vector.
*/
int Database::handleQuery(void* rows, int numCols, char** colVals, char** colNames) {
    auto rowsPtr = static_cast<Rows*>(rows);
    Row row;
    for (int i = 0; i < numCols; i++) {
        row[string(colNames[i])] = string(colVals[i]);
    }

    rowsPtr->push_back(row);
    return 0;
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

/**
 * Checks if the database contains the Modd object.
 * @param modd object to search for
 * @return true if found in db. false, otherwise.
*/
bool Database::contains(const Modd& modd) {
    std::stringstream sqlStr;
    sqlStr << boost::format("SELECT * FROM modd WHERE checkCode == %d") % modd.getCheckCode();

    Rows rows = this->query(sqlStr.str());

    return rows.size() != 0;
}

/**
 * Checks if the database contains the Video object.
 * @param video object to search for
 * @return true if found in db. false, otherwise.
*/
bool Database::contains(const Video& video) {
    std::stringstream sqlStr;
    // TODO: update to use hashes.
    sqlStr << boost::format("SELECT * FROM video WHERE moddCheckCode == %d") % video.getLinkedModd()->getCheckCode();

    Rows rows = this->query(sqlStr.str());

    return rows.size() != 0;
}

/**
 * Adds a modd entry to the database.
 * @param Modd object to insert
*/
void Database::addEntry(const Modd& modd) {
    // Return early if the item is already there.
    if (this->contains(modd)) return;

    std::stringstream sqlStr;
    sqlStr << boost::format(MODD_INS_STR) % modd.getCheckCode() % modd.getName() % modd.getDateTimeActual()
        % modd.getDuration() % modd.getFileSize() % modd.getPath().string();

    int result = this->execStatement(sqlStr.str(), 0);
    if (result != 0) {
        string errMsg = sqlite3_errmsg(this->m_dbHandle);
        throw std::runtime_error(errMsg);
    }
}

/**
 * Adds a video entry to the database.
 * @param video Video object to insert 
*/
void Database::addEntry(const Video& video) {
    // Return early if the item is already there.
    if (this->contains(video)) return;

    std::stringstream sqlStr;
    sqlStr << boost::format(VIDEO_INS_STR) % video.getName() % video.getLinkedModd()->getCheckCode() % video.getCreationTime()
        % video.getDuration() % video.getLocation().string() % video.getLinkedModd()->getFileSize();

    int result = this->execStatement(sqlStr.str(), 0);
    if (result != 0) {
        string errMsg = sqlite3_errmsg(this->m_dbHandle);
        throw std::runtime_error(errMsg);
    }
}