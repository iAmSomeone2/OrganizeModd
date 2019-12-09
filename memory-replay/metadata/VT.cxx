#include "VT.hxx"

VT::VT(std::string vtText) {
    // Value 0
    std::string value;
    std::size_t splitPt = vtText.find(':');
    value = vtText.substr(0, splitPt);

    this->m_field0 = std::stoul(value, nullptr, 10);

    // Value 1
    std::size_t oldSplit = splitPt;
    splitPt = vtText.find(':', splitPt + 1);
    value = vtText.substr(oldSplit + 1, splitPt);
    this->m_field1 = std::stoul(value, nullptr, 10);

    // Value 2
    oldSplit = splitPt;
    splitPt = vtText.find(':', splitPt + 1);
    value = vtText.substr(oldSplit + 1, splitPt);
    this->m_field2 = std::stod(value, nullptr);

    // Value 3
    oldSplit = splitPt;
    splitPt = vtText.find(':', splitPt + 1);
    value = vtText.substr(oldSplit + 1, splitPt);
    this->m_field3 = std::stod(value, nullptr);

    // Value 4
    oldSplit = splitPt;
    splitPt = vtText.find(':', splitPt + 1);
    value = vtText.substr(oldSplit + 1, splitPt);
    this->m_field4 = std::stod(value, nullptr);

    // Value 5
    oldSplit = splitPt;
    splitPt = vtText.find(':', splitPt + 1);
    value = vtText.substr(oldSplit + 1, splitPt);
    this->m_field5 = std::stoul(value, nullptr, 10);
}