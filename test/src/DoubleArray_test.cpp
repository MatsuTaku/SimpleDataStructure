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
    const size_t kCharPattern = 'z'-'a'+1;
    for (size_t i = 0; i < size; i++) {
        std::string s;
        auto key_size = rand() % kMaxKeySize+1;
        for (size_t j = 0; j < key_size; j++) {
            s.push_back('a'+uint8_t(rand() % kCharPattern));
        }
        keyset.push_back(s);
    }
    sort(keyset.begin(), keyset.end());
    keyset.erase(std::unique(keyset.begin(), keyset.end()), keyset.end());
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
    assert(set.end() - set.begin() == 5);
    using double_array_type = DoubleArray<char, uint32_t>;
    double_array_type da(set);
    for (std::string_view s : set) {
        EXPECT_TRUE(da.find(s) != nullptr);
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
    using double_array_type = DoubleArray<char, uint32_t>;
    double_array_type da(set);
    for (std::string_view s : set) {
        EXPECT_TRUE(da.find(s) != nullptr);
    }
}

TEST(DoubleArrayTest, SampleLargeLegacy) {
    using double_array_type = DoubleArray<char, uint32_t>;
    sim_ds::Stopwatch sw;
    double_array_type da(large_keyset);
    std::cout << "Build time(m): " << sw.get_milli_sec() << std::endl;
    for (std::string_view s : large_keyset) {
        EXPECT_TRUE(da.find(s) != nullptr);
    }
}

TEST(DoubleArrayTest, SampleLarge) {
    using double_array_type = DoubleArray<char, uint32_t>;
    sim_ds::Stopwatch sw;
    double_array_type da(large_keyset);
    std::cout << "Build time(m): " << sw.get_milli_sec() << std::endl;
    for (std::string_view s : large_keyset) {
        EXPECT_TRUE(da.find(s) != nullptr);
    }
}
