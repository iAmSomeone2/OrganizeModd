cmake_minimum_required(VERSION 3.10)
project(memory-replay VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -Wall -Og")
set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} -march=native -O3")

set(METADATA_SOURCES
    memory-replay/metadata/Modd.cxx memory-replay/metadata/Modd.hxx
    memory-replay/metadata/VT.cxx memory-replay/metadata/VT.hxx
    memory-replay/metadata/Video.cxx memory-replay/metadata/Video.hxx)

# Metadata static lib
add_library(metadata STATIC ${METADATA_SOURCES})
target_compile_options(metadata PRIVATE -Wall)
target_compile_features(metadata PUBLIC cxx_auto_type cxx_nullptr cxx_range_for)

# Database static lib
find_package(SQLite3 REQUIRED)
find_package(Boost 1.29.0 REQUIRED)
add_library(database STATIC memory-replay/Database.cxx memory-replay/Database.hxx)
target_link_libraries(database SQLite::SQLite3)
target_include_directories(database SYSTEM PRIVATE ${SQLite3_LIBRARIES} ${Boost_INCLUDE_DIRS})

# Output executable
add_executable(memory-replay memory-replay/main.cxx)
target_compile_options(memory-replay PRIVATE -Wall)
target_compile_features(memory-replay PUBLIC cxx_auto_type cxx_range_for)
target_link_libraries(memory-replay PRIVATE metadata database)