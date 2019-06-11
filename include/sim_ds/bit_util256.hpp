//
//  bit_util256.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/06/11.
//

#ifndef bit_util256_hpp
#define bit_util256_hpp

#include "bit_util.hpp"

#include <boost/multiprecision/cpp_int.hpp>

namespace sim_ds {

using boost::multiprecision::int256_t;
using boost::multiprecision::uint256_t;
using mask256_type = uint256_t;


// MARK: - Mask

template <unsigned Bits>
inline const mask256_type width_mask256 = (mask256_type(1) << Bits) - 1;

inline mask256_type WidthMask256(size_t width) {return (mask256_type(1) << width) - 1;}


template <unsigned Bits>
inline const mask256_type offset_mask256 = mask256_type(1) << Bits;

inline mask256_type OffsetMask256(size_t offset) {return mask256_type(1) << offset;}


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


// MARK: - popcnt

inline int popcnt256(uint256_t x) {
    return (popcnt64(uint64_t(x))) +
            popcnt64(uint64_t(x>>64)) +
            popcnt64(uint64_t(x>>128)) +
            popcnt64(uint64_t(x>>192));
}


// MARK: - ctz/clz

inline int ctz256(uint256_t x) {
#ifdef __BMI__
    if (x % uint256_t(1) << 64)
        return ctz(uint64_t(x));
    if ((x >> 64) % uint256_t(1) << 64)
        return 64 + ctz(uint64_t(x >> 64));
    if ((x >> 128) % uint256_t(1) << 64)
        return 128 + ctz(uint64_t(x >> 128));
    return 192 + ctz(uint64_t(x));
#else
    return popcnt256(uint256_t((x & -int256_t(x)) - 1));
#endif
}

inline int clz256(mask256_type x) {
#ifdef __LZCNT__
    if (x >> 192)
        return clz(uint64_t(x >> 192));
    if (x >> 128)
        return 64 + clz(uint64_t(x >> 128));
    if (x >> 64)
        return 128 + clz(uint64_t(x >> 64));
    return 192 + clz(uint64_t(x));
#elif defined(__POPCNT__)
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


// MARK: - xor_idx

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
        x = ((x << 128) | (x >> 128));
    return x;
}

}

#endif /* bit_util256_hpp */
