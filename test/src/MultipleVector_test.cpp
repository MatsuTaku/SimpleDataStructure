//
//  MultipleVector_test.cpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/07/09.
//

#include "gtest/gtest.h"
#include "sim_ds/MultipleVector.hpp"

using namespace sim_ds;

namespace {
    
class Wrapper {
    const MultipleVector& base_;
public:
    Wrapper(const MultipleVector& base) : base_(base) {}
    
    auto block(size_t index) const {
        return base_.block(index);
    }
    
};
    
std::vector<uint64_t> rand_vector(size_t size, size_t width = 64) {
    std::vector<uint64_t> vec(size);
    for (auto i = 0; i < size; i++) {
        auto rndWidth = rand() % width;
        vec[i] = 1ULL << rndWidth;
    }
    return vec;
}
    
}

TEST(MultipleVectorTest, Convert) {
    const auto size = 0x1000000;
    
    auto next_src = rand_vector(size, 32);
    auto check_src = rand_vector(size, 8);
    
    MultipleVector fva;
    fva.set_element_sizes({4, 1});
    fva.resize(size);
    
    for (auto i = 0; i < size; i++) {
        fva.set_nested_element<0>(i, next_src[i]);
        fva.set_nested_element<1>(i, check_src[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto next = fva.nested_element<0>(i);
        EXPECT_EQ(next, next_src[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto check = fva.nested_element<1>(i);
        EXPECT_EQ(check, check_src[i]);
    }
    
}

TEST(MultipleVectorTest, Operator) {
    const auto size = 0x1000000;
    
    auto next_src = rand_vector(size, 32);
    auto check_src = rand_vector(size, 8);
    
    MultipleVector fva;
    Wrapper wrapper(fva);
    fva.set_element_sizes({4, 1});
    fva.resize(size);
    
    for (auto i = 0; i < size; i++) {
        fva.block(i).template set<0>(next_src[i]);
        fva.block(i).template set<1>(check_src[i]);
    }
    
    for (auto i = 0; i < size; i++) {
        auto next = wrapper.block(i).template get<0>();
        EXPECT_EQ(next, next_src[i]);
    }

    for (auto i = 0; i < size; i++) {
        auto check = wrapper.block(i).template get<1>();
        EXPECT_EQ(check, check_src[i]);
    }
}
