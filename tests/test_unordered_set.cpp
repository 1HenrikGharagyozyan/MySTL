#include <gtest/gtest.h>
#include <string>
#include "mystl/unordered_set.hpp"

using namespace mystl;

TEST(UnorderedSetTest, DefaultConstruction) 
{
    UnorderedSet<int> set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
}

TEST(UnorderedSetTest, UniqueInsertion) 
{
    UnorderedSet<int> set;
    
    auto res1 = set.insert(10);
    EXPECT_TRUE(res1.second);
    
    auto res2 = set.insert(20);
    EXPECT_TRUE(res2.second);
    
    // We are trying to insert a duplicate
    auto res3 = set.insert(10);
    EXPECT_FALSE(res3.second); // Insertion should be rejected
    
    EXPECT_EQ(set.size(), 2);
}

TEST(UnorderedSetTest, ContainsAndFind) 
{
    UnorderedSet<std::string> set;
    set.insert("Engine");
    set.insert("Graphics");

    EXPECT_TRUE(set.contains("Engine"));
    EXPECT_FALSE(set.contains("Physics"));

    auto it = set.find("Graphics");
    ASSERT_NE(it, set.end());
    EXPECT_EQ(*it, "Graphics");
}

TEST(UnorderedSetTest, EraseAndClear) 
{
    UnorderedSet<int> set;
    set.insert(1);
    set.insert(2);
    set.insert(3);

    EXPECT_EQ(set.erase(2), 1);
    EXPECT_FALSE(set.contains(2));
    EXPECT_EQ(set.size(), 2);

    set.clear();
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
}

TEST(UnorderedSetTest, RehashBehavior) 
{
    UnorderedSet<int> set(2); // We start with a small bucket_count
    
    for (int i = 0; i < 20; ++i) 
    {
        set.insert(i);
    }

    // We verify that elements are not lost during rehashing
    EXPECT_EQ(set.size(), 20);
    for (int i = 0; i < 20; ++i) 
    {
        EXPECT_TRUE(set.contains(i));
    }
}