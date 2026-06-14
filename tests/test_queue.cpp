#include <gtest/gtest.h>
#include <string>

#include "mystl/queue.hpp"

using namespace mystl;

TEST(QueueTest, DefaultConstruction) 
{
    Queue<int> q;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TEST(QueueTest, PushAndPop) 
{
    Queue<int> q;
    q.push(10);
    q.push(20);
    q.push(30);

    EXPECT_EQ(q.size(), 3);
    EXPECT_EQ(q.front(), 10);
    EXPECT_EQ(q.back(), 30);

    q.pop();
    EXPECT_EQ(q.front(), 20);
    EXPECT_EQ(q.size(), 2);
}

TEST(QueueTest, EmplaceAndMove) 
{
    Queue<std::string> q;
    q.emplace("Hydra"); 
    
    std::string word = "Engine";
    q.push(std::move(word)); // for test

    EXPECT_EQ(q.front(), "Hydra");
    EXPECT_EQ(q.back(), "Engine");
    EXPECT_EQ(q.size(), 2);
}

TEST(QueueTest, Swap) 
{
    Queue<int> q1;
    q1.push(1);
    q1.push(2);

    Queue<int> q2;
    q2.push(99);

    q1.swap(q2);

    EXPECT_EQ(q1.size(), 1);
    EXPECT_EQ(q1.front(), 99);

    EXPECT_EQ(q2.size(), 2);
    EXPECT_EQ(q2.front(), 1);
}