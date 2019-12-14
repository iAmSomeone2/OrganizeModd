find_package(SQLite3 REQUIRED)
find_package(Boost 1.29.0 REQUIRED)

add_library(database STATIC Database.cxx Database.hxx)
target_link_libraries(database PRIVATE SQLite::SQLite3)
target_include_directories(database SYSTEM PRIVATE ${SQLite3_LIBRARIES} ${Boos_INCLUDE_DIRS})
