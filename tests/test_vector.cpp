#include <gtest/gtest.h>
#include "mystl/vector.hpp"
#include <string>

TEST(VectorTest, DefaultConstruction)
{
    mystl::Vector<int> vec;
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST(VectorTest, PushBackAndCapacityGrowth)
{
    mystl::Vector<int> vec;
    vec.push_back(1);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_GE(vec.capacity(), 1);

    vec.push_back(2);
    vec.push_back(3);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[2], 3);
}

TEST(VectorTest, ComplexObjects)
{
    mystl::Vector<std::string> vec;
    vec.push_back("Hello");
    vec.push_back("World");

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], "Hello");

    vec.pop_back();
    EXPECT_EQ(vec.size(), 1);
}

TEST(VectorTest, RangeBasedForLoop)
{
    mystl::Vector<int> vec = {1, 2, 3, 4, 5};
    int sum = 0;
    for (int val : vec)
    {
        sum += val;
    }
    EXPECT_EQ(sum, 15);
}

TEST(VectorTest, MoveSemantics)
{
    mystl::Vector<int> vec1 = {1, 2, 3};
    const int *old_data_ptr = &vec1[0];

    // Move vec1 to vec2
    mystl::Vector<int> vec2 = mystl::move(vec1);

    // vec1 should become empty
    EXPECT_EQ(vec1.size(), 0);
    // vec2 should receive the data, without copying (same pointer)
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2.data(), old_data_ptr); // Use data() to get the underlying pointer
}

TEST(VectorTest, CopySemantics)
{
    mystl::Vector<int> vec1 = {10, 20};
    mystl::Vector<int> vec2 = vec1; // Copy

    EXPECT_EQ(vec2.size(), 2);
    EXPECT_EQ(vec2[0], 10);

    // Modifying the copy should not affect the original
    vec2[0] = 99;
    EXPECT_EQ(vec1[0], 10);
}

TEST(VectorTest, InsertElements)
{
    mystl::Vector<int> vec = {1, 2, 4, 5};

    // Insert 3 at index 2 (before 4)
    auto it = vec.insert(vec.begin() + 2, 3);

    EXPECT_EQ(*it, 3);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec[2], 3);
    EXPECT_EQ(vec[4], 5);
}

TEST(VectorTest, EraseElements)
{
    mystl::Vector<int> vec = {10, 20, 30, 40};

    // Erase 20 (index 1)
    auto it = vec.erase(vec.begin() + 1);

    EXPECT_EQ(*it, 30); // Should return an iterator to the next element
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[1], 30);
}