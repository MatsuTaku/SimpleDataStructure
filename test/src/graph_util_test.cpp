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

TEST(GraphUtilTest, TrieDfs) {
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
    
    std::vector<uint8_t> dfs_label;
    trie.dfs([&](auto node, auto depth) {
        node.for_each_edge([&](auto c, auto e) {
            dfs_label.push_back(c);
        });
    });
    
    std::string cans = "abb#c##abc##";
    std::vector<uint8_t> ans;
    for (size_t i = 0; i < 12; i++) {
        auto c = cans[i];
        if (c == '#')
            c = 0;
        ans.push_back(c);
    }
    EXPECT_EQ(dfs_label, ans);
}

TEST(GraphUtilTest, TrieBfs) {
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
    
    std::vector<uint8_t> bfs_label;
    trie.bfs([&](auto node, auto depth) {
        node.for_each_edge([&](auto c, auto e) {
            bfs_label.push_back(c);
        });
    });
    std::string cans = "abb#ab#cc###";
    std::vector<uint8_t> ans;
    for (size_t i = 0; i < 12; i++) {
        auto c = cans[i];
        if (c == '#')
            c = 0;
        ans.push_back(c);
    }
    EXPECT_EQ(bfs_label, ans);
}
