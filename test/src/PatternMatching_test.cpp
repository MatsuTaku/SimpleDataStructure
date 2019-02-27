//
//  PatternMatching_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/02/27.
//

#include "gtest/gtest.h"
#include "sim_ds/PatternMatching.hpp"

TEST(PatternMatching, Kmp_sample) {
    std::string text = "abcabcababcababxabcabx";
    std::string key = "abcaba";
    std::vector<size_t> answer{3, 8};
    
    auto matchies = sim_ds::PatternMatchKMP(text, key);
    EXPECT_EQ(matchies.size(), answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), answer.size()); i++) {
        EXPECT_EQ(matchies[i], answer[i]);
    }
}

TEST(PatternMatching, Bm_sample) {
    std::string text = "abcabcababcababxabcabx";
    std::string key = "abcaba";
    std::vector<size_t> answer{3, 8};
    
    auto matchies = sim_ds::PatternMatchBM(text, key);
    EXPECT_EQ(matchies.size(), answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), answer.size()); i++) {
        EXPECT_EQ(matchies[i], answer[i]);
    }
}

TEST(PatternMatching, Bom_sample) {
    std::string text = "abcabcababcababxabcabx";
    std::string key = "abcaba";
    std::vector<size_t> answer{3, 8};
    
    auto matchies = sim_ds::PatternMatchBOM(text, key);
    EXPECT_EQ(matchies.size(), answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), answer.size()); i++) {
        EXPECT_EQ(matchies[i], answer[i]);
    }
}

TEST(PatternMatching, TurboBom_sample) {
    std::string text = "abcabcababcababxabcabx";
    std::string key = "abcaba";
    std::vector<size_t> answer{3, 8};
    
    auto matchies = sim_ds::PatternMatchTurboBOM(text, key);
    EXPECT_EQ(matchies.size(), answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), answer.size()); i++) {
        EXPECT_EQ(matchies[i], answer[i]);
    }
}
