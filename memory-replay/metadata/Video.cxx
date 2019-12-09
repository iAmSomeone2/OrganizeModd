#include "Video.hxx"

fs::path Video::determineLocation() {
    auto moddPath = this->m_linkedModd->getPath();
    auto fileName = moddPath.filename();
    auto dir = moddPath.parent_path();
}