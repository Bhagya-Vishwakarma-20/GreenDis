cmake_minimum_required(VERSION 3.16)

project(MyCacheApp)

set(CMAKE_CXX_STANDARD 17)

# add your executable
add_executable(MyCacheApp src/main.cpp)

# add the redis engine as subdirectory
add_subdirectory(cpp-redis-engine)

# link the engine library
target_link_libraries(MyCacheApp PRIVATE redis_engine_lib)
target_include_directories(MyCacheApp PRIVATE cpp-redis-engine/src)