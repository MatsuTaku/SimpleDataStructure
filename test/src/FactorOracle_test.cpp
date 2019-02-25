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
    EXPECT_TRUE(fo.accept("abba"));
    EXPECT_TRUE(fo.accept("aba"));
}

TEST(FactorOracleTest, full_exploration) {
    std::string_view text("abracatabra");
    sim_ds::FactorOracle fo("abracatabra");
    for (int i = 0; i < text.size(); i++) {
        for (int j = 1; j <= text.size(); j++) {
            EXPECT_TRUE(fo.accept(text.substr(i, j - i)));
        }
    }
}
