//
//  SACs_test.cpp
//
//  Created by 松本拓真 on 2018/05/14.
//

#include "SACs.hpp"
#include "gtest/gtest.h"

#include <random>

TEST(SACsTest, OneElem) {
    sim_ds::SACs8 sac;
    sac.setValue(0, 0xfff3);
    sac.build();
    EXPECT_EQ(0xfff3, sac[0]);
}

TEST(SACsTest, ConvertVector) {
    const auto size = 0xffff;
    std::vector<size_t> src(size);
    std::random_device rnd;
    for (auto i = 0; i < size; i++) {
        auto r1 = rnd() % 64 + 1;
        auto r2 = rnd() % r1 + 1;
        src[i] = (1U << r2) - 1;
    }
    sim_ds::SACs8 sac(src);
    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(src[i], sac[i]);
    }
}

TEST(SACsWVTest, ConvertVector) {
    const auto size = 0xffff;
    std::vector<size_t> src(size);
    std::random_device rnd;
    for (auto i = 0; i < size; i++) {
        auto r1 = rnd() % 64 + 1;
        auto r2 = rnd() % r1 + 1;
        src[i] = (1U << r2) - 1;
    }
    sim_ds::SACsWV sac(src);
    for (auto i = 0; i < size; i++) {
        EXPECT_EQ(src[i], sac[i]);
    }
    
}
