#include "base64.h"

#include <gtest/gtest.h>
#include <string>

TEST(Basic, Base64Test)
{
    std::string output;
    EXPECT_EQ(0, encode64("Hello, World!", &output));
    EXPECT_EQ("SGVsbG8sIFdvcmxkIQ==", output);
    EXPECT_EQ(0, decode64("SGVsbG8sIFdvcmxkIQ==", &output));
    EXPECT_EQ("Hello, World!", output);
}