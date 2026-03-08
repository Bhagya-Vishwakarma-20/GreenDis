#include <gtest/gtest.h>
#include "../src/protocol/CommandParser.h"

using namespace redis_engine::protocol;

TEST(ParserTest, ParseSetCommand) {
    auto req = CommandParser::Parse("SET mykey myval");
    EXPECT_EQ(req.command, "SET");
    ASSERT_EQ(req.args.size(), 2);
    EXPECT_EQ(req.args[0], "mykey");
    EXPECT_EQ(req.args[1], "myval");
}

TEST(ParserTest, ParseCaseInsensitive) {
    auto req = CommandParser::Parse("gEt mykey");
    EXPECT_EQ(req.command, "GET");
    ASSERT_EQ(req.args.size(), 1);
    EXPECT_EQ(req.args[0], "mykey");
}

TEST(ParserTest, ParseEmpty) {
    auto req = CommandParser::Parse("");
    EXPECT_TRUE(req.IsEmpty());
}

TEST(ParserTest, ParseExtraSpaces) {
    auto req = CommandParser::Parse("  DEL   key1  ");
    EXPECT_EQ(req.command, "DEL");
    ASSERT_EQ(req.args.size(), 1);
    EXPECT_EQ(req.args[0], "key1");
}
