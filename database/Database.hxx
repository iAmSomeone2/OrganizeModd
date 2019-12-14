#ifndef MEMORY_REPLAY_DATABASE_HXX
#define MEMORY_REPLAY_DATABASE_HXX

#include <filesystem>
#include <string>
#include <vector>
#include <map>

extern "C" {
#include <sqlite3.h>
};

#include "../metadata/Modd.hxx"
#include "../metadata/Video.hxx"

namespace fs = std::filesystem;
using std::string;
using std::vector;
using std::map;

namespace memory_replay {
    static const string PREPARE_MODD_TABLE =
    "CREATE TABLE IF NOT EXISTS modd (checkCode INTEGER UNIQUE, name TEXT, dateTime INTEGER, videoDuration REAL, videoFileSize INTEGER, moddFileLocation TEXT UNIQUE, PRIMARY KEY(checkCode))";
    static const string PREPARE_VIDEO_TABLE = 
    "CREATE TABLE IF NOT EXISTS video (hash BLOB PRIMARY KEY UNIQUE, name TEXT, moddCheckCode TEXT UNIQUE, dateTime INTEGER, duration REAL, fileLocation TEXT, fileSize INTEGER, FOREIGN KEY(moddCheckCode) REFERENCES modd(checkCode))";

    // USE WITH BOOST::FORMAT
    static const string MODD_INS_STR = "INSERT INTO \"modd\" (checkCode, name, dateTime, videoDuration, videoFileSize, moddFileLocation) VALUES (?, ?, ?, ?, ?, ?)";
    static const string VIDEO_INS_STR = "INSERT INTO \"video\" (hash, name, moddCheckCode, dateTime, duration, fileLocation, fileSize) VALUES (?, ?, ?, ?, ?, ?, ?)";

    typedef map<string, string> Row;    // Wraps a map of strings in a Row type.
    typedef vector<Row> Rows;      // Wraps a vector of Row(s) into a Rows type.

    class Database {
    public:
        explicit Database(fs::path dbPath);
        ~Database();

        Rows query(const string& stmt);

        Video   get(Hash hash);
        Modd    get(uint32_t checkCode);

        void updateEntries(const vector<Video*> videos);

        void addEntries(const vector<Modd*> modds);
        void addEntries(const vector<Video*> videos);

        bool contains(const Modd& modd) const;
        bool contains(const Video& video) const;
    private:
        sqlite3 *m_dbHandle;

        sqlite3_stmt *addEntry(const Modd& modd);
        sqlite3_stmt *addEntry(const Video& video);

        sqlite3_stmt *updateEntry(const Modd& modd);
        sqlite3_stmt *updateEntry(const Video& video);

        void sqliteError(const int& errCode);
        int execStatement(string statement, unsigned int flags);
        static int handleQuery(void* rows, int numCols, char** colVals, char** colNames);
    };
};

#endif // MEMORY_REPLAY_DATABASE_HXX