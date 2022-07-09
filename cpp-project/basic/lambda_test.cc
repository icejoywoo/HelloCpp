#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <future>

class LambdaTest : public testing::Test
{
public:
    LambdaTest() {};
    virtual ~LambdaTest() {};

protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(LambdaTest, BasicTest)
{
    {
        int x = 5;
        auto foo = [r = x + 1]{return r;};
        ASSERT_EQ(6, foo());
    }

    {
        std::string x = "hello c++ ";
        auto foo = [x = std::move(x)]{return x + "world";};
        ASSERT_EQ("hello c++ world", foo());
    }

    class Work
    {
    private:
        int value;
    public:
        Work() : value(42) {}
        std::future<int> spawn()
        {

//            return std::async([*this]() -> int {return value;});
            return std::async([=, tmp = *this]() -> int {return tmp.value;});
//            return std::async([=]() -> int {return value;}); // undefined behavior
        }
    };

    auto foo = [] {
        Work tmp;
        return tmp.spawn();
    };

    auto f = foo();
    f.wait();
    ASSERT_EQ(42, f.get());
}