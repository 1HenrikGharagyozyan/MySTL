#include <gtest/gtest.h>
#include "../include/mystl/string.hpp"

TEST(StringTest, SSOToHeapTransition) 
{
    mystl::String s;

    for(int i = 0; i < 15; ++i) 
        s.push_back('a');
    EXPECT_EQ(s.size(), 15);
    EXPECT_LE(s.capacity(), 15);

    s.push_back('b');
    EXPECT_EQ(s.size(), 16);
    EXPECT_GT(s.capacity(), 15);
    EXPECT_EQ(s[15], 'b');
}

TEST(StringTest, IteratorAndRangeBasedFor) 
{
    mystl::String s("Hello");
    std::string check;
    for(auto c : s) 
        check += c;

    EXPECT_EQ(check, "Hello");
}

TEST(StringTest, MoveSemantics) 
{
    mystl::String s1("Longer string to force heap allocation");
    mystl::String s2 = std::move(s1);
    
    EXPECT_EQ(s2, "Longer string to force heap allocation");
    EXPECT_EQ(s1.size(), 0); 
}