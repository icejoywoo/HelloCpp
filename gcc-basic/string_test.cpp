#include <gtest/gtest.h>
#include <string>

TEST(Basic, StringTest)
{
    std::string output;
    output = "Hello, World!";
    EXPECT_STREQ("Hello, World!", output.c_str());
    EXPECT_EQ(13, output.size());
}