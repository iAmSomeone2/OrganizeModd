#ifndef MEMORY_REPLAY_DATABASE_HXX
#define MEMORY_REPLAY_DATABASE_HXX

#include <filesystem>
#include <string>

extern "C" {
#include <sqlite3.h>
};

namespace fs = std::filesystem;
using std::string;

namespace memory_replay {
    static const char PREPARE_MODD_TABLE[] =
    "CREATE TABLE IF NOT EXISTS modd (checkCode INTEGER UNIQUE, name TEXT, dateTime INTEGER, videoDuration REAL, videoFileSize INTEGER, moddFileLocation TEXT UNIQUE, PRIMARY KEY(checkCode))";
    static const int PREPARE_MODD_LEN = 185;
    static const char PREPARE_VIDEO_TABLE[] = 
    "CREATE TABLE IF NOT EXISTS video (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, name TEXT, moddCheckCode TEXT UNIQUE, dateTime INTEGER, duration REAL, fileLocation TEXT, fileSize INTEGER, FOREIGN KEY(moddCheckCode) REFERENCES modd(checkCode))";
    static const int PREPARE_VIDEO_LEN = 242;

    class Database {
    public:
        explicit Database(fs::path dbPath);
        ~Database();
    private:
        sqlite3 *m_dbHandle;

        int execStatement(string statement, unsigned int flags);
    };
};

#endif // MEMORY_REPLAY_DATABASE_HXX