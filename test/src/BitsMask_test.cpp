//
//  BitsMask_test.cpp
//  sim_ds
//
//  Created by 松本拓真 on 2018/11/22.
//

#include "gtest/gtest.h"
#include "sim_ds/bit_tools.hpp"

using namespace sim_ds::bit_tools;

TEST(BitsMaskTest, each) {
    EXPECT_EQ(maskOfBits(0), 0x00);
    EXPECT_EQ(maskOfBits(1), 0x01);
    EXPECT_EQ(maskOfBits(2), 0x03);
    EXPECT_EQ(maskOfBits(8), 0xFF);
    EXPECT_EQ(maskOfBits(15), 0x7FFF);
    EXPECT_EQ(maskOfBits(63), 0x7FFFFFFFFFFFFFFF);
    EXPECT_EQ(maskOfBits(64), 0xFFFFFFFFFFFFFFFF);
    EXPECT_EQ(bits_mask<0>, 0x00);
    EXPECT_EQ(bits_mask<63>, 0x7FFFFFFFFFFFFFFF);
    EXPECT_EQ(bits_mask<64>, 0xFFFFFFFFFFFFFFFF);
    
    uint64_t mask = 0;
    for (int i = 0; i <= 64; i++) {
        EXPECT_EQ(maskOfBits(i), mask);
        mask = mask * 2 + 1;
    }
}
