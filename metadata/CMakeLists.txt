find_package(OpenSSL REQUIRED)
find_package(Boost 1.29.0 REQUIRED)

set(METADATA_SOURCES 
	Modd.cxx Modd.hxx
	VT.cxx VT.hxx
	Video.cxx Video.hxx
	Time.cxx Time.hxx)

add_library(metadata STATIC ${METADATA_SOURCES})
target_link_libraries(metadata PRIVATE OpenSSL::Crypto)
target_include_directories(metadata SYSTEM PRIVATE ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_compile_options(metadata PRIVATE -Wall)
target_compile_features(metadata PUBLIC cxx_auto_type cxx_nullptr cxx_range_for)
