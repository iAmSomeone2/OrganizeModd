#include <iostream>
#include <fstream>
#include <sstream>

#include "Modd.hxx"

Modd::Modd(const std::filesystem::path& moddFilePath) {
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
            } else if (key == "Filesize") {
                this->m_fileSize = std::stoull(value, nullptr, 10);
            } else if (key == "VTList") {
                if (value == "[") {
                    readArray = true;
                }
            } else {
                std::cout << "Parsing '" << key << "' not currently supported." << std::endl;
            }
        } else {
            if (line.find("]") != std::string::npos) {
                readArray = false;
                continue;
            }
            std::cout << "VT entry seen and ignored." << std::endl;
        }
    }
}

std::string Modd::vecToString(std::vector<char> txt) {
    std::string result;

    for (const auto& ltr : txt) {
        result += ltr;
        if (ltr == '\0') break;
    }

    return result;
}

std::string Modd::cleanText(char* moddText, std::size_t size) {
    std::string result;
    result.copy(moddText, size);
    return cleanText(result);
}

std::string Modd::cleanText(std::string& moddText) {
    std::string result;

    // Start by removing the unneeded lines of the file
    result = replaceFirst(moddText, XML_HEADER, "");
    result = replaceFirst(result, DATA_HEADER, "");
    result = replaceFirst(result, DATA_FOOTER, "");

    // Clean up tags
    result = replaceAll(result, "<key>", "");
    result = replaceAll(result, "</key>", ",");
    result = replaceAll(result, "<string>", "");
    result = replaceAll(result, "</string>", "\n");
    result = replaceAll(result, "<real>", "");
    result = replaceAll(result, "</real>", "\n");
    result = replaceAll(result, "<integer>", "");
    result = replaceAll(result, "</integer>", "\n");
    result = replaceAll(result, "<array>", "[\n");
    result = replaceAll(result, "</array>", "]");

    return result;
}

std::string replaceFirst(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, toReplace.length(), replaceWith);
}

std::string replaceAll(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::string result;
    std::string old;
    do {
        old = result;
        result = replaceFirst(s, toReplace, replaceWith);
        s = result;
    } while (result != old);

    return result;
}

void Modd::setActualTime(const TimeZone& tz) {
    uint64_t originalSecs = static_cast<uint64_t>(this->m_dateTimeOriginal * 86400);

    int tzOffset = static_cast<int>(tz) * 3600;

    this->m_dateTimeActual = (originalSecs - UNIX_MINUS_COM_EPOCH) + tzOffset;
}