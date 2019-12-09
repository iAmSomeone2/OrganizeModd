#include <filesystem>
#include <string>
#include <iostream>

#include "metadata/Modd.hxx"

const static std::filesystem::path testModd("/home/bdavidson/Videos/Home_Videos/1-26-2010/20100116110730.modd");
const static std::filesystem::path testPath("/home/bdavidson/Videos/Home_Videos/");

int main() {
    std::vector<Modd*> moddList;

    for (auto& p : std::filesystem::recursive_directory_iterator(testPath)) {
        if (p.is_regular_file()) {
            auto filePath = p.path();
            if (filePath.extension()  == ".modd") {
                moddList.push_back(new Modd(filePath));
            }
        }
    }

    // Clean up the moddList before exiting
    for (auto& modd : moddList) {
        delete modd;
    }

    return 0;
}