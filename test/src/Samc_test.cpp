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
