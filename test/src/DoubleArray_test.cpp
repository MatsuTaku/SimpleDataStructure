//
//  DoubleArray_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/06/18.
//

#include "gtest/gtest.h"
#include "sim_ds/DoubleArray.hpp"

using namespace sim_ds;

TEST(DoubleArrayTest, Sample) {
    std::vector<std::string> set = {
        "ab",
        "abc",
        "b",
        "bac",
        "bb"
    };
    DoubleArray<uint32_t>::input_trie trie;
    for (auto& s : set) {
        trie.insert(s, 1);
    }
    DoubleArray<uint32_t> da(trie);
    for (auto& s : set) {
        EXPECT_TRUE(da.accept(s));
    }
}

TEST(DoubleArrayTest, SampleLegacy) {
    std::vector<std::string> set = {
        "ab",
        "abc",
        "b",
        "bac",
        "bb"
    };
    DoubleArray<uint32_t>::input_trie trie;
    for (auto& s : set) {
        trie.insert(s, 1);
    }
    DoubleArray<uint32_t> da(trie);
    for (auto& s : set) {
        EXPECT_TRUE(da.accept(s));
    }
}
