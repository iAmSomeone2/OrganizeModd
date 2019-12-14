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
    sqlite3_exec(this->m_dbHandle, "BEGIN TRANSACTION", 0, nullptr, nullptr);
    sqlite3_stmt *stmt;
    // Modd table
    sqlite3_prepare_v3(this->m_dbHandle, PREPARE_MODD_TABLE.c_str(), PREPARE_MODD_TABLE.length(), 0, &stmt, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Video table
    sqlite3_prepare_v3(this->m_dbHandle, PREPARE_VIDEO_TABLE.c_str(), PREPARE_VIDEO_TABLE.length(), 0, &stmt, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_exec(this->m_dbHandle, "COMMIT", 0, nullptr, nullptr);
}

Database::~Database() {
    int result = sqlite3_close(this->m_dbHandle);
    if (result != SQLITE_OK) {
        string errStr = sqlite3_errmsg(this->m_dbHandle);
        std::cerr << errStr << std::endl;
    }
}

Video Database::get(Hash hash) {
    static const std::string vidSelect = "SELECT * FROM video WHERE hash == ?";

    sqlite3_stmt *stmt;
    int result = sqlite3_prepare_v3(this->m_dbHandle, vidSelect.c_str(), vidSelect.length() + hash.size(), 0, &stmt, nullptr);
    sqlite3_bind_blob(stmt, 1, hash.data(), hash.size(), SQLITE_TRANSIENT);

    std::string name;
    uint32_t moddCheckCode;
    uint64_t dateTime;
    double duration;
    fs::path fileLoc;
    uint64_t fileSize;
    if ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        moddCheckCode = sqlite3_column_int(stmt, 2);
        dateTime = sqlite3_column_int64(stmt, 3);
        duration = sqlite3_column_double(stmt, 4);
        fileLoc = fs::path(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        fileSize = sqlite3_column_int64(stmt, 6);
    } else {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to find matching entry in database.");
    }

    sqlite3_finalize(stmt);

    return Video(name, fileLoc, dateTime, duration, hash);
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
bool Database::contains(const Modd& modd) const {
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
bool Database::contains(const Video& video) const {
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
sqlite3_stmt *Database::addEntry(const Modd& modd){
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v3(this->m_dbHandle, MODD_INS_STR.c_str(), -1, 0, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(this->m_dbHandle));
    }

    // Bind the variables
    sqlite3_bind_int(stmt, 1, modd.getCheckCode());
    sqlite3_bind_text(stmt, 2, modd.getName().c_str(), modd.getName().length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, modd.getDateTimeActual());
    sqlite3_bind_double(stmt, 4, modd.getDuration());
    sqlite3_bind_int64(stmt, 5, modd.getFileSize());
    sqlite3_bind_text(stmt, 6, modd.getPath().c_str(), modd.getPath().string().length(), SQLITE_TRANSIENT);

    return stmt;
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
        auto stmt = this->addEntry(*modd);

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
sqlite3_stmt *Database::addEntry(const Video& video){
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v3(this->m_dbHandle, VIDEO_INS_STR.c_str(), VIDEO_INS_STR.size() + 128, 0, &statement, nullptr);
    sqlite3_bind_blob(statement, 1, video.getHash().data(), video.getHash().size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, video.getName().c_str(), video.getName().size(), SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 3, video.getLinkedModd()->getCheckCode());
    sqlite3_bind_int64(statement, 4, video.getCreationTime().unixSecs());
    sqlite3_bind_double(statement, 5, video.getDuration());
    sqlite3_bind_text(statement, 6, video.getLocation().c_str(), video.getLocation().string().size(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement, 7, video.getLinkedModd()->getFileSize());
    
    return statement;
}

void Database::updateEntries(const vector<Video*> videos) {
    // Start the transaction.
    sqlite3_exec(this->m_dbHandle, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    int updateCount = 0;
    for (const auto& video : videos) {
        sqlite3_stmt *stmt = this->updateEntry(*video);

        if (stmt != nullptr) {
            int result = sqlite3_step(stmt);
            if (result == SQLITE_BUSY) {
                sqlite3_exec(this->m_dbHandle, "ROLLBACK", nullptr, nullptr, nullptr);
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to acquire db lock.");
            }
            sqlite3_finalize(stmt);
            updateCount++;
        }
    }

    // Commit the transaction.
    sqlite3_exec(this->m_dbHandle, "COMMIT", nullptr, nullptr, nullptr);
    std::clog << updateCount << " updated/added video entries." << std::endl;
}

sqlite3_stmt *Database::updateEntry(const Video& video) {
    if (!this->contains(video)) return addEntry(video);

    // First determine what has changed.
    Video originalVid = this->get(video.getHash());

    std::stringstream execStr;
    execStr << "UPDATE video SET";
    bool commaNeeded;
    bool needsUpdate;
    if (video.getName() != originalVid.getName()) {
        execStr << " name = ?2";
        commaNeeded = true;
        needsUpdate = true;
    }
    if (video.getCreationTime().unixSecs() != originalVid.getCreationTime().unixSecs()) {
        if (commaNeeded) execStr << ",";
        execStr << " dateTime = ?3";
        commaNeeded = true;
        needsUpdate = true;
    }
    if (video.getDuration() != originalVid.getDuration()) {
        if (commaNeeded) execStr << ",";
        execStr << " duration = ?4";
        commaNeeded = true;
        needsUpdate = true;
    }
    if (video.getLocation() != originalVid.getLocation()) {
        if (commaNeeded) execStr << ",";
        execStr << " fileLocation = ?5";
        needsUpdate = true;
    }
    execStr << "WHERE hash == ?1";

    if (needsUpdate) return nullptr;

    string str = execStr.str();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v3(this->m_dbHandle, str.c_str(), -1, 0, &stmt, nullptr);
    sqlite3_bind_blob(stmt, 1, video.getHash().data(), video.getHash().size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, video.getName().c_str(), video.getName().length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, video.getCreationTime().unixSecs());
    sqlite3_bind_double(stmt, 4, video.getDuration());
    sqlite3_bind_text(stmt, 5, video.getLocation().c_str(), video.getLocation().string().length(), SQLITE_TRANSIENT);

    return stmt;
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
        sqlite3_stmt *stmt = this->addEntry(*video);

        int result = sqlite3_step(stmt);
        if (result == SQLITE_BUSY) {
            sqlite3_exec(this->m_dbHandle, "ROLLBACK", nullptr, nullptr, nullptr);
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to acquire db lock.");
        }
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