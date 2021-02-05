cmake_minimum_required(VERSION 3.17)
project(dedupe)

set(CMAKE_CXX_STANDARD 17)


# Base dependencies
find_package(PkgConfig REQUIRED)
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(SSL REQUIRED openssl>=1.1.0)

add_executable(dedupe main.cpp ThreadPool.cpp ThreadPool.hpp)

target_compile_features(dedupe PUBLIC
        cxx_std_17
        cxx_nullptr
        cxx_auto_type
        cxx_range_for
        )

target_include_directories(dedupe PRIVATE SYSTEM
        ${SSL_INCLUDE_DIRS}
        )

target_link_libraries(dedupe PRIVATE
        ${SSL_LIBRARIES}
        fmt::fmt
        Threads::Threads
        )
