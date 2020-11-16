#include "gtest/gtest.h"
#include "sim_ds/CHT.hpp"

#include <random>

TEST(CHT, CHTSetGet) {
    constexpr unsigned bits = 16;
    constexpr size_t size = 1u<<bits;

    std::vector<bool> src(size);
    std::random_device rnd;
    sim_ds::CHT<bits> cht(bits*2, size*2);
    for (int i = 0; i < size; i++) {
        uint64_t key = uint64_t(i) << bits;
        if (rnd()%2 == 0) {
            src[i] = true;
            cht.set(key, i);
        }
    }
    for (int i = 0; i < size; i++) {
        auto [s, v] = cht.get(uint64_t(i) << bits);
        if (src[i]) {
            EXPECT_TRUE(s);
            EXPECT_EQ(v, i);
        } else {
            EXPECT_FALSE(s);
        }
    }
}

TEST(CHT, CHTSetGetGrow) {
    constexpr unsigned bits = 16;
    constexpr size_t size = 1u<<bits;

    std::vector<bool> src(size);
    std::random_device rnd;
    sim_ds::CHT<bits> cht(bits*2);
    for (int i = 0; i < size; i++) {
        uint64_t key = uint64_t(i) << bits;
        if (rnd()%2 == 0) {
            src[i] = true;
            cht.set(key, i);
        }
    }
    for (int i = 0; i < size; i++) {
        auto [s, v] = cht.get(uint64_t(i) << bits);
        if (src[i]) {
            EXPECT_TRUE(s);
            EXPECT_EQ(v, i);
        } else {
            EXPECT_FALSE(s);
        }
    }
}