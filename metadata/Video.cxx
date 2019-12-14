#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

extern "C" {
#include <openssl/sha.h>
};

#include "Video.hxx"

using namespace memory_replay;

Video::Video(string name, fs::path loc, uint64_t createTime, double duration, Hash hash) {
    this->m_name = name;
    this->m_location = loc;
    this->m_creationTime.set(createTime);
    this->m_duration = duration;
    this->m_hash = hash;

    // Determine which container is being used.
    std::string vidExt = this->m_location.extension().string();
    for (const auto& extPair : CONTAINER_MAP) {
        if (vidExt == extPair.first) {
            this->m_container = extPair.second;
            break;
        }
    }

    this->m_audCodec = AudioCodec::UNKNOWN;

    if (this->m_container == Container::MPEG) {
        this->m_vidCodec = VideoCodec::MPEG2;
    } else {
        this->m_vidCodec = VideoCodec::UNKNOWN;
    }

    this->m_linkedModd = nullptr;
}

Video::Video(Modd& modd) {
    this->m_linkedModd = &modd;
    this->m_location = this->determineLocation();
    this->m_name = this->m_location.filename();
    this->m_creationTime.set(this->m_linkedModd->getDateTimeActual());
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

    if (this->m_container == Container::MPEG) {
        this->m_vidCodec = VideoCodec::MPEG2;
    } else {
        this->m_vidCodec = VideoCodec::UNKNOWN;
    }
    
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

/**
 * Uses the creation time data from the video to determine the appropriate directory structure. 
*/
bool Video::relocate(const fs::path& rootDir) {
    std::stringstream newPath;
    newPath << boost::format("%d/%s/") % this->m_creationTime.year() % MONTH_STR[this->m_creationTime.month()];

    fs::path outDir = rootDir;
    outDir.concat(newPath.str());
    
    bool success;

    try {
        success = fs::create_directories(outDir);
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() <<  std::endl;
    }

    fs::path outPath = outDir;
    outPath.concat(this->m_name);

    try {
        success = fs::copy_file(this->m_location, outPath);
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
    }

    try{
        if (success){
            success = fs::remove(this->m_location);
            this->m_location = outPath;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
    }

    this->m_linkedModd->relocate(outDir);

    return success;
}