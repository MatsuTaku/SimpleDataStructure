//
//  Mask_test.cpp
//  sim_ds
//
//  Created by 松本拓真 on 2018/11/22.
//

#include "gtest/gtest.h"
#include "sim_ds/bit_util.hpp"

using namespace sim_ds::bit_util;

TEST(MaskTest, bits) {
    EXPECT_EQ(WidthMask(0), 0x00);
    EXPECT_EQ(WidthMask(1), 0x01);
    EXPECT_EQ(WidthMask(2), 0x03);
    EXPECT_EQ(WidthMask(8), 0xFF);
    EXPECT_EQ(WidthMask(15), 0x7FFF);
    EXPECT_EQ(WidthMask(63), 0x7FFFFFFFFFFFFFFF);
    EXPECT_EQ(WidthMask(64), 0xFFFFFFFFFFFFFFFF);
    EXPECT_EQ(width_mask<0>, 0x00);
    EXPECT_EQ(width_mask<63>, 0x7FFFFFFFFFFFFFFF);
    EXPECT_EQ(width_mask<64>, 0xFFFFFFFFFFFFFFFF);
    
    uint64_t mask = 0;
    for (int i = 0; i <= 64; i++) {
        EXPECT_EQ(WidthMask(i), mask);
        mask = mask * 2 + 1;
    }
}

TEST(MaskTest, offset) {
    EXPECT_EQ(OffsetMask(0), 0x01);
    EXPECT_EQ(OffsetMask(1), 0x02);
    EXPECT_EQ(OffsetMask(2), 0x04);
    EXPECT_EQ(OffsetMask(8), 0x100);
    EXPECT_EQ(OffsetMask(15), 0x8000);
    EXPECT_EQ(OffsetMask(63), 0x8000000000000000);
    EXPECT_EQ(offset_mask<0>, 0x01);
    EXPECT_EQ(offset_mask<63>, 0x8000000000000000);
    
    uint64_t mask = 1;
    for (int i = 0; i < 64; i++) {
        EXPECT_EQ(OffsetMask(i), mask);
        mask <<= 1;
    }
}
