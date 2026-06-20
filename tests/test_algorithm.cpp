#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "mystl/algorithm.hpp"

using namespace mystl;

// ========================================================================
// NON-MODIFYING ALGORITHMS
// ========================================================================

TEST(AlgorithmTest, NonModifying) 
{
    std::vector<int> v = {1, 2, 3, 4, 5};

    // find
    auto it = mystl::find(v.begin(), v.end(), 3);
    EXPECT_EQ(*it, 3);
    
    it = mystl::find(v.begin(), v.end(), 10);
    EXPECT_EQ(it, v.end());

    // count
    std::vector<int> v_count = {1, 2, 2, 3, 2};
    EXPECT_EQ(mystl::count(v_count.begin(), v_count.end(), 2), 3);

    // all_of / any_of / none_of
    EXPECT_TRUE(mystl::all_of(v.begin(), v.end(), [](int i) { return i > 0; }));
    EXPECT_TRUE(mystl::any_of(v.begin(), v.end(), [](int i) { return i == 4; }));
    EXPECT_TRUE(mystl::none_of(v.begin(), v.end(), [](int i) { return i < 0; }));
}

// ========================================================================
// COMPARISONS & MIN/MAX
// ========================================================================

TEST(AlgorithmTest, Comparisons) 
{
    EXPECT_EQ(mystl::min(5, 10), 5);
    EXPECT_EQ(mystl::max(5, 10), 10);
    EXPECT_EQ(mystl::clamp(5, 10, 20), 10);
    EXPECT_EQ(mystl::clamp(25, 10, 20), 20);
    EXPECT_EQ(mystl::clamp(15, 10, 20), 15);

    std::vector<int> v1 = {1, 2, 3};
    std::vector<int> v2 = {1, 2, 3};
    std::vector<int> v3 = {1, 2, 4};

    EXPECT_TRUE(mystl::equal(v1.begin(), v1.end(), v2.begin()));
    EXPECT_TRUE(mystl::lexicographical_compare(v1.begin(), v1.end(), v3.begin(), v3.end()));
}

// ========================================================================
// MODIFYING ALGORITHMS (TAG DISPATCHING TESTS)
// ========================================================================

TEST(AlgorithmTest, Modifying) 
{
    // copy (standard iterators)
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst(3);
    mystl::copy(src.begin(), src.end(), dst.begin());
    
    EXPECT_EQ(dst[0], 1);
    EXPECT_EQ(dst[2], 3);

    // copy with memmove optimization (raw pointers to trivial types)
    int arr_src[] = {10, 20, 30};
    int arr_dst[3];
    mystl::copy(arr_src, arr_src + 3, arr_dst);
    
    EXPECT_EQ(arr_dst[1], 20);

    // fill
    std::vector<int> v_fill(5);
    mystl::fill(v_fill.begin(), v_fill.end(), 7);
    
    EXPECT_EQ(v_fill[0], 7);
    EXPECT_EQ(v_fill[4], 7);
}

// ========================================================================
// HEAP OPERATIONS
// ========================================================================

TEST(AlgorithmTest, HeapOperations) 
{
    std::vector<int> v = {3, 1, 4, 1, 5, 9};

    // make_heap
    mystl::make_heap(v.begin(), v.end(), mystl::less<int>());
    EXPECT_EQ(v.front(), 9); // Max-heap: 9 should be at the top

    // pop_heap
    mystl::pop_heap(v.begin(), v.end(), mystl::less<int>());
    v.pop_back(); // Remove 9 from the end of the vector
    EXPECT_EQ(v.front(), 5); // Now 5 is at the top

    // push_heap
    v.push_back(10);
    mystl::push_heap(v.begin(), v.end(), mystl::less<int>());
    EXPECT_EQ(v.front(), 10); // 10 becomes the new top
}