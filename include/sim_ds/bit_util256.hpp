//
//  bit_util256.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/06/11.
//

#ifndef bit_util256_hpp
#define bit_util256_hpp

#include "bit_util.hpp"

//#include <boost/multiprecision/cpp_int.hpp>

namespace sim_ds {
namespace bit_util {


// MARK: - popcnt

inline int popcnt256(const uint64_t* x_addr) {
    int cnt = 0;
    for (int i = 0; i < 4; i++)
        cnt += popcnt64(*(x_addr+i));
    return cnt;
}


// MARK: - ctz/clz

inline int ctz256(const uint64_t* x_addr) {
    for (int i = 0; i < 4; i++) if (*(x_addr+i))
        return 64 * i + ctz(*(x_addr+i));
    return 256;
}

inline int clz256(const uint64_t* x_addr) {
    for (int i = 3; i >= 0; i--) if (*(x_addr+i))
        return 64 * i + clz(*(x_addr+i));
    return 256;
}


// MARK: - xor_idx

// Input addres alignment must be 32 bytes only if using intrinsics.
inline void xor_idx256(const uint64_t* x_addr, uint8_t y, uint64_t* dst_addr) {
#if defined(__AVX2__)
#if defined(__AVX512VL__) && defined(__AVX512F__)
    __m256i xx = _mm256_load_epi64(dst_addr);
#else
    __m256i xx = _mm256_load_si256(reinterpret_cast<__m256i*>(dst_addr));
#endif
    if (y & 1) {
        auto xxl = _mm256_slli_epi64(_mm256_and_si256(xx, _mm256_set1_epi64x(0x5555555555555555)), 1);
        auto xxr = _mm256_srli_epi64(_mm256_and_si256(xx, _mm256_set1_epi64x(0xAAAAAAAAAAAAAAAA)), 1);
        xx = _mm256_or_si256(xxl, xxr);
    }
    if (y & 2) {
        auto xxl = _mm256_slli_epi64(_mm256_and_si256(xx, _mm256_set1_epi64x(0x3333333333333333)), 2);
        auto xxr = _mm256_srli_epi64(_mm256_and_si256(xx, _mm256_set1_epi64x(0xCCCCCCCCCCCCCCCC)), 2);
        xx = _mm256_or_si256(xxl, xxr);
    }
    if (y & 4) {
        auto xxl = _mm256_slli_epi64(_mm256_and_si256(xx, _mm256_set1_epi64x(0x0F0F0F0F0F0F0F0F)), 4);
        auto xxr = _mm256_srli_epi64(_mm256_and_si256(xx, _mm256_set1_epi64x(0xF0F0F0F0F0F0F0F0)), 4);
        xx = _mm256_or_si256(xxl, xxr);
    }
#if defined(__AVX512VL__) && defined(__AVX512F__)
    if (y & 8)
        xx = _mm256_permutexvar_epi8(_mm256_set_epi8(30, 31, 28, 29, 26, 27, 24, 25, 22, 23, 20, 21, 18, 19, 16, 17, 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1), xx);
    if (y & 16)
        xx = _mm256_permutexvar_epi16(_mm256_set_epi16(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1), xx);
    if (y & 32)
        xx = _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), xx);
    if (y & 64)
        xx = _mm256_permutexvar_epi64(_mm256_set_epi64(2, 3, 0, 1), xx);
    if (y & 128)
        xx = _mm256_permutexvar_epi128(_mm256_set_epi128(0, 1), xx);
    _mm256_store_epi64(dst_addr, xx);
#else
    if (y & 8)
        xx = _mm256_or_si256(_mm256_srli_epi16(xx, 8), _mm256_slli_epi16(xx, 8));
    if (y & 16)
        xx = _mm256_or_si256(_mm256_srli_epi32(xx, 16), _mm256_slli_epi32(xx, 16));
    if (y & 32)
        xx = _mm256_or_si256(_mm256_srli_epi64(xx, 32), _mm256_slli_epi64(xx, 32));
    if (y & 64)
        xx = _mm256_or_si256(_mm256_bsrli_epi128(xx, 8), _mm256_bslli_epi128(xx, 8));
    if (y & 128)
        xx = _mm256_or_si256(_mm256_srli_si256(xx, 16), _mm256_slli_si256(xx, 16));
    _mm256_store_si256(reinterpret_cast<__m256i*>(dst_addr), xx);
#endif
    
#else
    for (int i = 0; i < 4; i++)
        *(dst_addr+i) = *(x_addr+i);
    auto for_each_word =[&](auto act) {
        for (int i = 0; i < 4; i++)
            act(*(dst_addr+i));
    };
    if (y & 1) {
        for_each_word([](auto& x) {
            x = ((x & 0x5555555555555555) << 1) | ((x & 0xAAAAAAAAAAAAAAAA) >> 1);
        });
    }
    if (y & 2) {
        for_each_word([](auto& x) {
            x = ((x & 0x3333333333333333) << 2) | ((x & 0xCCCCCCCCCCCCCCCC) >> 2);
        });
    }
    if (y & 4) {
        for_each_word([](auto& x) {
            x = ((x & 0x0F0F0F0F0F0F0F0F) << 4) | ((x & 0xF0F0F0F0F0F0F0F0) >> 4);
        });
    }
    if (y & 8) {
        for_each_word([](auto& x) {
            x = ((x & 0x00FF00FF00FF00FF) << 8) | ((x & 0xFF00FF00FF00FF00) >> 8);
        });
    }
    if (y & 16) {
        for_each_word([](auto& x) {
            x = ((x & 0x0000FFFF0000FFFF) << 16) | ((x & 0xFFFF0000FFFF0000) >> 16);
        });
    }
    if (y & 32) {
        for_each_word([](auto& x) {
            x = (x << 32) | (x >> 32);
        });
    }
    if (y & 64) {
        std::swap(*(dst_addr), *(dst_addr+1));
        std::swap(*(dst_addr+2), *(dst_addr+3));
    }
    if (y & 128) {
        std::swap(*(dst_addr), *(dst_addr+2));
        std::swap(*(dst_addr+1), *(dst_addr+3));
    }
#endif
}

} // namespace bit_util
} // namespace sim_ds

#endif /* bit_util256_hpp */
