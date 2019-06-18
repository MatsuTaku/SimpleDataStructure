//
//  DoubleArray_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/06/18.
//

#include "gtest/gtest.h"
#include "sim_ds/DoubleArray.hpp"

using namespace sim_ds;

namespace {

std::vector<std::string> make_sample_keyset(size_t size) {
    std::vector<std::string> keyset;
    const size_t kMaxKeySize = 32;
    const size_t kCharPattern = 32;
    for (size_t i = 0; i < size; i++) {
        std::string s;
        auto key_size = rand() % kMaxKeySize;
        for (size_t j = 0; j < key_size; j++) {
            s.push_back(uint8_t(kCharPattern - rand() % kCharPattern));
        }
        keyset.push_back(s);
    }
    return keyset;
}

std::vector<std::string> large_keyset = make_sample_keyset(0x10000);

}


TEST(DoubleArrayTest, SampleLegacy) {
    std::vector<std::string> set = {
        "ab",
        "abc",
        "b",
        "bac",
        "bb"
    };
    using double_array_type = DoubleArray<uint32_t, true>;
    double_array_type::input_trie trie;
    for (std::string_view s : set) {
        trie.insert(s, 1);
    }
    double_array_type da(trie);
    for (std::string_view s : set) {
        EXPECT_TRUE(da.accept(s));
    }
}

TEST(DoubleArrayTest, Sample) {
    std::vector<std::string> set = {
        "ab",
        "abc",
        "b",
        "bac",
        "bb"
    };
    using double_array_type = DoubleArray<uint32_t, false>;
    double_array_type::input_trie trie;
    for (std::string_view s : set) {
        trie.insert(s, 1);
    }
    double_array_type da(trie);
    for (std::string_view s : set) {
        EXPECT_TRUE(da.accept(s));
    }
}

TEST(DoubleArrayTest, SampleLargeLegacy) {
    using double_array_type = DoubleArray<uint32_t, true>;
    double_array_type::input_trie trie;
    for (auto& s : large_keyset) {
        trie.insert(s, 1);
    }
    sim_ds::Stopwatch sw;
    double_array_type da(trie);
    std::cout << "Build time(m): " << sw.get_milli_sec() << std::endl;
    for (std::string_view s : large_keyset) {
        EXPECT_TRUE(da.accept(s));
    }
}

TEST(DoubleArrayTest, SampleLarge) {
    using double_array_type = DoubleArray<uint32_t, false>;
    double_array_type::input_trie trie;
    for (auto& s : large_keyset) {
        trie.insert(s, 1);
    }
    sim_ds::Stopwatch sw;
    double_array_type da(trie);
    std::cout << "Build time(m): " << sw.get_milli_sec() << std::endl;
    for (std::string_view s : large_keyset) {
        EXPECT_TRUE(da.accept(s));
    }
}
