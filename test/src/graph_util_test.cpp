//
//  graph_util_test.cpp
//
//  Created by 松本拓真 on 2019/05/24.
//

#include "gtest/gtest.h"
#include "sim_ds/graph_util.hpp"

using namespace sim_ds;

TEST(GraphUtilTest, Trie) {
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
    for (auto& s : set) {
        EXPECT_TRUE(trie.traverse(s) != nullptr);
    }
}
