#include "Video.hxx"

fs::path Video::determineLocation() {
    auto moddPath = this->m_linkedModd->getPath();
    auto ext = moddPath.extension();
    auto dir = moddPath.parent_path();

    std::size_t extLoc = moddPath.string().find(ext);
    auto fileName = moddPath.string().substr(0, extLoc);
}