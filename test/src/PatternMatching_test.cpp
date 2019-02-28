//
//  PatternMatching_test.cpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/02/27.
//

#include "gtest/gtest.h"
#include "sim_ds/PatternMatching.hpp"

namespace {

const std::string sample_text = "abcabcababcababxabcabx";

const std::string sample_key = "abcaba";
    
const std::vector<size_t> sample_answer{3, 8};

const std::string make_sample_text_large() {
    std::string text = sample_text + sample_text;
    for (size_t i = 0; i < 26; i++)
        text += text.substr(rand() % (text.size() / 2));
    return text;
}

const std::string sample_text_large = make_sample_text_large();

const auto sample_answer_large = sim_ds::PatternMatchSimple(sample_text_large, sample_key); // Forgive me
    
}

TEST(PatternMatching, Simple_sample) {
    auto matchies = sim_ds::PatternMatchSimple(sample_text, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer[i]);
    }
}

TEST(PatternMatching, Kmp_sample) {
    auto matchies = sim_ds::PatternMatchKMP(sample_text, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer[i]);
    }
}

TEST(PatternMatching, Bm_sample) {
    auto matchies = sim_ds::PatternMatchBM(sample_text, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer[i]);
    }
}

TEST(PatternMatching, Bom_sample) {
    auto matchies = sim_ds::PatternMatchBOM(sample_text, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer[i]);
    }
}

TEST(PatternMatching, TurboBom_sample) {
    auto matchies = sim_ds::PatternMatchTurboBOM(sample_text, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer[i]);
    }
}

TEST(PatternMatching, SundayQS_sample) {
    auto matchies = sim_ds::PatternMatchSundayQS(sample_text, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer[i]);
    }
}

TEST(PatternMatching, SundayMS_sample) {
    auto matchies = sim_ds::PatternMatchSundayMS(sample_text, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer[i]);
    }
}

TEST(PatternMatching, Simple_large) {
    auto matchies = sim_ds::PatternMatchSimple(sample_text_large, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer_large.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer_large.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer_large[i]);
    }
}

TEST(PatternMatching, Kmp_large) {
    auto matchies = sim_ds::PatternMatchKMP(sample_text_large, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer_large.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer_large.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer_large[i]);
    }
}

TEST(PatternMatching, Bm_large) {
    auto matchies = sim_ds::PatternMatchBM(sample_text_large, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer_large.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer_large.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer_large[i]);
    }
}

TEST(PatternMatching, Bom_large) {
    auto matchies = sim_ds::PatternMatchBOM(sample_text_large, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer_large.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer_large.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer_large[i]);
    }
}

TEST(PatternMatching, TurboBom_large) {
    auto matchies = sim_ds::PatternMatchTurboBOM(sample_text_large, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer_large.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer_large.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer_large[i]);
    }
}

TEST(PatternMatching, SundayQS_large) {
    auto matchies = sim_ds::PatternMatchSundayQS(sample_text_large, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer_large.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer_large.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer_large[i]);
    }
}

TEST(PatternMatching, SundayMS_large) {
    auto matchies = sim_ds::PatternMatchSundayMS(sample_text_large, sample_key);
    EXPECT_EQ(matchies.size(), sample_answer_large.size());
    for (size_t i = 0; i < std::min(matchies.size(), sample_answer_large.size()); i++) {
        EXPECT_EQ(matchies[i], sample_answer_large[i]);
    }
}
