#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

extern "C" {
#include <openssl/sha.h>
};

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

    try {
        this->determineHash();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << ": " << this->m_location << std::endl;
    }
    
}

fs::path Video::determineLocation() {
    auto moddPath = this->m_linkedModd->getPath();
    fs::path videoPath;
    for (const auto& ext : VIDEO_EXTS) {
        // Lower case
        videoPath = moddPath.replace_extension(ext);
        if (fs::exists(videoPath)) {
            break;
        }
        // Upper case
        string upperExt = ext;
        boost::to_upper(upperExt);
        videoPath = moddPath.replace_extension(upperExt);
        if (fs::exists(videoPath)) {
            break;
        }
    }

    return videoPath;
}

void Video::determineHash() {
    // Read in some of the file.
    std::ifstream videoFile(this->m_location);
    if (!videoFile.is_open()) {
        throw std::runtime_error("Failed to open video file");
    }

    std::vector<char> fileData;
    fileData.resize(READ_SIZE);

    videoFile.readsome(fileData.data(), fileData.size());

    videoFile.close();

    this->m_hash.resize(SHA256_DIGEST_LENGTH);

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, fileData.data(), fileData.size());
    SHA256_Final(this->m_hash.data(), &sha256);
}