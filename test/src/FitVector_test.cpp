//
//  FitVector_test.cpp
//  ALL_BUILD
//
//  Created by 松本拓真 on 2018/05/14.
//

#include "gtest/gtest.h"
#include "sim_ds/FitVector.hpp"

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
    auto value = 0x1a5a5;
    auto vec = FitVector(17, 0xFFF, value);
    for (auto i = 0; i < 0xFFF; i++)
        EXPECT_EQ(vec[i], value);
    
    auto vec2 = std::move(vec);
    for (auto i = 0; i < 0xFFF; i++)
        EXPECT_EQ(vec2[i], value);
    
}
