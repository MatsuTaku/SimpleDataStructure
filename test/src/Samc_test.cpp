//
//  Samc_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/05/25.
//

#include "gtest/gtest.h"
#include "sim_ds/string_util/Samc.hpp"

using namespace sim_ds;

namespace {
const std::vector<std::string> sample = {
    "ab",
    "abc",
    "b",
    "bac",
    "bb"
};
}

TEST(SamcTest, Sample) {
    Samc<uint32_t, true> samc(sample.begin(), sample.end());
    for (auto& s : sample) {
        EXPECT_TRUE(samc.accept(s));
    }
}

TEST(SamcTest, SampleLookup) {
    SamcDict<uint32_t> samc(sample.begin(), sample.end());

    std::set<size_t> ids;
    for (size_t i = 0; i < sample.size(); i++) {
        ids.insert(samc.lookup(sample[i]));
    }
    EXPECT_EQ(ids.size(), sample.size());
}

TEST(SamcTest, SampleAccess) {
    SamcDict<uint32_t> samc(sample.begin(), sample.end());

    std::vector<std::string> gets;
    for (size_t i = 0; i < sample.size(); i++) {
        gets.push_back(samc.access(i));
    }
    sort(gets.begin(), gets.end());
    for (size_t i = 0; i < gets.size(); i++) {
        EXPECT_EQ(gets[i], sample[i]);
    }
}

//TEST(SamcTest, corpus) {
//    std::vector<std::string> corpus;
//    std::ifstream ifs("../../corpus/Japan_Postal_Code_sorted.txt");
//    if (!ifs) {
//        std::cerr<<"file not found" << std::endl;
//        exit(1);
//    }
//    std::string s;
//    for (std::string s; std::getline(ifs, s); ) {
//        if (s.empty()) continue;
//        corpus.push_back(s);
//        std::cerr<<s<<std::endl;
//    }
//    Samc<uint32_t> samc(corpus.begin(), corpus.end());
//    for (auto& s : corpus) {
//        EXPECT_TRUE(samc.accept(s));
//    }
//}
