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
    
    size_t skip(uint8_t pos) const {
        return skip_[pos];
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

std::vector<size_t> PatternMatchTurboBOM(std::string_view text, std::string_view key) {
    FactorOracle oracle(key.rbegin(), key.rend());
    Kmp kmp(key);
    auto kmp_search = [&key, &kmp](std::string_view text) -> long long {
        long long pos = 0;
        for (size_t i = 0; i < text.size(); i++) {
            while (pos > -1 and key[pos] != text[i])
                pos = kmp.next(pos);
            pos++;
        }
        return pos;
    };
    const size_t kWindowSize = key.size();
    const double alpha = 0.1;
    const size_t kKmpWindowSize = (kWindowSize - 1) * alpha + 1;
    
    std::vector<size_t> matchies;
    size_t i = kWindowSize - 1;
    long long critpos = 0;
    while (i < text.size()) {
        size_t state = 0;
        long long pos = i;
        while (pos >= critpos and oracle.image(state, text[pos]))
            pos--;
        if (pos == critpos - 1) { // match
            matchies.push_back(i + 1 - kWindowSize);
            auto next = kmp.next(kWindowSize);
            critpos = i + 1;
            i += kWindowSize - next;
        } else {
            auto next = kmp_search(text.substr(pos + 1, kKmpWindowSize));
            i = pos + kKmpWindowSize - next + kWindowSize;
            critpos = pos + kKmpWindowSize + 1;
        }
    }
    return matchies;
}

}

#endif /* PatternMatching_hpp */
