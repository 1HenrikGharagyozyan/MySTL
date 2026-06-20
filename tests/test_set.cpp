#include <gtest/gtest.h>
#include <string>

#include "mystl/set.hpp"

using namespace mystl;

TEST(SetTest, DefaultConstruction) 
{
    Set<int> s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
}

TEST(SetTest, InsertAndUnique) 
{
    Set<int> s;
    auto res1 = s.insert(10);
    EXPECT_TRUE(res1.second);
    EXPECT_EQ(*res1.first, 10);

    auto res2 = s.insert(20);
    EXPECT_TRUE(res2.second);

    // Trying to insert a duplicate
    auto res3 = s.insert(10);
    EXPECT_FALSE(res3.second); // Insertion should be rejected
    EXPECT_EQ(*res3.first, 10); // Iterator should point to the existing element

    EXPECT_EQ(s.size(), 2);
}

TEST(SetTest, IterationIsSorted) 
{
    Set<int> s = {50, 20, 40, 10, 30};
    
    // In a red-black tree, elements are always sorted
    int expected = 10;
    for (int val : s) 
    {
        EXPECT_EQ(val, expected);
        expected += 10;
    }
}

TEST(SetTest, FindAndContains) 
{
    Set<std::string> s;
    s.emplace("Hydra");
    s.emplace("Engine");

    EXPECT_TRUE(s.contains("Hydra"));
    EXPECT_FALSE(s.contains("OpenGL"));

    auto it = s.find("Engine");
    ASSERT_NE(it, s.end());
    EXPECT_EQ(*it, "Engine");
}

TEST(SetTest, EraseElement) 
{
    Set<int> s = {1, 2, 3, 4, 5};
    
    EXPECT_EQ(s.erase(3), 1);
    EXPECT_EQ(s.size(), 4);
    EXPECT_FALSE(s.contains(3));

    // Deletion of non-existent element
    EXPECT_EQ(s.erase(99), 0);
}