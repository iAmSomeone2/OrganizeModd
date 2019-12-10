#include "Video.hxx"

using namespace memory_replay;

Video::Video(const Modd& modd) {
    this->m_linkedModd = &modd;
    this->m_location = this->determineLocation();
    this->m_name = this->m_location.filename();
    this->m_creationTime = this->m_linkedModd->getDateTimeActual();
    this->m_duration = this->m_linkedModd->getDuration();

    // Determine which container is being used.
    std::string vidExt = this->m_location.extension().string();
    for (const auto& extPair : CONTAINER_MAP) {
        if (vidExt == extPair.first) {
            this->m_container = extPair.second;
            break;
        }
    }

    this->m_audCodec = AudioCodec::UNKNOWN;
    this->m_vidCodec = VideoCodec::UNKNOWN;
}

fs::path Video::determineLocation() {
    auto moddPath = this->m_linkedModd->getPath();
    fs::path videoPath;
    for (const auto& ext : VIDEO_EXTS) {
        videoPath = moddPath.replace_extension(ext);
        if (fs::exists(videoPath)) {
            break;
        }
    }

    return videoPath;
}