#ifndef MEMORY_REPLAY_DATABASE_HXX
#define MEMORY_REPLAY_DATABASE_HXX

#include <filesystem>
#include <string>
#include <vector>
#include <map>

extern "C" {
#include <sqlite3.h>
};

#include "metadata/Modd.hxx"
#include "metadata/Video.hxx"

namespace fs = std::filesystem;
using std::string;
using std::map;

namespace memory_replay {
    static const char PREPARE_MODD_TABLE[] =
    "CREATE TABLE IF NOT EXISTS modd (checkCode INTEGER UNIQUE, name TEXT, dateTime INTEGER, videoDuration REAL, videoFileSize INTEGER, moddFileLocation TEXT UNIQUE, PRIMARY KEY(checkCode))";
    static const int PREPARE_MODD_LEN = 185;
    static const char PREPARE_VIDEO_TABLE[] = 
    "CREATE TABLE IF NOT EXISTS video (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, name TEXT, moddCheckCode TEXT UNIQUE, dateTime INTEGER, duration REAL, fileLocation TEXT, fileSize INTEGER, FOREIGN KEY(moddCheckCode) REFERENCES modd(checkCode))";
    static const int PREPARE_VIDEO_LEN = 242;

    // USE WITH BOOST::FORMAT
    static const string MODD_INS_STR = "INSERT INTO \"modd\" (checkCode, name, dateTime, videoDuration, videoFileSize, moddFileLocation) VALUES (%d, \"%s\", %d, %0.15f, %d, \"%s\")";
    static const string VIDEO_INS_STR = "INSERT INTO \"video\" (name, moddCheckCode, dateTime, duration, fileLocation, fileSize) VALUES (\"%s\", %d, %d, %0.15f, \"%s\", %d)";

    typedef map<string, string> Row;    // Wraps a map of strings in a Row type.
    typedef std::vector<Row> Rows;      // Wraps a vector of Row(s) into a Rows type.

    class Database {
    public:
        explicit Database(fs::path dbPath);
        ~Database();

        Rows query(const string& stmt);

        void addEntry(const Modd& modd);
        void addEntry(const Video& video);

        void updateEntry(const Modd& modd);
        void updateEntry(const Video& video);

        bool contains(const Modd& modd);
        bool contains(const Video& video);
    private:
        sqlite3 *m_dbHandle;

        int execStatement(string statement, unsigned int flags);
        static int handleQuery(void* rows, int numCols, char** colVals, char** colNames);
    };
};

#endif // MEMORY_REPLAY_DATABASE_HXX