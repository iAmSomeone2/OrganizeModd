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
    string sqlStr = "SELECT * FROM modd WHERE checkCode == ?";

    sqlite3_stmt *stmt;
    sqlite3_prepare_v3(this->m_dbHandle, sqlStr.c_str(), sqlStr.size() + 8, 0, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, modd.getCheckCode());

    int stepResult = sqlite3_step(stmt);
    bool result = stepResult == SQLITE_ROW;
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

/**
 * Checks if the database contains the Video object.
 * @param video object to search for
 * @return true if found in db. false, otherwise.
*/
bool Database::contains(const Video& video) {
    string sqlStr = "SELECT * FROM video WHERE hash == ?";

    sqlite3_stmt *stmt;
    sqlite3_prepare_v3(this->m_dbHandle, sqlStr.c_str(), sqlStr.size() + video.getHash().size(), 0, &stmt, nullptr);
    sqlite3_bind_blob(stmt, 1, video.getHash().data(), video.getHash().size(), SQLITE_TRANSIENT);

    int stepResult = sqlite3_step(stmt);
    bool result = stepResult == SQLITE_ROW;
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
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
        std::stringstream errStr;
        errStr << sqlite3_errmsg(this->m_dbHandle) << "\nProblem command: " << sqlStr.str() << std::endl;
        throw std::runtime_error(errStr.str());
    }
}

void Database::addEntries(const vector<Modd*> modds) {
    // Check which modds are already in the DB
    vector<Modd*> appendModds;
    for (const auto& modd : modds) {
        if (!this->contains(*modd)) {
            appendModds.push_back(modd);
        }
    }
    // Start the transaction.
    sqlite3_exec(this->m_dbHandle, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    for (const auto& modd : appendModds) {
        sqlite3_stmt *stmt;
        if(sqlite3_prepare_v3(this->m_dbHandle, MODD_INS_STR.c_str(), -1, 0, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(this->m_dbHandle));
        }

        // Bind the variables
        sqlite3_bind_int(stmt, 1, modd->getCheckCode());
        sqlite3_bind_text(stmt, 2, modd->getName().c_str(), modd->getName().length(), SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, modd->getDateTimeActual());
        sqlite3_bind_double(stmt, 4, modd->getDuration());
        sqlite3_bind_int64(stmt, 5, modd->getFileSize());
        sqlite3_bind_text(stmt, 6, modd->getPath().c_str(), modd->getPath().string().length(), SQLITE_TRANSIENT);

        sqlite3_step(stmt);
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    }

    // Commit the transaction.
    sqlite3_exec(this->m_dbHandle, "COMMIT", nullptr, nullptr, nullptr);
}

/**
 * Adds a video entry to the database.
 * @param video Video object to insert 
*/
void Database::addEntry(const Video& video) {
    // Return early if the item is already there.
    if (this->contains(video)) return;

    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v3(this->m_dbHandle, VIDEO_INS_STR.c_str(), VIDEO_INS_STR.size() + 128, 0, &statement, nullptr);
    sqlite3_bind_blob(statement, 1, video.getHash().data(), video.getHash().size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, video.getName().c_str(), video.getName().size(), SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 3, video.getLinkedModd()->getCheckCode());
    sqlite3_bind_int64(statement, 4, video.getCreationTime());
    sqlite3_bind_double(statement, 5, video.getDuration());
    sqlite3_bind_text(statement, 6, video.getLocation().c_str(), video.getLocation().string().size(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement, 7, video.getLinkedModd()->getFileSize());
    
    sqlite3_step(statement);
    this->sqliteError(result);
    result = sqlite3_reset(statement);
    this->sqliteError(result);
    result = sqlite3_finalize(statement);
    this->sqliteError(result);
}

void Database::addEntries(const vector<Video*> videos) {
    // Check which modds are already in the DB
    vector<Video*> appendVids;
    for (const auto& video : videos) {
        if (!this->contains(*video)) {
            appendVids.push_back(video);
        }
    }
    // Start the transaction.
    sqlite3_exec(this->m_dbHandle, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    for (const auto& video : appendVids) {
        sqlite3_stmt *stmt;
        if(sqlite3_prepare_v3(this->m_dbHandle, VIDEO_INS_STR.c_str(), -1, 0, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(this->m_dbHandle));
        }

        sqlite3_bind_blob(stmt, 1, video->getHash().data(), video->getHash().size(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, video->getName().c_str(), video->getName().size(), SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, video->getLinkedModd()->getCheckCode());
        sqlite3_bind_int64(stmt, 4, video->getCreationTime());
        sqlite3_bind_double(stmt, 5, video->getDuration());
        sqlite3_bind_text(stmt, 6, video->getLocation().c_str(), video->getLocation().string().size(), SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 7, video->getLinkedModd()->getFileSize());

        sqlite3_step(stmt);
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    }

    // Commit the transaction.
    sqlite3_exec(this->m_dbHandle, "COMMIT", nullptr, nullptr, nullptr);
}

void Database::sqliteError(const int& errCode) {
    if (errCode != SQLITE_OK || errCode != SQLITE_DONE) {
        std::stringstream errStr;
        errStr << sqlite3_errmsg(this->m_dbHandle) << std::endl;
        throw std::runtime_error(errStr.str());
    }
}