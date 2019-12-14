#include "Time.hxx"

using namespace memory_replay;

Time::Time() {
    this->m_unixSecs = 0;
}

Time::Time(uint64_t unixSecs) {
    this->m_unixSecs = unixSecs;
}

void Time::set(uint64_t unixSecs) {
    this->m_unixSecs = unixSecs;
}

int Time::year() const {
    return (this->m_unixSecs / YEAR_SECS) + 1970;
}

Month Time::month() const {
    uint monthNum = (this->m_unixSecs % YEAR_SECS) / MONTH_SECS;
    return Month(monthNum);
}

uint64_t Time::unixSecs() const {
    return this->m_unixSecs;
}