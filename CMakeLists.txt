cmake_minimum_required(VERSION 3.10)
project(memory-replay VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -Wall -Og")
set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} -march=native -O3")

set(METADATA_SOURCES memory-replay/metadata/Modd.cxx memory-replay/metadata/Modd.hxx)

add_library(metadata STATIC ${METADATA_SOURCES})

add_executable(memory-replay memory-replay/main.cxx)
target_link_libraries(memory-replay PRIVATE metadata)