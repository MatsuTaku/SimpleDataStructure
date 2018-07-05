
//
//  WaveletTree_test.cpp
//  bench
//
//  Created by 松本拓真 on 2018/06/07.
//

#include "WaveletTree.hpp"
#include "gtest/gtest.h"

using namespace sim_ds;

TEST(WaveletTreeTest, Unit) {
    std::vector<int> vec = {0, 1, 2, 3, 4, 5, 6, 7};
    WaveletTree wt(vec);
    for (auto i = 0; i < vec.size(); i++) {
        EXPECT_EQ(vec[i], wt[i]);
    }
}

TEST(WaveletTreeTest, Convert) {
    const auto size = 0xFFFF;
    std::vector<size_t> src(size);
    const auto Max = 8;
    for (auto i = 0; i < size; i++) {
        auto r1 = std::rand() % Max + 1;
        src[i] = std::rand() % r1;
    }
    
    WaveletTree wv(src);
    
    for (auto i = 0; i < size; i++) {
//        std::cout << "[" << i << "]" << std::endl;
        EXPECT_EQ(src[i], wv[i]);
    }
}

TEST(WaveletTreeTest, Rank) {
    const auto size = 0xFFFF;
    std::vector<size_t> src(size);
    const auto Max = 8;
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
    }
    WaveletTree wv(src);
    
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
        EXPECT_EQ(ranks[i], wv.rank(i));
    }
}
