//
//  MinHeap_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2018/12/24.
//

#include "gtest/gtest.h"
#include "sim_ds/MinHeap.hpp"

using sim_ds::MinHeap;

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
