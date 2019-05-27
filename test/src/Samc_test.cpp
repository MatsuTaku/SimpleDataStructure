//
//  Samc_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/05/25.
//

#include "gtest/gtest.h"
#include "sim_ds/Samc.hpp"

using namespace sim_ds;

TEST(SamcTest, Sample) {
    std::vector<std::string> set = {
        "ab",
        "abc",
        "b",
        "bac",
        "bb"
    };
    graph_util::Trie<int> trie;
    for (auto& s : set) {
        trie.insert(s, 1);
    }
    Samc<uint32_t> samc(trie);
    for (auto& s : set) {
        EXPECT_TRUE(samc.accept(s));
    }
}

TEST(SamcTest, SampleLookup) {
    std::vector<std::string> set = {
        "ab",
        "abc",
        "b",
        "bac",
        "bb"
    };
    graph_util::Trie<int> trie;
    for (auto& s : set) {
        trie.insert(s, 1);
    }
    SamcDict<uint32_t> samc(trie);
    
    std::set<size_t> ids;
    for (size_t i = 0; i < set.size(); i++) {
        ids.insert(samc.lookup(set[i]));
    }
    EXPECT_EQ(ids.size(), set.size());
}

TEST(SamcTest, SampleAccess) {
    std::vector<std::string> set = {
        "ab",
        "abc",
        "b",
        "bac",
        "bb"
    };
    graph_util::Trie<int> trie;
    for (auto& s : set) {
        trie.insert(s, 1);
    }
    SamcDict<uint32_t> samc(trie);
    
    std::vector<std::string> gets;
    for (size_t i = 0; i < set.size(); i++) {
        gets.push_back(samc.access(i));
    }
    sort(gets.begin(), gets.end());
    for (size_t i = 0; i < gets.size(); i++) {
        EXPECT_EQ(gets[i], set[i]);
    }
}
