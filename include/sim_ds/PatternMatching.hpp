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

namespace pattern_matching {
    
// MARK: - Algorithms

class _PatternMatchingInterface {
protected:
    std::string_view key_;
    
public:
    _PatternMatchingInterface(std::string_view key) : key_(key) {}
    
    virtual std::vector<size_t> find_all(std::string_view text) const = 0;
    
};
    
    
// MARK: Simple

class Simple : _PatternMatchingInterface {
public:
    Simple(std::string_view key) : _PatternMatchingInterface(key) {}
    
    std::vector<size_t> find_all(std::string_view text) const override {
        std::vector<size_t> matchies;
        const long long kSize = key_.size();
        for (size_t i = 0; i <= text.size() - kSize; i++) {
            size_t pos = 0;
            while (pos < kSize and key_[pos] == text[i + pos])
                pos++;
            if (pos == kSize) // match
                matchies.push_back(i);
        }
        return matchies;
    }
    
};


// MARK: KPM

class Kmp : _PatternMatchingInterface {
private:
    std::vector<long long> next_;
    
public:
    Kmp(std::string_view key) : _PatternMatchingInterface(key), next_(key.size() + 1) {
        next_[0] = -1;
        size_t n = -1;
        for (size_t i = 0; i < key.size(); i++) {
            while (n != -1 and key[i] != key[n])
                n = next_[n];
            n++;
            if (i + 1 < key.size() and key[i + 1] == key[n])
                next_[i + 1] = next_[n];
            else
                next_[i + 1] = n;
        }
    }
    
    size_t next(size_t pos) const {return next_[pos];}
    
    std::vector<size_t> find_all(std::string_view text) const override {
        std::vector<size_t> matchies;
        long long pos = 0;
        for (size_t i = 0; i < text.size(); i++) {
            while (pos > -1 and key_[pos] != text[i])
                pos = next_[pos];
            pos++;
            if (pos == key_.size()) {
                matchies.push_back(i + 1 - key_.size());
            }
        }
        return matchies;
    }
    
};


// MARK: BM

class Bm : _PatternMatchingInterface {
private:
    std::vector<size_t> skip_;
    
public:
    Bm(std::string_view key) : _PatternMatchingInterface(key), skip_(0xFF, key.size()) {
        for (size_t i = 0; i < key.size() - 1; i++) {
            skip_[key[i]] = key.size() - i - 1;
        }
    }
    
    size_t skip(uint8_t type) const {return skip_[type];}
    
    std::vector<size_t> find_all(std::string_view text) const override {
        const size_t kWindowSize = key_.size();
        
        std::vector<size_t> matchies;
        long long i = kWindowSize - 1;
        while (i < text.size()) {
            long long pos = i;
            long long k = kWindowSize - 1;
            while (k >= 0 and key_[k] == text[pos]) {
                pos--; k--;
            }
            if (k == -1) // match
                matchies.push_back(pos + 1);
            i += skip_[text[i]];
            
        }
        return matchies;
    }
    
};


// MARK: BOM
    
class Bom : virtual protected _PatternMatchingInterface {
protected:
    FactorOracle oracle_;
    
public:
    Bom(std::string_view key) : _PatternMatchingInterface(key), oracle_(key.rbegin(), key.rend()) {}
    
    std::vector<size_t> find_all(std::string_view text) const override {
        const long long kWindowSize = key_.size();
        
        std::vector<size_t> matchies;
        long long i = kWindowSize - 1;
        while (i < text.size()) {
            size_t state = 0;
            long long pos = i;
            while (pos > i - kWindowSize and oracle_.image(state, text[pos]))
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
    
};


// MARK: Turbo-BOM

class TurboBom : public Bom {
private:
    Kmp kmp_;
    
    // Size rate to key length by KPM search on Turbo-BOM algorithm.
    // Value 0.5 is considered to most practical one that noticed in paper "Factor Oreacle: A New Structure for Pattern Matching".
    static constexpr float kAlpha = 0.5;
    
public:
    TurboBom(std::string_view key) : _PatternMatchingInterface(key), Bom(key), kmp_(key) {}
    
    std::vector<size_t> find_all(std::string_view text) const override {
        const long long kWindowSize = key_.size();
        const long long kKmpWindowSize = kWindowSize * kAlpha;
        
        std::vector<size_t> matchies;
        long long back = kWindowSize - 1;
        long long critpos = 0;
        while (back < text.size()) {
            size_t state = 0;
            long long pos = back;
            const long long begin = back - kWindowSize + 1;
            while (pos >= begin and oracle_.image(state, text[pos]))
                pos--;
            pos++;
            if (pos == begin) { // match
                matchies.push_back(begin);
                critpos = back + 1;
                back += kWindowSize - kmp_.next(kWindowSize);
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
    
private:
    long long kmp_search(std::string_view text, long long pos = 0) const {
        for (size_t i = 0; i < text.size(); i++) {
            while (pos > -1 and key_[pos] != text[i])
                pos = kmp_.next(pos);
                pos++;
        }
        return pos;
    };
    
};


// MARK: Sunday

class Sunday : virtual protected _PatternMatchingInterface {
protected:
    struct Pat {
        long long loc;
        uint8_t c;
    };
    std::vector<Pat> pattern_;
    
public:
    Sunday(std::string_view key) : _PatternMatchingInterface(key) {
        const size_t kKeySize = key.size();
        std::vector<size_t> min_shift(kKeySize);
        for (long long i = 0; i < kKeySize; i++) {
            long long j = i - 1;
            for (; j >= 0; j--)
                if (key[j] == key[i])
                    break;
            min_shift[i] = i - j;
        }
        auto maxshift_comp = [&](const Pat& l, const Pat& r) -> bool {
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
    
};

class SundayQS : public Sunday {
private:
    std::vector<size_t> td1_;
    
public:
    // Deltas like improved bm skip function
    SundayQS(std::string_view key) : _PatternMatchingInterface(key), Sunday(key) {
        const size_t kKeySize = key_.size();
        td1_.assign(0xFF, kKeySize + 1);
        for (size_t i = 0; i < key_.size(); i++)
            td1_[key_[i]] = kKeySize - i;
    }
    
    size_t delta1(uint8_t type) const {return td1_[type];}
    
    std::vector<size_t> find_all(std::string_view text) const override {
        const size_t kKeySize = key_.size();
        
        std::vector<size_t> matchies;
        size_t i = 0;
        while (i <= text.size() - kKeySize) {
            size_t pos = 0;
            auto* pat = ordered_pattern(pos);
            while (pos < kKeySize and pat->c == text[i + pat->loc]) {
                pos++; ++pat;
            }
            if (pos == kKeySize)
                matchies.push_back(i);
            if (i + kKeySize == text.size())
                break;
            i += delta1(text[i + kKeySize]);
        }
        return matchies;
    }
 
};

class SundayMS : public SundayQS {
private:
    std::vector<size_t> td2_;
    
public:
    // Deltas like kpm next function
    SundayMS(std::string_view key) : _PatternMatchingInterface(key), SundayQS(key) {
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
    
    size_t delta2(size_t pos) const {return td2_[pos];}
    
    std::vector<size_t> find_all(std::string_view text) const override {
        const size_t kKeySize = key_.size();
        
        std::vector<size_t> matchies;
        size_t i = 0;
        while (i <= text.size() - kKeySize) {
            size_t pos = 0;
            auto* pat = ordered_pattern(pos);
            while (pos < kKeySize and pat->c == text[i + pat->loc]) {
                pos++; ++pat;
            }
            if (pos == kKeySize)
                matchies.push_back(i);
            if (i + kKeySize == text.size())
                break;
            i += std::max(delta1(text[i + kKeySize]), delta2(pos));
        }
        return matchies;
    }
    
};


// MARK: - Pattern matching functions

std::vector<size_t> PatternMatchSimple(std::string_view text, std::string_view key) {
    return Simple(key).find_all(text);
}

std::vector<size_t> PatternMatchKMP(std::string_view text, std::string_view key) {
    return Kmp(key).find_all(text);
}

std::vector<size_t> PatternMatchBM(std::string_view text, std::string_view key) {
    return Bm(key).find_all(text);
}

std::vector<size_t> PatternMatchBOM(std::string_view text, std::string_view key) {
    return Bom(key).find_all(text);
}

std::vector<size_t> PatternMatchTurboBOM(std::string_view text, std::string_view key) {
    return TurboBom(key).find_all(text);
}

std::vector<size_t> PatternMatchSundayQS(std::string_view text, std::string_view key) {
    return SundayQS(key).find_all(text);
}

std::vector<size_t> PatternMatchSundayMS(std::string_view text, std::string_view key) {
    return SundayMS(key).find_all(text);
}


} // namespace pattern_matching
    
} // namespace sim_ds

#endif /* PatternMatching_hpp */
