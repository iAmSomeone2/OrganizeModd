#include <filesystem>
#include <string>
#include <iostream>

extern "C" {
#include <unistd.h>
};

#include "config.hxx"
#include "metadata/Modd.hxx"
#include "metadata/Video.hxx"
#include "database/Database.hxx"

using namespace memory_replay;
namespace fs = std::filesystem;
using std::string;

const static std::filesystem::path testModd("/home/bdavidson/Videos/Home_Videos/1-26-2010/20100116110730.modd");
const static std::filesystem::path testPath("/home/bdavidson/Videos/Home_Videos/");

int main(int argc, char** argv) {

    map<const Option, bool> enabledOpts = {
        {Option::Update, false},
        {Option::Relocate, false}
    };

    fs::path searchDir("./");
    fs::path outDir("./");

    int opt;
    while ((opt = getopt(argc, argv, OPTS_STR)) != -1) {
        // 'u' updates the DB. 'r' relocates files to the specified location
        switch (opt) {
            case 'u':
                searchDir = fs::path(optarg);
                enabledOpts[Option::Update] = true;
                break;
            case 'r':
                outDir = fs::path(optarg);
                enabledOpts[Option::Relocate] = true;
                break;
            case ':':
                std::cerr << "option needs a value" << std::endl;
                break;
            case '?':
                std::cerr << "unknown option: " << opt << std::endl;
                break;
        }
    }

    std::vector<Modd*> moddList;
    std::vector<Video*> videoList;

    if (enabledOpts[Option::Update]) {
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
        
        for (auto& modd : moddList) {
            videoList.push_back(new Video(*modd));
        }

        // Add videos to db
        std::cout << "Updating video files in database..." << std::endl;
        db.updateEntries(videoList);
    }
    

    if (enabledOpts[Option::Relocate]) {
        // Relocate a few files.
        std::cout << "Relocating misplaced videos..." << std::endl;
        for (auto& video : videoList) {
            video->relocate(outDir);
        }

        // Clean up the videoList before exiting
        for (auto& video : videoList) {
            delete video;
        }

        // Clean up the moddList before exiting
        for (auto& modd : moddList) {
            delete modd;
        }
    }
    
    std::cout << "Done!" << std::endl;

    return 0;
}