//
//  calc.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/12/10.
//

#ifndef calc_hpp
#define calc_hpp

#include <vector>

namespace sim_ds {
    
namespace calc {
    
/* Calculate minimal number of units of argument required for value expression. */
inline constexpr size_t SizeFitsInUnits(unsigned long long value, const size_t unit) {
    size_t size = 0;
    while (static_cast<bool>(value >> (++size * unit))) continue;
    return size;
}

template<size_t _Bits>
inline constexpr size_t size_fits(unsigned long long value) {
    return SizeFitsInUnits(value, _Bits);
}

/* Calculate minimal number of bytes required for value expression. */
inline constexpr size_t SizeFitsInBytes(unsigned long long value) {
    return size_fits<8>(value);
}

/* Calculate minimal number of bits required for value expression. */
inline constexpr size_t SizeFitsInBits(unsigned long long value) {
    return size_fits<1>(value);
}

template <typename CONTAINER>
inline size_t SizeFitsAsList(unsigned long long value, const CONTAINER sizes) {
    size_t size = 0;
    while (static_cast<bool>(value >>= sizes[size++])) continue;
    return size;
}

template <class CONTAINER>
inline std::vector<size_t> bit_size_frequency_list(const CONTAINER& list, bool shouldShow = false) {
    std::vector<size_t> map;
    auto maxSize = 0;
    for (size_t i = 0; i < list.size(); i++) {
        auto size = SizeFitsInBits(list[i]);
        if (size > maxSize) {
            map.resize(size, 0);
            maxSize = size;
        }
        map[size - 1]++;
    }
    
    if (shouldShow) {
        for (auto i = 0; i < map.size(); i++)
            std::cout << "[" << i + 1 << "]: " << map[i] << std::endl;
    }
    
    return map;
}

template <class CONTAINER>
inline std::vector<size_t> cummulative_frequency_list(const CONTAINER& list, bool shouldShow = false) {
    std::vector<size_t> cf;
    auto map = bit_size_frequency_list(list);
    auto count = 0;
    cf.assign(map.size(), 0);
    for (auto i = map.size(); i > 0; i--) {
        count += map[i - 1];
        cf[i - 1] = count;
    }
    
    if (shouldShow) {
        std::cout << "Cummulative frequency of vector" << std::endl;
        for (int i = 0; i < cf.size(); i++)
            std::cout << "[" << i + 1 << "]: " << map[i] << std::endl;
    }
    
    return cf;
}

inline size_t additional_size_of_rank(const double l) {
    using std::ceil;
    auto a = ceil(l / 8) * 8;
    auto b = (4 * 8 + ceil(log2(l))) * ceil(l / 256) ;
    return a + b;
}

template <class CONTAINER>
inline std::vector<size_t> split_positions_optimized_for_dac(const CONTAINER& list, const size_t maxLevels = 8) {
    auto cf = cummulative_frequency_list(list);
    
    const auto m = cf.size() - 1;
    std::vector<size_t> s(cf.size(), 0), l(cf.size(), 0), b(cf.size(), 0);
    for (int t = m; t >= 0; --t) {
        auto minSize = INFINITY;
        auto minPos = m;
        for (auto i = t + 1; i <= m; i++) {
            auto currentSize = s[i] + cf[t] * (i - t) + additional_size_of_rank(cf[t]);
            if (currentSize < minSize) {
                minSize = currentSize;
                minPos = i;
            }
        }
        if (minSize < cf[t] * ((m + 1) - t)) {
            s[t] = minSize;
            l[t] = l[minPos] + 1;
            b[t] = minPos - t;
        } else {
            s[t] = cf[t] * ((m + 1) - t);
            l[t] = 1;
            b[t] = (m + 1) - t;
        }
    }
    auto L = l[0];
    auto t = 0;
    std::vector<size_t> bk;
    bk.reserve(8);
    bk.resize(L);
    for (auto k = 0; k <= L; k++) {
        bk[k] = b[t];
        t = t + b[t];
    }
    
    if (L > maxLevels) { // TODO: Skeptical algorithm
        std::vector<bool> sepPos(cf.size() + 1, false);
        auto p = 0;
        for (auto v : bk) {
            p += v;
            sepPos[p] = true;
        }
        using P = std::pair<uint64_t, uint64_t>;
        auto cmp = [](P lhs, P rhs) {
            return lhs.first > rhs.first;
        };
        std::vector<P> pq;
        for (auto i = 1; i < m; i++) {
            pq.push_back(P(cf[i - 1] - cf[i], i));
        }
        std::sort(pq.begin(), pq.end(), cmp);
        auto numSeps = L;
        while (numSeps > maxLevels && !pq.empty()) {
            auto curP = pq.back();
            pq.pop_back();
            if (sepPos[curP.second]) {
                sepPos[curP.second] = false;
                numSeps--;
            }
        }
        
        bk.resize(0);
        auto lastP = 0;
        for (auto i = 0; i < sepPos.size(); i++) {
            if (sepPos[i]) {
                bk.push_back(i - lastP);
                lastP = i;
            }
        }
    }
    

    return bk;
}
    
} // namespace calc
    
} // namespace sim_ds

#endif /* calc_hpp */
