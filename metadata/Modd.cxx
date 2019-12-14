#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include "Modd.hxx"

using namespace memory_replay;

Modd::Modd(const fs::path& moddFilePath) {
    bool readArray = false;

    // Get the name of the modd file
    this->m_name = moddFilePath.filename();
    this->m_location = moddFilePath;

    // Read the file in.
    std::ifstream moddFile(moddFilePath.string(), std::ios::ate | std::ios::binary);
    if (!moddFile.is_open()) {
        throw std::runtime_error("Failed to open target file.");
    }

    int numChars = moddFile.tellg();
    moddFile.seekg(moddFile.beg);

    std::vector<char> readBuf;
    readBuf.resize(numChars + 1);
    readBuf[numChars] = '\0';
    moddFile.read(readBuf.data(), numChars);
    
    moddFile.close();

    std::string notBuf = vecToString(readBuf);
    std::stringstream moddTxt;
    moddTxt << cleanText(notBuf);

    std::string line;

    while (std::getline(moddTxt, line)) {
        bool readSuccess = moddTxt.rdstate() == std::ios_base::goodbit;
        if (!(readSuccess)) {
            break;
        }

        if (!readArray) {
            std::size_t commaLoc = line.find(",");
            std::string key = line.substr(0, commaLoc);
            std::string value = line.substr(commaLoc+1);

            if (key == "CheckCode") {
                this->m_checkCode = std::stoul(value, nullptr, 16);
            } else if (key == "DateTimeOriginal") {
                this->m_dateTimeOriginal = std::stof(value, nullptr);
                this->setActualTime(TimeZone::CST);
            } else if (key == "Duration") {
                this->m_duration = std::stof(value, nullptr);
            } else if (key == "FileSize") {
                this->m_fileSize = std::stoull(value, nullptr, 10);
            } else if (key == "VTList") {
                if (value == "[") {
                    readArray = true;
                }
            } else {
                // I can't track down why this is being funny.
                // std::cout << "\nWeird file: " << moddFilePath.string() << std::endl;
                // std::cout << "Parsing \'" << key << "\' not currently supported." << std::endl;
            }
        } else {
            if (line.find("]") != std::string::npos) {
                readArray = false;
                continue;
            }
            this->m_vtList.push_back(new VT(line));
        }
    }
}

Modd::~Modd() {
    for (auto& vt : this->m_vtList) {
        delete vt;
    }
}

/**
 * Converts a char vector to a string.
 * @param txt char vector
 * @return string object made from txt
 */
std::string Modd::vecToString(std::vector<char> txt) {
    std::string result;

    for (const auto& ltr : txt) {
        result += ltr;
        if (ltr == '\0') break;
    }

    return result;
}

/**
 * Removes all superfluous text from the provided string.
 * @param moddText raw text from .modd file.
 * @param size number of characters in raw text.
 * @return processed string.
 */
std::string Modd::cleanText(char* moddText, std::size_t size) {
    std::string result;
    result.copy(moddText, size);
    return cleanText(result);
}

/**
 * Removes all superfluous text from the provided string.
 * @param moddText raw text from .modd file.
 * @return processed string.
 */
std::string Modd::cleanText(std::string& moddText) {
    // Start by removing the unneeded lines of the file
    auto result = boost::algorithm::replace_first_copy(moddText, XML_HEADER, "");
    boost::algorithm::replace_first(result, DATA_HEADER, "");
    boost::algorithm::replace_first(result, DATA_FOOTER, "");

    // Clean up tags
    boost::algorithm::replace_all(result, "<key>", "");
    boost::algorithm::replace_all(result, "</key>", ",");
    boost::algorithm::replace_all(result, "<string>", "");
    boost::algorithm::replace_all(result, "</string>", "\n");
    boost::algorithm::replace_all(result, "<real>", "");
    boost::algorithm::replace_all(result, "</real>", "\n");
    boost::algorithm::replace_all(result, "<integer>", "");
    boost::algorithm::replace_all(result, "</integer>", "\n");
    boost::algorithm::replace_all(result, "<array>", "[\n");
    boost::algorithm::replace_all(result, "</array>", "]");

    boost::algorithm::trim_left(result);
    boost::algorithm::trim_right(result);

    return result;
}

std::string Modd::replaceFirst(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, toReplace.length(), replaceWith);
}

std::string Modd::replaceAll(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::string result;
    std::string old;
    do {
        old = result;
        result = replaceFirst(s, toReplace, replaceWith);
        s = result;
    } while (result != old);

    return result;
}

bool Modd::relocate(const fs::path& outDir) {
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

    return success;
}

void Modd::setActualTime(const TimeZone& tz) {
    uint64_t originalSecs = static_cast<uint64_t>(this->m_dateTimeOriginal * 86400);

    int tzOffset = static_cast<int>(tz) * 3600;

    this->m_dateTimeActual = (originalSecs - UNIX_MINUS_COM_EPOCH) + tzOffset;
}