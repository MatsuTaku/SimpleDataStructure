//
//  BitVector_test.cpp
//  bench
//
//  Created by 松本拓真 on 2018/06/12.
//

#include "gtest/gtest.h"
#include "sim_ds/BitVector.hpp"

using namespace sim_ds;

TEST(BitVectorTest, Convert) {
    std::vector<bool> bits(0xFFFFFF);
    for (auto i = 0; i < bits.size(); i++)
        bits[i] = rand() % 2;
    
    BitVector bv(bits, false);
    for (auto i = 0; i < bits.size(); i++)
        EXPECT_EQ(bits[i], bv[i]);
    
}

TEST(BitVectorTest, Rank) {
    const auto size = 0xFFFFFF;
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

TEST(BitVectorTest, Select) {
    const auto size = 0xFFFFFF;
    std::vector<bool> bits(size);
    std::vector<size_t> selects;
    for (auto i = 0; i < bits.size();) {
        bits[i] = true;
        selects.push_back(i);
        size_t randLen = rand() % 4 + 1;
        i += randLen;
    }
    
    BitVector bv(bits, true, true);
    for (auto i = 0; i < selects.size(); i++)
        EXPECT_EQ(bv.select(i), selects[i]);
    
}
