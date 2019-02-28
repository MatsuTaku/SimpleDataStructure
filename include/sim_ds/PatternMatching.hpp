//
//  PatternMatching.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/02/27.
//

#ifndef PatternMatching_hpp
#define PatternMatching_hpp

#include "basic.hpp"
#include "FactorOracle.hpp"

namespace sim_ds {

// MARK: Simple

std::vector<size_t> PatternMatchSimple(std::string_view text, std::string_view key) {
    std::vector<size_t> matchies;
    const long long kSize = key.size();
    for (size_t i = 0; i <= text.size() - kSize; i++) {
        size_t pos = 0;
        while (pos < kSize and key[pos] == text[i + pos])
            pos++;
        if (pos == kSize) // match
            matchies.push_back(i);
    }
    return matchies;
}


// MARK: KPM

class Kmp {
private:
    std::vector<long long> next_;
    
public:
    Kmp(std::string_view key) : next_(key.size() + 1) {
        next_[0] = -1;
        size_t n = -1;
        for (size_t i = 0; i < key.size(); i++) {
            while (n != -1 and key[i] != key[n])
                n = next(n);
            n++;
            if (i + 1 < key.size() and key[i + 1] == key[n])
                next_[i + 1] = next(n);
            else
                next_[i + 1] = n;
        }
    }
    
    size_t next(size_t pos) const {return next_[pos];}
    
};

std::vector<size_t> PatternMatchKMP(std::string_view text, std::string_view key) {
    Kmp kmp(key);
    
    std::vector<size_t> matchies;
    long long pos = 0;
    for (size_t i = 0; i < text.size(); i++) {
        while (pos > -1 and key[pos] != text[i])
            pos = kmp.next(pos);
        pos++;
        if (pos == key.size()) {
            matchies.push_back(i + 1 - key.size());
        }
    }
    return matchies;
}


// MARK: BM

class Bm {
private:
    std::vector<size_t> skip_;
    
public:
    Bm(std::string_view key) : skip_(0xFF, key.size()) {
        for (size_t i = 0; i < key.size() - 1; i++) {
            skip_[key[i]] = key.size() - i - 1;
        }
    }
    
    size_t skip(uint8_t type) const {
        return skip_[type];
    }
    
};

std::vector<size_t> PatternMatchBM(std::string_view text, std::string_view key) {
    Bm bm(key);
    const size_t kWindowSize = key.size();
    
    std::vector<size_t> matchies;
    long long i = kWindowSize - 1;
    while (i < text.size()) {
        long long pos = i;
        long long k = kWindowSize - 1;
        while (k >= 0 and key[k] == text[pos]) {
            pos--; k--;
        }
        if (k == -1) // match
            matchies.push_back(pos + 1);
        i += bm.skip(text[i]);
        
    }
    return matchies;
}


// MARK: BOM

std::vector<size_t> PatternMatchBOM(std::string_view text, std::string_view key) {
    FactorOracle oracle(key.rbegin(), key.rend());
    const long long kWindowSize = key.size();
    
    std::vector<size_t> matchies;
    long long i = kWindowSize - 1;
    while (i < text.size()) {
        size_t state = 0;
        long long pos = i;
        while (pos > i - kWindowSize and oracle.image(state, text[pos]))
            pos--;
        if (pos == i - kWindowSize) { // match
            matchies.push_back(i + 1 - kWindowSize);
            i++;
        } else {
            i = pos + kWindowSize;
        }
    }
    return matchies;
}


// MARK: Turbo-BOM

// Size rate to key length by KPM search on Turbo-BOM algorithm.
// Value 0.5 is considered to most practical that noticed in paper "Factor Oreacle: A New Structure for Pattern Matching".
constexpr float kTurboBomAlpha = 0.5;

std::vector<size_t> PatternMatchTurboBOM(std::string_view text, std::string_view key) {
    FactorOracle oracle(key.rbegin(), key.rend());
    Kmp kmp(key);
    auto kmp_search = [&key, &kmp](std::string_view text, long long pos = 0) -> long long {
        for (size_t i = 0; i < text.size(); i++) {
            while (pos > -1 and key[pos] != text[i])
                pos = kmp.next(pos);
            pos++;
        }
        return pos;
    };
    const long long kWindowSize = key.size();
    const long long kKmpWindowSize = kWindowSize * kTurboBomAlpha;
    
    std::vector<size_t> matchies;
    long long back = kWindowSize - 1;
    long long critpos = 0;
    while (back < text.size()) {
        size_t state = 0;
        long long pos = back;
        const long long begin = back - kWindowSize + 1;
        while (pos >= begin and oracle.image(state, text[pos]))
            pos--;
        pos++;
        if (pos == begin) { // match
            matchies.push_back(begin);
            critpos = back + 1;
            back += kWindowSize - kmp.next(kWindowSize);
        } else {
            const auto prefix_size = pos > critpos ? 0 : critpos - begin;
            const auto kmp_begin = std::max(pos, critpos);
            const auto next = kmp_search(text.substr(kmp_begin, kKmpWindowSize), prefix_size);
            critpos = kmp_begin + kKmpWindowSize;
            back = kmp_begin + kKmpWindowSize - next + kWindowSize - 1;
        }
    }
    return matchies;
}


// MARK: Sunday

class Sunday {
private:
    std::string_view key_;
    struct Pat {
        long long loc;
        uint8_t c;
    };
    std::vector<Pat> pattern_;
    std::vector<size_t> td1_;
    std::vector<size_t> td2_;
    
public:
    Sunday(std::string_view key) : key_(key) {
        const size_t kKeySize = key_.size();
        std::vector<size_t> min_shift(kKeySize);
        for (long long i = 0; i < kKeySize; i++) {
            long long j = i - 1;
            for (; j >= 0; j--)
                if (key_[j] == key_[i])
                    break;
            min_shift[i] = i - j;
        }
        auto maxshift_comp = [&](Pat& l, Pat& r) -> bool {
            auto lms = min_shift[l.loc], rms = min_shift[r.loc];
            return lms != rms ? lms < rms : l.loc < r.loc;
        };
        for (size_t i = 0; i < key_.size(); i++)
            pattern_.push_back({static_cast<long long>(i), static_cast<uint8_t>(key[i])});
        std::sort(pattern_.begin(), pattern_.end(), maxshift_comp);
    }
    
    const Pat* ordered_pattern(size_t num) const {
        return &pattern_[num];
    }
    
    // Deltas like improved bm skip function
    void BuildTd1() {
        const size_t kKeySize = key_.size();
        td1_.assign(0xFF, kKeySize + 1);
        for (size_t i = 0; i < key_.size(); i++)
            td1_[key_[i]] = kKeySize - i;
    }
    
    size_t delta1(uint8_t type) const {
        return td1_[type];
    }
    
    // Deltas like kpm next function
    void BuildTd2() {
        const size_t kKeySize = key_.size();
        auto match_shift = [&](size_t pos, long long lshift) -> size_t {
            for (; lshift < kKeySize; lshift++) {
                long long ppos = pos;
                while (--ppos >= 0) {
                    auto& pat = pattern_[ppos];
                    long long j = pat.loc - lshift;
                    if (j < 0)
                        continue;
                    if (pat.c != key_[j])
                        break;
                }
                if (ppos == -1)
                    break; // all matched
            }
            return lshift;
        };
        
        td2_.resize(kKeySize + 1);
        td2_[0] = 1;
        for (size_t pos = 1; pos < kKeySize + 1; pos++) {
            td2_[pos] = match_shift(pos, td2_[pos - 1]);
        }
        for (size_t pos = 0; pos < pattern_.size(); pos++) {
            auto lshift = td2_[pos];
            while (lshift < kKeySize) {
                auto& pat = pattern_[pos];
                long long i = pat.loc - lshift;
                if (i < 0 or pat.c != key_[i])
                    break;
                lshift++;
                lshift = match_shift(pos, lshift);
            }
            td2_[pos] = lshift;
        }
    }
    
    size_t delta2(size_t pos) const {
        return td2_[pos];
    }
    
};

std::vector<size_t> PatternMatchSundayQS(std::string_view text, std::string_view key) {
    Sunday sunday(key);
    sunday.BuildTd1();
    const size_t kKeySize = key.size();
    
    std::vector<size_t> matchies;
    size_t i = 0;
    while (i <= text.size() - kKeySize) {
        size_t pos = 0;
        auto* pat = sunday.ordered_pattern(pos);
        while (pos < kKeySize and pat->c == text[i + pat->loc]) {
            pos++; ++pat;
        }
        if (pos == kKeySize)
            matchies.push_back(i);
        if (i + kKeySize == text.size())
            break;
        i += sunday.delta1(text[i + kKeySize]);
    }
    return matchies;
}

std::vector<size_t> PatternMatchSundayMS(std::string_view text, std::string_view key) {
    Sunday sunday(key);
    sunday.BuildTd1();
    sunday.BuildTd2();
    const size_t kKeySize = key.size();
    
    std::vector<size_t> matchies;
    size_t i = 0;
    while (i <= text.size() - kKeySize) {
        size_t pos = 0;
        auto* pat = sunday.ordered_pattern(pos);
        while (pos < kKeySize and pat->c == text[i + pat->loc]) {
            pos++; ++pat;
        }
        if (pos == kKeySize)
            matchies.push_back(i);
        if (i + kKeySize == text.size())
            break;
        i += std::max(sunday.delta1(text[i + kKeySize]), sunday.delta2(pos));
    }
    return matchies;
}

}

#endif /* PatternMatching_hpp */
