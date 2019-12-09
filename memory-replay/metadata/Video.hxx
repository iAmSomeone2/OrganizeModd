#ifndef MEMORY_REPLAY_VIDEO_HXX
#define MEMORY_REPLAY_VIDEO_HXX

#include <string>
#include <vector>
#include <filesystem>

#include "Modd.hxx"

namespace fs = std::filesystem;


static const std::string VIDEO_EXTS[] = {".mpg", ".mpeg", ".mp4", ".m4v", ".mkv"};

enum class Container {
    MPEG,       // MPEG-1/2 container.
    MP4,        // MPEG-4 container.
    MKV,        // Matroska container.
    UNKNOWN     // Unsupported container.
};

enum class VideoCodec {
    MPEG2,      // MPEG-2 video codec
    X264,       // x264 video codec
    X265,       // x265/HEVC video codec
    UNKNOWN     // Unsupported video codec
};

enum class AudioCodec {
    AC3,        // AC3 audio codec
    AAC,        // AAC audio codec
    VORBIS,     // Vorbis audio codec
    UNKNOWN     // Unsupported audio codec
};

class Video {
public:
    explicit Video(Modd& modd);
private:
    std::string         m_name;         // Filename
    fs::path            m_location;     // Path to location on system
    uint64_t            m_creationTime; // Unix-based creation time
    double              m_duration;     // Duration in seconds
    std::vector<char>   m_hash;         // SHA-256 based hash
    Container           m_container;    // Container type
    VideoCodec          m_vidCodec;     // Video encoding codec
    AudioCodec          m_audCodec;     // Audio encoding codec
    Modd*               m_linkedModd;   // Modd associated with this video.

    fs::path determineLocation();
};

#endif // MEMORY_REPLAY_VIDEO_HXX