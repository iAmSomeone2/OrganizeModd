#ifndef MEMORY_REPLAY_TIME_HXX
#define MEMORY_REPLAY_TIME_HXX

#include <map>
#include <string>

using std::string;
using std::map;

namespace memory_replay {
    static const uint64_t YEAR_SECS = 31556926;
    static const uint64_t MONTH_SECS = 2629743;

    enum class Month {
        January = 0,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December,
    };

    static map<Month, const string> MONTH_STR = {
        {Month::January, "01_January"},
        {Month::February, "02_February"},
        {Month::March, "03_March"},
        {Month::April, "04_April"},
        {Month::May, "05_May"},
        {Month::June, "06_June"},
        {Month::July, "07_July"},
        {Month::August, "08_August"},
        {Month::September, "09_September"},
        {Month::October, "10_October"},
        {Month::November, "11_November"},
        {Month::December, "12_December"}
    };

    class Time {
    public:
        Time();
        explicit Time(uint64_t unixSecs);

        void set(uint64_t unixSecs);

        int         year()      const;
        Month       month()     const;
        uint64_t    unixSecs()  const;
    private:
        uint64_t m_unixSecs;
    };
};

#endif // MEMORY_REPLAY_TIME_HXX