#ifndef MEMORY_REPLAY_MODD_HXX
#define MEMORY_REPLAY_MODD_HXX

#include <string>
#include <filesystem>
#include <vector>

static const char XML_HEADER[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
static const char DATA_HEADER[] = "<plist version=\"1.0\"><dict><key>MetaDataList</key><array><dict>";
static const int HEADER_LENGTH = 101;
static const char DATA_FOOTER[] = "</dict></array><key>XMLFileType</key><string>ModdXML</string></dict></plist>";

static const uint64_t UNIX_MINUS_COM_EPOCH = 2209161600;

/**
 * Selection of American time zones assigned their UTC offsets.
 */
enum class TimeZone {
    EST = 5,    // Eastern Standard Time
    CST,        // Central Standard Time
    MST,        // Mountain Standard Time
    PST         // Pacific Standard Time
};

static std::string replaceFirst(std::string& s, const std::string& toReplace, const std::string& replaceWith);
static std::string replaceAll(std::string& s, const std::string& toReplace, const std::string& replaceWith);

class Modd {
public:
    explicit Modd(const std::filesystem::path& moddFilePath);
protected:
    void setActualTime(const TimeZone& tz);
private:
    std::string             m_name;             // Derived from file name
    std::filesystem::path   m_location;         // Location of .modd file in filesystem
    uint32_t                m_checkCode;        // Unknown hash algorithm
    float                   m_dateTimeOriginal; // Measured in days since Dec. 30 1899
    uint64_t                m_dateTimeActual;   // Unix-standard version of m_dateTimeOriginal
    float                   m_duration;          // Seconds in clip.
    uint64_t                m_fileSize;         // Size of file in bytes.
    //VT[]                    m_vtList;           // Unkown purpose. Potentially video timings

    static std::string vecToString(std::vector<char> txt);
    static std::string cleanText(char* moddText, std::size_t size);
    static std::string cleanText(std::string& moddText);
};

#endif // MEMORY_REPLAY_MODD_HXX