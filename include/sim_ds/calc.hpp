//
//  calc.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/12/10.
//

#ifndef calc_hpp
#define calc_hpp

#include <vector>

namespace sim_ds::calc {

/* Calculate minimal number of units of argument required for value expression. */
inline constexpr size_t SizeFitsInUnits(unsigned long long value, const size_t unit) {
    size_t size = 0;
    while (static_cast<bool>(value >> (++size * unit))) continue;
    return size;
}

template<size_t Bits>
inline constexpr size_t size_fits(unsigned long long value) {
    return SizeFitsInUnits(value, Bits);
}

/* Calculate minimal number of bytes required for value expression. */
inline constexpr size_t SizeFitsInBytes(unsigned long long value) {
    return size_fits<8>(value);
}

/* Calculate minimal number of bits required for value expression. */
inline constexpr size_t SizeFitsInBits(unsigned long long value) {
    return size_fits<1>(value);
}

template <typename Container>
inline size_t SizeFitsAsList(unsigned long long value, const Container sizes) {
    size_t size = 0;
    while (static_cast<bool>(value >>= sizes[size++])) continue;
    return size;
}

template <class Container, typename T>
inline void bit_length_frequencies(const Container& list, std::vector<T>* result) {
    std::vector<size_t> map;
    size_t max_size = 0;
    for (size_t i = 0; i < list.size(); i++) {
        auto size = SizeFitsInBits(list[i]);
        if (size > max_size) {
            map.resize(size, 0);
            max_size = size;
        }
        map[size - 1]++;
    }
    
    result->reserve(map.size());
    result->resize(0);
    std::transform(map.begin(), map.end(), std::back_inserter(*result), [](auto x) {return x;});
}

template <class Container, typename T>
inline void cummulative_frequency_list(const Container& list, std::vector<T>* result, bool should_show = false) {
    std::vector<size_t> cf;
    std::vector<size_t> map;
    bit_length_frequencies(list, &map);
    auto count = 0;
    cf.assign(map.size(), 0);
    for (auto i = map.size(); i > 0; i--) {
        count += map[i - 1];
        cf[i - 1] = count;
    }
    
    if (should_show) {
        std::cout << "Cummulative frequency of vector" << std::endl;
        for (size_t i = 0; i < cf.size(); i++)
            std::cout << "[" << i + 1 << "]: " << map[i] << std::endl;
    }
    
    result->reserve(cf.size());
    result->resize(0);
    std::transform(cf.begin(), cf.end(), std::back_inserter(*result), [](auto x) {return x;});
}

inline size_t additional_bit_size_of_rank(double n) {
    return (((n-1)/64+1) + ((n/512+1) * 2)) * 64; // about 1.25*n bits
}

template <class Container, typename T>
inline void split_positions_optimized_for_dac(const Container& list, std::vector<T>* result, const size_t max_levels = 8) {
    if (list.empty())
        return;
    
    std::vector<size_t> cf;
    cummulative_frequency_list(list, &cf);
    const auto m = cf.size() - 1;
    std::vector<size_t> s(m+1, 0), l(m+1, 0), b(m+1, 0);
    for (int t = m; t >= 0; --t) {
        auto min_size = std::numeric_limits<size_t>::max();
        auto min_pos = m;
        for (size_t i = t + 1; i <= m; i++) {
            auto current_size = s[i] + cf[t] * (i - t) + additional_bit_size_of_rank(cf[t]);
            if (current_size < min_size) {
                min_size = current_size;
                min_pos = i;
            }
        }
        if (min_size < cf[t] * ((m + 1) - t)) {
            s[t] = min_size;
            l[t] = l[min_pos] + 1;
            b[t] = min_pos - t;
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
    for (size_t k = 0; k <= L; k++) {
        bk[k] = b[t];
        t = t + b[t];
    }
    
    if (L > max_levels) { // TODO: Skeptical algorithm
        std::vector<bool> sep_pos(cf.size() + 1, false);
        auto p = 0;
        for (auto v : bk) {
            p += v;
            sep_pos[p] = true;
        }
        using P = std::pair<uint64_t, uint64_t>;
        auto cmp = [](P lhs, P rhs) {
            return lhs.first > rhs.first;
        };
        std::vector<P> pq;
        for (size_t i = 1; i < m; i++) {
            pq.push_back(P(cf[i - 1] - cf[i], i));
        }
        std::sort(pq.begin(), pq.end(), cmp);
        auto num_seps = L;
        while (num_seps > max_levels && !pq.empty()) {
            auto curP = pq.back();
            pq.pop_back();
            if (sep_pos[curP.second]) {
                sep_pos[curP.second] = false;
                num_seps--;
            }
        }
        
        bk.resize(0);
        auto last_p = 0;
        for (size_t i = 0; i < sep_pos.size(); i++) {
            if (sep_pos[i]) {
                bk.push_back(i - last_p);
                last_p = i;
            }
        }
    }

    result->reserve(bk.size());
    result->resize(0);
    std::transform(bk.begin(), bk.end(), std::back_inserter(*result), [](auto x) {return x;});
}

} // namespace sim_ds::calc

#endif /* calc_hpp */
