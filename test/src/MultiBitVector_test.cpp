//
//  MultiBitVector_test.cpp
//  bench
//
//  Created by 松本拓真 on 2018/05/15.
//

#include "gtest/gtest.h"
#include "sim_ds/MultiBitVector.hpp"

#include <random>

using namespace sim_ds;

template <int TYPE_SIZE>
void testElem() {
    MultiBitVector<TYPE_SIZE> multiBits;
    std::random_device rnd;
    const auto size = 0xFFF;
    std::vector<size_t> src(size);
    const auto Max = 1U << TYPE_SIZE;
    for (auto i = 0; i < size; i++) {
        src[i] = i % Max;
        multiBits.set(i, i % Max);
    }
    multiBits.build();
    
    for (auto i = 0; i < size; i++)
        EXPECT_EQ(src[i], multiBits[i]);
}

template <int TYPE_SIZE>
void testRank() {
    MultiBitVector<TYPE_SIZE> multiBits;
    std::random_device rnd;
    const auto size = 0xFFFF;
    std::vector<size_t> src(size);
    const auto Max = 1U << TYPE_SIZE;
    auto cmax = 0;
    auto v = 0;
    auto sum = 0;
    for (auto k = Max; k > 0; k--)
        sum += k;
    std::vector<size_t> smallMap(sum);
    v = 0;
    cmax = 0;
    for (auto i = 0; i < sum; i++) {
        smallMap[i] = v;
        v++;
        if (v > cmax) {
            v = 0;
            cmax++;
        }
    }
    
    for (auto i = 0; i < size; i++) {
        src[i] = smallMap[i % sum];
        multiBits.set(i, smallMap[i % sum]);
    }
    multiBits.build();
    
    std::vector<size_t> ranks(size, 0);
    for (auto i = 0; i < size; i++) {
        if (src[i] == 0) continue;
        auto rank = i / sum * (Max - src[i]);
        for (auto j = 0; j < (i % sum); j++) {
            if (smallMap[j] == src[i])
                rank++;
        }
        ranks[i] = rank;
    }
    for (auto i = 0; i < size; i++) {
        if (src[i] == 0) continue;
        EXPECT_EQ(ranks[i], multiBits.rank(i));
    }
}

TEST(MultiBitVectorTest, ElemTwo) {
    testElem<1>();
}

TEST(MultiBitVectorTest, ElemFour) {
    testElem<2>();
}

TEST(MultiBitVectorTest, ElemEight) {
    testElem<3>();
}

TEST(MultiBitVectorTest, RankTwo) {
    testRank<1>();
}

TEST(MultiBitVectorTest, RankFour) {
    testRank<2>();
}

TEST(MultiBitVectorTest, RankEight) {
    testRank<3>();
}
