#include <gtest/gtest.h>
#include <stdexcept>
#include "mystl/deque.hpp"
#include "mystl/string.hpp"
#include "mystl/utility.hpp" // Если у тебя там реализован mystl::move

// Тестовая структура для строгой проверки утечек памяти
struct Tracker 
{
    static int instances;
    int id;

    Tracker(int i = 0) : id(i) { ++instances; }
    Tracker(const Tracker& other) : id(other.id) { ++instances; }
    ~Tracker() { --instances; }
    
    bool operator==(const Tracker& other) const { return id == other.id; }
};
int Tracker::instances = 0;

TEST(DequeTest, DefaultConstruction) 
{
    mystl::Deque<int> d;
    EXPECT_TRUE(d.empty());
    EXPECT_EQ(d.size(), 0);
}

TEST(DequeTest, PushAndPop) 
{
    mystl::Deque<int> d;
    
    // Проверка push_back и аллокации новых блоков
    for (int i = 0; i < 1000; ++i) 
    {
        d.push_back(i);
    }
    EXPECT_EQ(d.size(), 1000);
    EXPECT_EQ(d.front(), 0);
    EXPECT_EQ(d.back(), 999);

    // Проверка push_front и аллокации блоков в начале
    for (int i = 1; i <= 500; ++i) 
    {
        d.push_front(-i);
    }
    EXPECT_EQ(d.size(), 1500);
    EXPECT_EQ(d.front(), -500);

    // Проверка pop_front
    for (int i = 0; i < 500; ++i) 
    {
        d.pop_front();
    }
    EXPECT_EQ(d.front(), 0);

    // Проверка pop_back
    for (int i = 0; i < 500; ++i) 
    {
        d.pop_back();
    }
    EXPECT_EQ(d.size(), 500);
    EXPECT_EQ(d.back(), 499);
}

TEST(DequeTest, IteratorsAndElementAccess) 
{
    mystl::Deque<int> d = {10, 20, 30, 40, 50};
    
    int sum = 0;
    for (int val : d) 
    {
        sum += val;
    }
    EXPECT_EQ(sum, 150);

    // Арифметика итераторов O(1)
    auto it = d.begin();
    EXPECT_EQ(*(it + 2), 30);
    EXPECT_EQ(*(d.end() - 1), 50);
    
    // Прямой доступ
    EXPECT_EQ(d[1], 20);
    EXPECT_EQ(d.at(3), 40);
    EXPECT_THROW(d.at(10), std::out_of_range);
}

TEST(DequeTest, MemoryManagement) 
{
    Tracker::instances = 0; // Сбрасываем счетчик перед тестом
    {
        mystl::Deque<Tracker> d;
        for (int i = 0; i < 1000; ++i) 
        {
            d.emplace_back(i);
        }
        EXPECT_EQ(Tracker::instances, 1000);
        
        d.pop_front();
        d.pop_back();
        EXPECT_EQ(Tracker::instances, 998); // Должно уничтожиться 2 объекта
        
        d.clear();
        EXPECT_EQ(Tracker::instances, 0); // Память полностью очищена
    }
    EXPECT_EQ(Tracker::instances, 0); // Проверка после выхода из scope
}

TEST(DequeTest, CopyAndMoveSemantics) 
{
    mystl::Deque<int> d1 = {1, 2, 3};
    mystl::Deque<int> d2 = d1; // Copy constructor

    EXPECT_EQ(d2.size(), 3);
    EXPECT_EQ(d2[0], 1);
    
    mystl::Deque<int> d3 = std::move(d1); // Move constructor
    EXPECT_EQ(d3.size(), 3);
    EXPECT_EQ(d3[2], 3);
    
    EXPECT_EQ(d1.size(), 0); // Оригинал должен стать пустым после move
    EXPECT_TRUE(d1.empty());
}