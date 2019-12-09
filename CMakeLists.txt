cmake_minimum_required(VERSION 3.10)
project(memory-replay VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -Wall -Og")
set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} -march=native -O3")

set(METADATA_SOURCES
    memory-replay/metadata/Modd.cxx memory-replay/metadata/Modd.hxx
    memory-replay/metadata/VT.cxx memory-replay/metadata/VT.hxx
    memory-replay/metadata/Video.cxx memory-replay/metadata/Video.hxx)

find_package(SQLite3 REQUIRED)

add_library(metadata STATIC ${METADATA_SOURCES})
target_link_libraries(metadata SQLite::SQLite3)
target_include_directories(metadata SYSTEM PRIVATE ${SQLite3_LIBRARIES})

add_executable(memory-replay memory-replay/main.cxx)
target_link_libraries(memory-replay PRIVATE metadata)