/*
 * HelloCppTest.cpp
 *
 *  Created on: 2013年10月12日
 *      Author: icejoywoo
 */

#include "hello_cpp.h"
#include "gtest/gtest.h"

TEST(FooTest, HandleNoneZeroInput)
{
    EXPECT_EQ(2, Foo(4, 10));
    EXPECT_EQ(6, Foo(30, 18));
}
