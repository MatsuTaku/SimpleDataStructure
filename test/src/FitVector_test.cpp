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
    const auto size = 0xFFFFFF;
    
    std::vector<size_t> source(size);
    for (auto i = 1; i <= source.size(); i++)
        source[i] = (1U << (rand() % 18)) - 1;
    FitVector vector(source);
    for (auto i = 0; i < size; i++)
        EXPECT_EQ(source[i], vector[i]);
}

TEST(FitVectorTest, Move) {
    const size_t size = 0xFFFFFF;
    
    auto value = 0x1a5a5;
    auto vec = FitVector(17, size, value);
    for (auto i = 0; i < size; i++)
        EXPECT_EQ(vec[i], value);
    
    auto vec2 = std::move(vec);
    for (auto i = 0; i < size; i++)
        EXPECT_EQ(vec2[i], value);
    
}
