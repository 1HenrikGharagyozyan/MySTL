#include <gtest/gtest.h>
#include <vector>
#include <functional>
#include "mystl/priority_queue.hpp"

using namespace mystl;

TEST(PriorityQueueTest, DefaultConstruction) 
{
    PriorityQueue<int> pq;
    EXPECT_TRUE(pq.empty());
    EXPECT_EQ(pq.size(), 0);
}

TEST(PriorityQueueTest, PushAndTopMaxHeap) 
{
    PriorityQueue<int> pq;
    
    pq.push(10);
    pq.push(30);
    pq.push(20);
    pq.push(5);

    // In a max-heap, the top element is always the largest
    EXPECT_EQ(pq.size(), 4);
    EXPECT_EQ(pq.top(), 30);
}

TEST(PriorityQueueTest, PopSequence) 
{
    PriorityQueue<int> pq;
    pq.push(10);
    pq.push(30);
    pq.push(20);

    // They should come out in strictly descending order
    EXPECT_EQ(pq.top(), 30);
    pq.pop();
    
    EXPECT_EQ(pq.top(), 20);
    pq.pop();
    
    EXPECT_EQ(pq.top(), 10);
    pq.pop();

    EXPECT_TRUE(pq.empty());
}

TEST(PriorityQueueTest, MinHeapWithGreater) 
{
    // We pass std::greater to make a min-heap
    PriorityQueue<int, std::vector<int>, std::greater<int>> pq;
    
    pq.push(50);
    pq.push(10);
    pq.push(40);

    // Now the smallest element should be at the top
    EXPECT_EQ(pq.top(), 10);
    pq.pop();
    EXPECT_EQ(pq.top(), 40);
}

TEST(PriorityQueueTest, ConstructorFromRange) 
{
    std::vector<int> vec = {10, 50, 20, 30, 15};
    
    // make_heap will be invoked inside the constructor
    PriorityQueue<int> pq(vec.begin(), vec.end());
    
    EXPECT_EQ(pq.size(), 5);
    EXPECT_EQ(pq.top(), 50);
}