//
//  FactorOracle_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/02/24.
//

#include "gtest/gtest.h"
#include "sim_ds/FactorOracle.hpp"

TEST(FactorOracleTest, sample) {
    sim_ds::FactorOracle fo("abbbabaaab");
    EXPECT_TRUE(fo.accept("abbb"));
    EXPECT_TRUE(fo.accept("aaab"));
    EXPECT_TRUE(fo.accept("abbbab"));
    EXPECT_TRUE(fo.accept("bab"));
    EXPECT_TRUE(fo.accept("aba"));
    EXPECT_TRUE(fo.accept("abba")); // False acceptance
    EXPECT_TRUE(fo.accept("abbbaa")); // False acceptance
}

TEST(FactorOracleTest, full_exploration_small) {
    std::string_view text("abracatabra");
    sim_ds::FactorOracle fo("abracatabra");
    for (int i = 0; i < text.size(); i++) {
        for (int j = 1; j <= text.size(); j++) {
            EXPECT_TRUE(fo.accept(text.substr(i, j - i)));
        }
    }
}

TEST(FactorOracleTest, full_exploration_large) {
    std::string text;
    const size_t SIZE = 0xFFFF;
    const std::string alphabets = "abcd";
    for (size_t i = 0; i < SIZE; i++) {
        text.push_back(alphabets[rand() % 4]);
    }
    sim_ds::FactorOracle fo(text);
    std::string_view text_view(text);
    for (int i = 0; i < SIZE; i++) {
        EXPECT_TRUE(fo.accept(text_view.substr(i)));
    }
}
