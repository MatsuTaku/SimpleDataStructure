//
//  bit_util256.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/06/11.
//

#ifndef bit_util256_hpp
#define bit_util256_hpp

#include "bit_util.hpp"

namespace sim_ds::bit_util {


// MARK: - set

inline void set256_epi1(bool bit, uint64_t* x_addr) {
#ifdef __AVX2__
    uint64_t mask = bit ? uint64_t(-1) : uint64_t(0);
    _mm256_store_si256(reinterpret_cast<__m256i*>(x_addr), _mm256_set1_epi64x(mask));
#else
    uint64_t mask = bit ? uint64_t(-1) : uint64_t(0);
    for (int i = 0; i < 4; i++)
        *(x_addr+i) = mask;
#endif
}


// MARK: - copy

inline void copy256_epi64(const uint64_t* x_addr, uint64_t* dst_addr) {
#ifdef __AVX2__
    _mm256_store_si256(reinterpret_cast<__m256i*>(dst_addr), _mm256_load_si256(reinterpret_cast<const __m256i*>(x_addr)));
#else
    for (int i = 0; i < 4; i++)
        *(dst_addr+i) = *(x_addr+i);
#endif
}

// MARK: - equal

inline bool is_zero256(const uint64_t* x_addr) {
    for (int i = 0; i < 4; i++) {
        if (*(x_addr+i) != 0)
            return false;
    }
    return true;
}
    
#ifdef __AVX2__
inline bool is_zero256_intrinsics(__m256i x) {
    int iszero_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi64(x, _mm256_set1_epi64x(0)));
    return iszero_mask == (int)0xFFFFFFFF;
}
#endif


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


// MARK: - xor_map

void xor_map(const uint64_t* x_addr, uint8_t mask, uint64_t* dst_addr) {
    for (int i = 0; i < 4; i++)
        *(dst_addr+i) = *(x_addr+i);
    if (mask & 1) {
        for (int i = 0; i < 4; i++) {
            auto& x = *(dst_addr+i);
            x = ((x & 0x5555555555555555) << 1) | ((x & 0xAAAAAAAAAAAAAAAA) >> 1);
        }
    }
    if (mask & 2) {
        for (int i = 0; i < 4; i++) {
            auto& x = *(dst_addr+i);
            x = ((x & 0x3333333333333333) << 2) | ((x & 0xCCCCCCCCCCCCCCCC) >> 2);
        }
    }
    if (mask & 4) {
        for (int i = 0; i < 4; i++) {
            auto& x = *(dst_addr+i);
            x = ((x & 0x0F0F0F0F0F0F0F0F) << 4) | ((x & 0xF0F0F0F0F0F0F0F0) >> 4);
        }
    }
    if (mask & 8) {
        for (int i = 0; i < 4; i++) {
            auto& x = *(dst_addr+i);
            x = ((x & 0x00FF00FF00FF00FF) << 8) | ((x & 0xFF00FF00FF00FF00) >> 8);
        }
    }
    if (mask & 16) {
        for (int i = 0; i < 4; i++) {
            auto& x = *(dst_addr+i);
            x = ((x & 0x0000FFFF0000FFFF) << 16) | ((x & 0xFFFF0000FFFF0000) >> 16);
        }
    }
    if (mask & 32) {
        for (int i = 0; i < 4; i++) {
            auto& x = *(dst_addr+i);
            x = (x << 32) | (x >> 32);
        }
    }
    if (mask & 64) {
        std::swap(*(dst_addr+0), *(dst_addr+1));
        std::swap(*(dst_addr+2), *(dst_addr+3));
    }
    if (mask & 128) {
        std::swap(*(dst_addr+0), *(dst_addr+2));
        std::swap(*(dst_addr+1), *(dst_addr+3));
    }
}

#if defined(__AVX2__)
__m256i xor_map_intrinsics(__m256i x, uint8_t mask) {
    if (mask & 1) {
        x = _mm256_or_si256(_mm256_slli_epi64(_mm256_and_si256(x, _mm256_set1_epi64x(0x5555555555555555)), 1),
                            _mm256_srli_epi64(_mm256_and_si256(x, _mm256_set1_epi64x(0xAAAAAAAAAAAAAAAA)), 1));
    }
    if (mask & 2) {
        x = _mm256_or_si256(_mm256_slli_epi64(_mm256_and_si256(x, _mm256_set1_epi64x(0x3333333333333333)), 2),
                            _mm256_srli_epi64(_mm256_and_si256(x, _mm256_set1_epi64x(0xCCCCCCCCCCCCCCCC)), 2));
    }
    if (mask & 4) {
        x = _mm256_or_si256(_mm256_slli_epi64(_mm256_and_si256(x, _mm256_set1_epi64x(0x0F0F0F0F0F0F0F0F)), 4),
                            _mm256_srli_epi64(_mm256_and_si256(x, _mm256_set1_epi64x(0xF0F0F0F0F0F0F0F0)), 4));
    }
#if defined(__AVX512VL__) && defined(__AVX512F__)
    if (mask & 8)
        x = _mm256_permutexvar_epi8(_mm256_set_epi8(30, 31, 28, 29, 26, 27, 24, 25, 22, 23, 20, 21, 18, 19, 16, 17, 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1), x);
    if (mask & 16)
        x = _mm256_permutexvar_epi16(_mm256_set_epi16(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1), x);
    if (mask & 32)
        x = _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), x);
    if (mask & 64)
        x = _mm256_permutexvar_epi64(_mm256_set_epi64(2, 3, 0, 1), x);
    if (mask & 128)
        x = _mm256_permutexvar_epi128(_mm256_set_epi128(0, 1), x);
#else
    if (mask & 8)
        x = _mm256_or_si256(_mm256_srli_epi16(x, 8), _mm256_slli_epi16(x, 8));
    if (mask & 16)
        x = _mm256_or_si256(_mm256_srli_epi32(x, 16), _mm256_slli_epi32(x, 16));
    if (mask & 32)
        x = _mm256_permutevar8x32_epi32(x, _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1));
    if (mask & 64)
        x = _mm256_permute4x64_epi64(x, 0b10110001);
    if (mask & 128)
        x = _mm256_permute2x128_si256(x, x, 0x21);
#endif
    return x;
}
#endif

} // namespace sim_ds::bit_util

#endif /* bit_util256_hpp */
