//
//  sort_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2018/12/24.
//

#include "gtest/gtest.h"
#include "sim_ds/sort.hpp"

TEST(SortTest, heap) {
    size_t size = 0x10000;
    std::vector<size_t> vec(size);
    for (auto& v : vec) {
        v = rand() % 0x10000;
    }
    
    std::vector<size_t> heap_vec = vec;
    
    std::sort(vec.begin(), vec.end());
    sim_ds::HeapSort(heap_vec.begin(), heap_vec.end());
    
    for (size_t i = 0; i < size; i++) {
        EXPECT_EQ(vec[i], heap_vec[i]);
    }
}
