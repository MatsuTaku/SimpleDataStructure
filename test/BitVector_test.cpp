//
//  BitVector_test.cpp
//  bench
//
//  Created by 松本拓真 on 2018/06/12.
//

#include "BitVector.hpp"
#include "gtest/gtest.h"

using namespace sim_ds;

TEST(BitVectorTest, Convert) {
    std::vector<bool> bits(0xFFF);
    for (auto i = 0; i < bits.size(); i++)
        bits[i] = rand() % 2;
    
    BitVector bv(bits, false);
    for (auto i = 0; i < bits.size(); i++)
        EXPECT_EQ(bits[i], bv[i]);
    
}

TEST(BitVectorTest, Rank) {
    std::vector<bool> bits(0xFFF);
    for (auto i = 0; i < bits.size(); i++)
        bits[i] = i % 3 == 2;
    
    BitVector bv(bits, true);
    for (auto i = 0; i < bits.size(); i++)
        EXPECT_EQ(i / 3, bv.rank(i));
    
}
