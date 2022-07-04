#include <gtest/gtest.h>
#include <iostream>
#include <string>

class StringTest : public testing::Test
{
public:
    StringTest() {};
    virtual ~StringTest() {};

protected:
    virtual void SetUp()
    {
        std::cout << "setup" << std::endl;
    }

    virtual void TearDown()
    {
        std::cout << "teardown" << std::endl;
    }
};

TEST_F(StringTest, BasicTest)
{
    std::string output;
    output = "Hello, World!";
    EXPECT_STREQ("Hello, World!", output.c_str());
    EXPECT_EQ(13, output.size());
}