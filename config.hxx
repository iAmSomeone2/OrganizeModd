#ifndef MEMORY_REPLAY_CONFIG_HXX
#define MEMORY_REPLAY_CONFIG_HXX

#include <map>

namespace memory_replay {
    static const char OPTS_STR[] = ":u:r:";

    enum class Option {
        Update,
        Relocate
    };
};
#endif // MEMORY_REPLAY_CONFIG_HXX