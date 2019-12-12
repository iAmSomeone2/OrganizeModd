#include <filesystem>
#include <string>
#include <iostream>

#include "metadata/Modd.hxx"
#include "metadata/Video.hxx"
#include "Database.hxx"

using namespace memory_replay;
namespace fs = std::filesystem;

const static std::filesystem::path testModd("/home/bdavidson/Videos/Home_Videos/1-26-2010/20100116110730.modd");
const static std::filesystem::path testPath("/home/bdavidson/Videos/Home_Videos/");

int main(int argc, char** argv) {
    fs::path searchDir("./");

    if (argc > 1) {
        searchDir = argv[1];
    }

    std::vector<Modd*> moddList;

    std::cout << "Searching for modd files..." << std::endl;
    for (auto& p : fs::recursive_directory_iterator(searchDir)) {
        if (p.is_regular_file()) {
            auto filePath = p.path();
            if (filePath.extension()  == ".modd") {
                moddList.push_back(new Modd(filePath));
            }
        }
    }

    Database db(fs::path("library.db"));

    // Add modds to db
    std::cout << "Updating modd files in database..." << std::endl;
    db.addEntries(moddList);

    std::cout << "Searching for video files..." << std::endl;
    std::vector<Video*> videoList;
    for (auto& modd : moddList) {
        videoList.push_back(new Video(*modd));
    }

    // Add videos to db
    std::cout << "Updating video files in database..." << std::endl;
    db.addEntries(videoList);

    // Clean up the videoList before exiting
    for (auto& video : videoList) {
        delete video;
    }

    // Clean up the moddList before exiting
    for (auto& modd : moddList) {
        delete modd;
    }

    return 0;
}