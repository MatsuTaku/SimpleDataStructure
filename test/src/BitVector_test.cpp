//
//  BitVector_test.cpp
//  bench
//
//  Created by 松本拓真 on 2018/06/12.
//

#include "sim_ds/BitVector.hpp"
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
    const auto size = 0xFFF;
    std::vector<bool> bits(size);
    std::vector<size_t> ranks(size);
    size_t count = 0;
    for (auto i = 0; i < bits.size(); i++) {
        ranks[i] = count;
        if (rand() % 7 == 0) {
            bits[i] = true;
            count++;
        }
    }
    
    BitVector bv(bits, true);
    for (auto i = 0; i < bits.size(); i++)
        EXPECT_EQ(bv.rank(i), ranks[i]);
    
}
