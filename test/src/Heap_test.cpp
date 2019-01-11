//
//  Heap_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2018/12/24.
//

#include "gtest/gtest.h"
#include "sim_ds/Heap.hpp"

using sim_ds::MinHeap;
using sim_ds::MaxHeap;

TEST(MinHeapTest, convert) {
    std::vector<int> vec = {3,6,8,4,9,7,1,5,2,0};
    MinHeap heap(vec);
    for (int i = 0; i < vec.size(); i++) {
        EXPECT_EQ(heap.front(), i);
        heap.pop();
    }
}

TEST(MinHeapTest, push) {
    std::vector<int> vec = {3,6,8,4,9,7,1,5,2,0};
    MinHeap heap;
    for (auto v : vec) {
        heap.push(v);
    }
    for (int i = 0; i < vec.size(); i++) {
        EXPECT_EQ(heap.front(), i);
        heap.pop();
    }
}

TEST(MinHeapTest, iterator) {
    std::vector<int> vec = {3,6,8,4,9,7,1,5,2,0};
    MinHeap heap(vec.begin(), vec.end());
    for (int i = 0; i < vec.size(); i++) {
        EXPECT_EQ(heap.front(), i);
        heap.pop();
    }
}


TEST(MaxHeapTest, convert) {
    std::vector<int> vec = {3,6,8,4,9,7,1,5,2,0};
    MaxHeap heap(vec);
    for (int i = 0; i < vec.size(); i++) {
        EXPECT_EQ(heap.front(), vec.size() - 1 - i);
        heap.pop();
    }
}

TEST(MaxHeapTest, push) {
    std::vector<int> vec = {3,6,8,4,9,7,1,5,2,0};
    MaxHeap heap;
    for (auto v : vec) {
        heap.push(v);
    }
    for (int i = 0; i < vec.size(); i++) {
        EXPECT_EQ(heap.front(), vec.size() - 1 - i);
        heap.pop();
    }
}

TEST(MaxHeapTest, iterator) {
    std::vector<int> vec = {3,6,8,4,9,7,1,5,2,0};
    MaxHeap heap(vec.begin(), vec.end());
    for (int i = 0; i < vec.size(); i++) {
        EXPECT_EQ(heap.front(), vec.size() - 1 - i);
        heap.pop();
    }
}
