//
//  DACs_test.cpp
//  DistributedAC
//
//  Created by 松本拓真 on 2018/05/14.
//

#include "sim_ds/DACs.hpp"
#include "gtest/gtest.h"

#include <random>

TEST(DACsTest, ConvertVector) {
    const auto size = 0xffff;
    std::vector<size_t> src(size);
    std::random_device rnd;
    for (auto i = 0; i < size; i++) {
        src[i] = (1U << (rnd() % 32)) - 1;
    }
    sim_ds::DACs dac(src);
    for (auto i = 0; i < size; i++)
        EXPECT_EQ(src[i], dac[i]);
}
