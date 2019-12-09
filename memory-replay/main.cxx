#include <filesystem>
#include <string>
#include <iostream>

#include "metadata/Modd.hxx"

const static std::filesystem::path testModd("/home/bdavidson/Videos/Home_Videos/1-26-2010/20100116110730.modd");

int main() {
    Modd modd(testModd);

    return 0;
}