//
//  bit_tools.hpp
//  bench
//
//  Created by 松本拓真 on 2018/05/16.
//

#ifndef bit_tools_hpp
#define bit_tools_hpp

#include "basic.hpp"
#include "log.hpp"
#ifdef SIDS_USE_BOOST
#include <boost/multiprecision/cpp_int.hpp>
#endif
#ifdef __BMI2__
#include <immintrin.h>
#endif
#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif

namespace sim_ds::bit_util {

// MARK: - Mask

using mask_type = id_type;
constexpr size_t kMaxWidthOfMask = sizeof(mask_type) * 8;
constexpr mask_type kMaskFill = std::numeric_limits<mask_type>::max();

template <size_t Bits>
inline constexpr mask_type width_mask = (1ull << Bits) - 1;

inline constexpr mask_type WidthMask(size_t width) {
    assert(width <= kMaxWidthOfMask);
    return (1ull << width) - 1;
}

template <typename T>
inline T bits_extract_len(T x, size_t len) {return x & WidthMask(len);}

template <size_t Offset>
inline constexpr mask_type offset_mask = mask_type(1) << Offset;

inline constexpr mask_type OffsetMask(size_t offset) {
    assert(offset < kMaxWidthOfMask);
    return mask_type(1) << offset;
}

#ifdef SIDS_USE_BOOST
using boost::multiprecision::int256_t;
using boost::multiprecision::uint256_t;
using mask256_type = uint256_t;

template <unsigned Unit, unsigned Shift>
const mask256_type pattern_mask256;

// 0x5555555555555555555555555555555555555555555555555555555555555555
template<> const mask256_type pattern_mask256<1, 0> = mask256_type("38597363079105398474523661669562635951089994888546854679819194669304376546645");
// 0xAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
template<> const mask256_type pattern_mask256<1, 1> = mask256_type("77194726158210796949047323339125271902179989777093709359638389338608753093290");
// 0x3333333333333333333333333333333333333333333333333333333333333333
template<> const mask256_type pattern_mask256<2, 0> = mask256_type("23158417847463239084714197001737581570653996933128112807891516801582625927987");
// 0xCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
template<> const mask256_type pattern_mask256<2, 2> = mask256_type("92633671389852956338856788006950326282615987732512451231566067206330503711948");
// 0x0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F
template<> const mask256_type pattern_mask256<4, 0> = mask256_type("6811299366900952671974763824040465167839410862684739061144563765171360567055");
// 0xF0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0
template<> const mask256_type pattern_mask256<4, 4> = mask256_type("108980789870415242751596221184647442685430573802955824978313020242741769072880");
// 0x00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF
template<> const mask256_type pattern_mask256<8, 0> = mask256_type("450552876409790643671482431940419874915447411150352389258589821042463539455");
// 0xFF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
template<> const mask256_type pattern_mask256<8, 8> = mask256_type("115341536360906404779899502576747487978354537254490211650198994186870666100480");
// 0x0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF
template<> const mask256_type pattern_mask256<16, 0> = mask256_type("1766820105243087041267848467410591083712559083657179364930612997358944255");
// 0xFFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000
template<> const mask256_type pattern_mask256<16, 16> = mask256_type("115790322417210952336529717160220497262186272106556906860092653394915770695680");
// 0x00000000FFFFFFFF00000000FFFFFFFF00000000FFFFFFFF00000000FFFFFFFF
template<> const mask256_type pattern_mask256<32, 0> = mask256_type("26959946660873538060741835960174461801791452538186943042387869433855");
// 0xFFFFFFFF00000000FFFFFFFF00000000FFFFFFFF00000000FFFFFFFF00000000
template<> const mask256_type pattern_mask256<32, 32> = mask256_type("115792089210356248762697446947946071893095522863849111501270640965525260206080");
// 0x0000000000000000FFFFFFFFFFFFFFFF0000000000000000FFFFFFFFFFFFFFFF
template<> const mask256_type pattern_mask256<64, 0> = mask256_type("6277101735386680763495507056286727952657427581105975853055");
// 0xFFFFFFFFFFFFFFFF0000000000000000FFFFFFFFFFFFFFFF0000000000000000
template<> const mask256_type pattern_mask256<64, 64> = mask256_type("115792089237316195417293883273301227089774477609353836086800156426807153786880");
// 0x00000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
template<> const mask256_type pattern_mask256<128, 0> = mask256_type("340282366920938463463374607431768211455");
// 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000000000000000000000000000
template<> const mask256_type pattern_mask256<128, 128> = mask256_type("115792089237316195423570985008687907852929702298719625575994209400481361428480");
#endif


// MARK: - popcnt

constexpr uint8_t kLtCnt[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

inline constexpr uint8_t popcnt8(uint8_t x) {
    return kLtCnt[x];
}

inline constexpr uint16_t popcnt16(uint16_t x) {
    return kLtCnt[(x>>8)&0xFF] + kLtCnt[x&0xFF];
}

inline uint32_t popcnt32(uint32_t x) {
#ifdef __POPCNT__
    return _mm_popcnt_u32(x);
#else
    x = x-((x>>1) & 0x55555555ull);
    x = (x & 0x33333333ull) + ((x>>2) & 0x33333333ull);
    return (0x10101010*x >>28)+(0x01010101*x >>28);
#endif
}

inline uint64_t popcnt64(uint64_t x) {
#ifdef __POPCNT__
    return _mm_popcnt_u64(x);
#else
    x = x-((x>>1) & 0x5555555555555555ull);
    x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Full;
    return 0x0101010101010101ull*x >> 56;
#endif
}

#ifdef SIDS_USE_BOOST
inline int popcnt256(uint256_t x) {
    return (popcnt64(uint64_t(x))) +
            popcnt64(uint64_t(x>>64)) +
            popcnt64(uint64_t(x>>128)) +
            popcnt64(uint64_t(x>>192));
}
#endif

template <typename Word>
inline Word popcnt(Word x) {
    if constexpr (sizeof(Word) == 1)
        return popcnt8(x);
    else if constexpr (sizeof(Word) == 2)
        return popcnt16(x);
    else if constexpr (sizeof(Word) == 4)
        return popcnt32(x);
    else if constexpr (sizeof(Word) == 8)
        return popcnt64(x);
    else
        return 0;
}

template <typename Word>
inline Word cnt(Word x, size_t len) {
    return popcnt(bits_extract_len(x, len));
}

inline uint64_t popcnt11(uint64_t x) {
    x = (x&(x>>1)) & 0x5555555555555555ull;
#ifdef __POPCNT__
    return _mm_popcnt_u64(x);
#else
    x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Full;
    return (0x0101010101010101ull*x >> 56);
#endif
}

inline uint64_t popcnt10(uint64_t x) {
    x = (~x&(x>>1)) & 0x5555555555555555ull;
#ifdef __POPCNT__
    return _mm_popcnt_u64(x);
#else
    x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Full;
    return (0x0101010101010101ull*x >> 56);
#endif
}

inline uint64_t popcnt111(uint64_t x) {
    x = (x&(x>>1)&(x>>2)) & 0x1249249249249249ull;
#ifdef __POPCNT__
    return _mm_popcnt_u64(x);
#else
    x = (x + (x>>3)) & 0x30C30C30C30C30C3ull;
    return (((0x0041041041041041ull*x) >> 54) & 0x3F) + (x >> 60);
#endif
}

inline uint64_t popcnt110(uint64_t x) {
    x = (~x&(x>>1)&(x>>2)) & 0x1249249249249249ull;
#ifdef __POPCNT__
    return _mm_popcnt_u64(x);
#else
    x = (x + (x>>3)) & 0x30C30C30C30C30C3ull;
    return (((0x0041041041041041ull*x) >> 54) & 0x3F) + (x >> 60);
#endif
}

inline uint64_t popcnt101(uint64_t x) {
    x = (x&(~x>>1)&(x>>2)) & 0x1249249249249249ull;
#ifdef __POPCNT__
    return _mm_popcnt_u64(x);
#else
    x = (x + (x>>3)) & 0x30C30C30C30C30C3ull;
    return (((0x0041041041041041ull*x) >> 54) & 0x3F) + (x >> 60);
#endif
}

inline uint64_t popcnt100(uint64_t x) {
    x = (~x&(~x>>1)&(x>>2)) & 0x1249249249249249ull;
#ifdef __POPCNT__
    return _mm_popcnt_u64(x);
#else
    x = (x + (x>>3)) & 0x30C30C30C30C30C3ull;
    return (((0x0041041041041041ull*x) >> 54) & 0x3F) + (x >> 60);
#endif
}

// MARK: - select

constexpr uint8_t kLtSel[9][256] = {
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        8, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        8, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
        5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1
    },
    {
        8, 8, 8, 2, 8, 3, 3, 2, 8, 4, 4, 2, 4, 3, 3, 2,
        8, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
        8, 6, 6, 2, 6, 3, 3, 2, 6, 4, 4, 2, 4, 3, 3, 2,
        6, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
        8, 7, 7, 2, 7, 3, 3, 2, 7, 4, 4, 2, 4, 3, 3, 2,
        7, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
        7, 6, 6, 2, 6, 3, 3, 2, 6, 4, 4, 2, 4, 3, 3, 2,
        6, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
        8, 8, 8, 2, 8, 3, 3, 2, 8, 4, 4, 2, 4, 3, 3, 2,
        8, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
        8, 6, 6, 2, 6, 3, 3, 2, 6, 4, 4, 2, 4, 3, 3, 2,
        6, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
        8, 7, 7, 2, 7, 3, 3, 2, 7, 4, 4, 2, 4, 3, 3, 2,
        7, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
        7, 6, 6, 2, 6, 3, 3, 2, 6, 4, 4, 2, 4, 3, 3, 2,
        6, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2
    },
    {
        8, 8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 4, 8, 4, 4, 3,
        8, 8, 8, 5, 8, 5, 5, 3, 8, 5, 5, 4, 5, 4, 4, 3,
        8, 8, 8, 6, 8, 6, 6, 3, 8, 6, 6, 4, 6, 4, 4, 3,
        8, 6, 6, 5, 6, 5, 5, 3, 6, 5, 5, 4, 5, 4, 4, 3,
        8, 8, 8, 7, 8, 7, 7, 3, 8, 7, 7, 4, 7, 4, 4, 3,
        8, 7, 7, 5, 7, 5, 5, 3, 7, 5, 5, 4, 5, 4, 4, 3,
        8, 7, 7, 6, 7, 6, 6, 3, 7, 6, 6, 4, 6, 4, 4, 3,
        7, 6, 6, 5, 6, 5, 5, 3, 6, 5, 5, 4, 5, 4, 4, 3,
        8, 8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 4, 8, 4, 4, 3,
        8, 8, 8, 5, 8, 5, 5, 3, 8, 5, 5, 4, 5, 4, 4, 3,
        8, 8, 8, 6, 8, 6, 6, 3, 8, 6, 6, 4, 6, 4, 4, 3,
        8, 6, 6, 5, 6, 5, 5, 3, 6, 5, 5, 4, 5, 4, 4, 3,
        8, 8, 8, 7, 8, 7, 7, 3, 8, 7, 7, 4, 7, 4, 4, 3,
        8, 7, 7, 5, 7, 5, 5, 3, 7, 5, 5, 4, 5, 4, 4, 3,
        8, 7, 7, 6, 7, 6, 6, 3, 7, 6, 6, 4, 6, 4, 4, 3,
        7, 6, 6, 5, 6, 5, 5, 3, 6, 5, 5, 4, 5, 4, 4, 3
    },
    {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
        8, 8, 8, 8, 8, 8, 8, 5, 8, 8, 8, 5, 8, 5, 5, 4,
        8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 4,
        8, 8, 8, 6, 8, 6, 6, 5, 8, 6, 6, 5, 6, 5, 5, 4,
        8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 4,
        8, 8, 8, 7, 8, 7, 7, 5, 8, 7, 7, 5, 7, 5, 5, 4,
        8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 4,
        8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
        8, 8, 8, 8, 8, 8, 8, 5, 8, 8, 8, 5, 8, 5, 5, 4,
        8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 4,
        8, 8, 8, 6, 8, 6, 6, 5, 8, 6, 6, 5, 6, 5, 5, 4,
        8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 4,
        8, 8, 8, 7, 8, 7, 7, 5, 8, 7, 7, 5, 7, 5, 5, 4,
        8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 4,
        8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4
    },
    {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
        8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 5,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
        8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 5,
        8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6,
        8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 5,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
        8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 5,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
        8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 5,
        8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6,
        8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 5
    },
    {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
        8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
        8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6
    },
    {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7
    },
    {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
    }
};

/* Select operation for 1 index at byte */
inline uint8_t sel(uint64_t x, size_t i) {
#ifdef __BMI2__
    return __builtin_ctzll(_pdep_u64(1ull << (i-1), x)) + 1; // for 1 index
#else
    size_t ret = 0;
    auto bit_cnt = bit_util::popcnt32(x);
    if (bit_cnt < i) {
        x >>= 32;
        ret += 32;
        i -= bit_cnt;
    }
    bit_cnt = bit_util::popcnt16(x);
    if (bit_cnt < i) {
        x >>= 16;
        ret += 16;
        i -= bit_cnt;
    }
    bit_cnt = bit_util::popcnt8(x);
    if (bit_cnt < i) {
        x >>= 8;
        ret += 8;
        i -= bit_cnt;
    }
    
    return ret + kLtSel[i][x & 0xFF];
#endif
}
    
/* Bit field extract */
inline uint64_t bextr(uint64_t x, size_t start, size_t len) {
#ifdef __BMI__
    return _bextr_u64(x, start, len);
#else
    return bits_extract_len(x >> start, len);
#endif
}


// MARK: - ctz/clz

template <typename T>
inline constexpr int ctz(T x) {
#ifdef __GNUC__
    return std::__ctz(x);
#else
    return popcnt((x & -x) - 1);
#endif
}

#ifdef SIDS_USE_BOOST
inline int ctz256(uint256_t x) {
    return popcnt256(uint256_t((x & -int256_t(x)) - 1));
}
#endif

template <typename T>
inline constexpr int clz(T x) {
#ifdef __GNUC__
    return std::__clz(x);
#else
#ifdef __POPCNT__
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return popcnt(~x);
#else
    if (x == 0)
        return 64;
    int c = 0;
    if (x & 0xFFFFFFFF00000000) {
        x &= 0xFFFFFFFF00000000;
        c |= 32;
    }
    if (x & 0xFFFF0000FFFF0000) {
        x &= 0xFFFF0000FFFF0000;
        c |= 16;
    }
    if (x & 0xFF00FF00FF00FF00) {
        x &= 0xFF00FF00FF00FF00;
        c |= 8;
    }
    if (x & 0xF0F0F0F0F0F0F0F0) {
        x &= 0xF0F0F0F0F0F0F0F0;
        c |= 4;
    }
    if (x & 0xCCCCCCCCCCCCCCCC) {
        x &= 0xCCCCCCCCCCCCCCCC;
        c |= 2;
    }
    if (x & 0xAAAAAAAAAAAAAAAA) {
        x &= 0xAAAAAAAAAAAAAAAA;
        c |= 1;
    }
    return c ^ 63;
#endif
#endif
}

#ifdef SIDS_USE_BOOST
inline int clz256(mask256_type x) {
#ifdef __POPCNT__
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x |= x >> 64;
    x |= x >> 128;
    return popcnt256(~x);
#else
    if (x == 0)
        return 256;
    int c = 0;
    if (x & pattern_mask256<128, 128>) {
        x &= pattern_mask256<128, 128>;
        c |= 128;
    }
    if (x & pattern_mask256<64, 64>) {
        x &= pattern_mask256<64, 64>;
        c |= 64;
    }
    if (x & pattern_mask256<32, 32>) {
        x &= pattern_mask256<32, 32>;
        c |= 32;
    }
    if (x & pattern_mask256<16, 16>) {
        x &= pattern_mask256<16, 16>;
        c |= 16;
    }
    if (x & pattern_mask256<8, 8>) {
        x &= pattern_mask256<8, 8>;
        c |= 8;
    }
    if (x & pattern_mask256<4, 4>) {
        x &= pattern_mask256<4, 4>;
        c |= 4;
    }
    if (x & pattern_mask256<2, 2>) {
        x &= pattern_mask256<2, 2>;
        c |= 2;
    }
    if (x & pattern_mask256<1, 1>) {
        x &= pattern_mask256<1, 1>;
        c |= 1;
    }
    return c ^ 255;
#endif
}
#endif


// MARK: - xor_idx

#ifdef SIDS_USE_BOOST
inline mask256_type xor_idx(mask256_type x, uint8_t y) {
    if (y & 1)
        x = (((x&pattern_mask256<1, 0>) << 1) | ((x&pattern_mask256<1, 1>) >> 1));
    if (y & 2)
        x = (((x&pattern_mask256<2, 0>) << 2) | ((x&pattern_mask256<2, 2>) >> 2));
    if (y & 4)
        x = (((x&pattern_mask256<4, 0>) << 4) | ((x&pattern_mask256<4, 4>) >> 4));
    if (y & 8)
        x = (((x&pattern_mask256<8, 0>) << 8) | ((x&pattern_mask256<8, 8>) >> 8));
    if (y & 16)
        x = (((x&pattern_mask256<16, 0>) << 16) | ((x&pattern_mask256<16, 16>) >> 16));
    if (y & 32)
        x = (((x&pattern_mask256<32, 0>) << 32) | ((x&pattern_mask256<32, 32>) >> 32));
    if (y & 64)
        x = (((x&pattern_mask256<64, 0>) << 64) | ((x&pattern_mask256<64, 64>) >> 64));
    if (y & 128)
        x = (((x&pattern_mask256<128, 0>) << 128) | ((x&pattern_mask256<128, 128>) >> 128));
    return x;
}
#endif


} // namespace sim_ds

#endif /* bit_tools_hpp */
