cmake_minimum_required(VERSION 3.10)
project(memory-replay VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} -march=native -O3")

set(METADATA_SOURCES
    metadata/Modd.cxx metadata/Modd.hxx
    metadata/VT.cxx metadata/VT.hxx
    metadata/Video.cxx metadata/Video.hxx
    metadata/Time.cxx metadata/Time.hxx)

# Metadata static lib

add_subdirectory(metadata)

# Database static lib

add_subdirectory(database)

# Output executable
add_executable(memory-replay main.cxx config.hxx)
target_compile_options(memory-replay PRIVATE -Wall)
target_compile_features(memory-replay PUBLIC cxx_auto_type cxx_range_for)
target_link_libraries(memory-replay PRIVATE metadata database)
