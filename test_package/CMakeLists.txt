cmake_minimum_required(VERSION 3.5)
project(libtar-test-package)

set(CMAKE_VERBOSE_MAKEFILE ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} CONAN_PKG::libtar)
if(DEFINED CONAN_LIBS_ZLIB)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC -DHAVE_LIBZ)
endif()

enable_testing()
add_test(NAME libtar-test
         COMMAND ${CMAKE_PROJECT_NAME}
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)