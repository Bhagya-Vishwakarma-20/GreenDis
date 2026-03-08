#include <gtest/gtest.h>
#include "../src/config/ConfigLoader.h"
#include <fstream>
#include <cstdio>

using namespace redis_engine::config;

TEST(ConfigLoaderTest, LoadDefaultWhenFileMissing) {
    auto config = ConfigLoader::Load("non_existent_config.json");
    EXPECT_EQ(config.port, 6379);
    EXPECT_EQ(config.thread_pool_size, 4);
}

TEST(ConfigLoaderTest, LoadValidConfig) {
    std::string test_file = "test_config.json";
    std::ofstream out(test_file);
    out << "{\"port\": 8080, \"thread_pool_size\": 8}";
    out.close();

    auto config = ConfigLoader::Load(test_file);
    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.thread_pool_size, 8);
    EXPECT_EQ(config.max_memory, 1024 * 1024 * 1024); // default

    std::remove(test_file.c_str());
}
