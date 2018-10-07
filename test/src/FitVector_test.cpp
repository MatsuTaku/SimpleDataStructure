//
//  FitVector_test.cpp
//  ALL_BUILD
//
//  Created by 松本拓真 on 2018/05/14.
//

#include "sim_ds/FitVector.hpp"
#include "gtest/gtest.h"

using namespace sim_ds;

TEST(FitVectorTest, ConvertVector) {
    const auto size = 16;
    std::vector<size_t> source(size);
    for (auto i = 1; i <= source.size(); i++)
        source[i] = 0x71c71c71c71c71c7 & ((1U << (i * 4)) - 1);
    FitVector vector(source);
    for (auto i = 0; i < size; i++)
        EXPECT_EQ(source[i], vector[i]);
}

TEST(FitVectorTest, Move) {
    auto vec = FitVector(16, 0xFFF, 0xF0F0);
    for (auto i = 0; i < 0xFFF; i++)
        EXPECT_EQ(vec[i], 0xF0F0);
    
    auto vec2 = std::move(vec);
    for (auto i = 0; i < 0xFFF; i++)
        EXPECT_EQ(vec2[i], 0xF0F0);
    
}
