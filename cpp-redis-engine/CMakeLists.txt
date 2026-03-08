cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(cpp-redis-engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# Thread support
find_package(Threads REQUIRED)

# FetchContent for dependencies
include(FetchContent)

# nlohmann/json
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Google Benchmark
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/config
    ${PROJECT_SOURCE_DIR}/src/concurrency
    ${PROJECT_SOURCE_DIR}/src/controllers
    ${PROJECT_SOURCE_DIR}/src/eviction
    ${PROJECT_SOURCE_DIR}/src/persistence
    ${PROJECT_SOURCE_DIR}/src/protocol
    ${PROJECT_SOURCE_DIR}/src/server
    ${PROJECT_SOURCE_DIR}/src/services
    ${PROJECT_SOURCE_DIR}/src/storage
    ${PROJECT_SOURCE_DIR}/src/utils
)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
# Exclude main for library target or test executables
list(FILTER SOURCES EXCLUDE REGEX "src/main.cpp")

# Core Engine Library
add_library(redis_engine_lib ${SOURCES})
target_link_libraries(redis_engine_lib PUBLIC
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)
# Note: For Boost.Asio, we'll try to use raw sockets or standalone Asio if Boost is not available. 
# Wait, let's just use raw sockets on Windows/Linux or non-boost asio.
# Actually, nlohmann and spdlog are good.
# Let's add target compile definitions to use standalone ASIO if we want, or we can just use raw winsock/sys sockets. The prompt allowed "Boost.Asio or raw sockets". Let's use raw sockets for simplicity and no external Boost dependency to build fast, or download standalone asio.
# Using standalone Asio header-only for networking is cleaner than raw sockets.
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-28-0
)
FetchContent_MakeAvailable(asio)
target_include_directories(redis_engine_lib PUBLIC ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(redis_engine_lib PUBLIC ASIO_STANDALONE)
if(WIN32)
    target_compile_definitions(redis_engine_lib PUBLIC _WIN32_WINNT=0x0601) # Windows 7+
endif()


# Main Executable
add_executable(cpp-redis-engine src/main.cpp)
target_link_libraries(cpp-redis-engine PRIVATE redis_engine_lib)

# Tests
enable_testing()
add_executable(unit_tests 
    tests/storage_tests.cpp
    # Add other test files here
)
target_link_libraries(unit_tests PRIVATE 
    redis_engine_lib
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(unit_tests)

# Benchmarks
add_executable(kv_benchmark benchmarks/kv_benchmark.cpp)
target_link_libraries(kv_benchmark PRIVATE 
    redis_engine_lib
    benchmark::benchmark_main
)
