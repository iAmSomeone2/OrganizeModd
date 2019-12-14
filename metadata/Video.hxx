#ifndef MEMORY_REPLAY_VIDEO_HXX
#define MEMORY_REPLAY_VIDEO_HXX

#include <string>
#include <vector>
#include <map>
#include <filesystem>

#include "Modd.hxx"
#include "Time.hxx"

namespace fs = std::filesystem;
using std::string;

namespace memory_replay {
    static const std::string VIDEO_EXTS[] = {".mpg", ".mpeg", ".mp4", ".m4v", ".mkv", ".avi"};

    static const uint32_t READ_SIZE = 5120000; // 5MiB

    enum class Container {
        MPEG,       // MPEG-1/2 container.
        MP4,        // MPEG-4 container.
        MKV,        // Matroska container.
        AVI,        // AVI container.
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

    static const std::map<std::string, Container> CONTAINER_MAP({
        {VIDEO_EXTS[0], Container::MPEG},
        {VIDEO_EXTS[1], Container::MPEG},
        {VIDEO_EXTS[2], Container::MP4},
        {VIDEO_EXTS[3], Container::MP4},
        {VIDEO_EXTS[4], Container::MKV},
        {VIDEO_EXTS[5], Container::AVI}
    });

    typedef std::vector<uint8_t> Hash;

    class Video {
    public:
        Video(string name, fs::path loc, uint64_t createTime, double duration, Hash hash);
        explicit Video(Modd& modd);

        bool relocate(const fs::path& rootDir);

        // Getters
        /**
         * Gets the name of the video.
         * @return string containing name.
        */
        string      getName()           const { return this->m_name; };
        fs::path    getLocation()       const { return this->m_location; };
        Time        getCreationTime()   const { return this->m_creationTime; };
        double      getDuration()       const { return this->m_duration; };
        Hash        getHash()           const { return this->m_hash; };
        Container   getContainer()      const { return this->m_container; };
        VideoCodec  getVideoCodec()     const { return this->m_vidCodec; };
        AudioCodec  getAudioCodec()     const { return this->m_audCodec; };
        Modd*       getLinkedModd()     const { return this->m_linkedModd; };
    private:
        string              m_name;         // Filename
        fs::path            m_location;     // Path to location on system
        Time                m_creationTime; // Unix-based creation time
        double              m_duration;     // Duration in seconds
        Hash                m_hash;         // SHA-256 based hash
        Container           m_container;    // Container type
        VideoCodec          m_vidCodec;     // Video encoding codec
        AudioCodec          m_audCodec;     // Audio encoding codec
        Modd*               m_linkedModd;   // Modd associated with this video.

        /**
         * Confirms location of video file based on location of associated Modd. 
        */
        fs::path determineLocation();

        void determineHash();
    };
};


#endif // MEMORY_REPLAY_VIDEO_HXX