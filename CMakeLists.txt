cmake_minimum_required(VERSION 3.12)
project(hft)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

file( GLOB SOURCES_COMMON LIST_DIRECTORIES false src/*.h src/*.cpp )
file( GLOB_RECURSE SOURCES_PRODUCER LIST_DIRECTORIES false src/producer/*.h src/producer/*.cpp )
file( GLOB_RECURSE SOURCES_CONSUMER LIST_DIRECTORIES false src/consumer/*.h src/consumer/*.cpp )

find_package( OpenSSL REQUIRED )
find_package( Boost REQUIRED COMPONENTS system json )
include_directories( ${CMAKE_CURRENT_LIST_DIR}/src ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} )

add_executable( producer ${SOURCES_COMMON} ${SOURCES_PRODUCER} )
target_link_libraries( producer PUBLIC ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )

add_executable( consumer ${SOURCES_COMMON} ${SOURCES_CONSUMER} )
target_link_libraries( consumer PUBLIC ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )

source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX \\ FILES ${SOURCES_COMMON} )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/producer PREFIX \\ FILES ${SOURCES_PRODUCER} )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/consumer PREFIX \\ FILES ${SOURCES_CONSUMER} )

