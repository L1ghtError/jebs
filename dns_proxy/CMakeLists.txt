cmake_minimum_required (VERSION 3.5)
project(dns_proxy)
# includes
include_directories("include")
# sources
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*")
add_executable(dns_proxy ${SOURCES})
add_executable(test_dump "./test/dump.c")
