//
//  bit_tools.hpp
//  bench
//
//  Created by 松本拓真 on 2018/05/16.
//

#ifndef bit_tools_hpp
#define bit_tools_hpp

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#include "basic.hpp"
#include "log.hpp"

namespace sim_ds::bit_util {

// MARK: - Mask

using mask_type = id_type;
constexpr size_t kMaxWidthOfMask = sizeof(mask_type) * 8;
constexpr mask_type kMaskFill = std::numeric_limits<mask_type>::max();

template <size_t Bits>
inline constexpr mask_type width_mask = (1ull << Bits) - 1;
template <>
inline constexpr mask_type width_mask<64> = kMaskFill;

inline constexpr mask_type WidthMask(size_t width) {
    assert(width <= kMaxWidthOfMask);
    return (1ull << width) - 1;
}

template <size_t Offset>
inline constexpr mask_type offset_mask = mask_type(1) << Offset;

inline constexpr mask_type OffsetMask(size_t offset) {
    assert(offset < kMaxWidthOfMask);
    return mask_type(1) << offset;
}


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
    return popcnt(x & WidthMask(len));
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


// MARK: - bextr

/* Bit field extract */
inline uint64_t bextr(uint64_t x, size_t start, size_t len) {
#ifdef __BMI__
    return _bextr_u64(x, start, len);
#else
    return (x >> start) & WidthMask(len);
#endif
}


// MARK: - ctz/clz

inline int ctz(uint32_t x) {
#ifdef __BMI__
    return _tzcnt_u32(x);
#elif defined(__POPCNT__)
  return _mm_popcnt_u32((x & -x) - 1);
#else
	// Arrange of nlz algorithm in http://www.nminoru.jp/~nminoru/programming/bitcount.html#leading-0bits
	if (x == 0)
		return 32;
	unsigned long long c = 0;
	if (x & 0x0000FFFF) {
		x &= 0x0000FFFF;
		c |= 16;
	}
	if (x & 0x00FF00FF) {
		x &= 0x00FF00FF;
		c |= 8;
	}
	if (x & 0x0F0F0F0F) {
		x &= 0x0F0F0F0F;
		c |= 4;
	}
	if (x & 0x33333333) {
		x &= 0x33333333;
		c |= 2;
	}
	if (x & 0x55555555) {
		c |= 1;
	}
	return c ^ 31;
#endif
}

inline int ctz(uint64_t x) {
#ifdef __BMI__
    return _tzcnt_u64(x);
#elif defined(__POPCNT__)
  return _mm_popcnt_u64((x & -x) - 1);
#else
	// Arrange of nlz algorithm in http://www.nminoru.jp/~nminoru/programming/bitcount.html#leading-0bits
	if (x == 0)
		return 64;
	unsigned long long c = 0;
	if (x & 0x00000000FFFFFFFF) {
		x &= 0x00000000FFFFFFFF;
		c |= 32;
	}
	if (x & 0x0000FFFF0000FFFF) {
		x &= 0x0000FFFF0000FFFF;
		c |= 16;
	}
	if (x & 0x00FF00FF00FF00FF) {
		x &= 0x00FF00FF00FF00FF;
		c |= 8;
	}
	if (x & 0x0F0F0F0F0F0F0F0F) {
		x &= 0x0F0F0F0F0F0F0F0F;
		c |= 4;
	}
	if (x & 0x3333333333333333) {
		x &= 0x3333333333333333;
		c |= 2;
	}
	if (x & 0x5555555555555555) {
		c |= 1;
	}
	return c ^ 63;
#endif
}

inline int clz(uint32_t x) {
#ifdef __LZCNT__
    return _lzcnt_u32(x);
#elif defined(__POPCNT__)
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return popcnt32(~x);
#else
    if (x == 0)
        return 32;
    int c = 0;
    if (x & 0xFFFF0000) {
        x &= 0xFFFF0000;
        c |= 16;
    }
    if (x & 0xFF00FF00) {
        x &= 0xFF00FF00;
        c |= 8;
    }
    if (x & 0xF0F0F0F0) {
        x &= 0xF0F0F0F0;
        c |= 4;
    }
    if (x & 0xCCCCCCCC) {
        x &= 0xCCCCCCCC;
        c |= 2;
    }
    if (x & 0xAAAAAAAA) {
        c |= 1;
    }
    return c ^ 63;
#endif
}
    
inline int clz(uint64_t x) {
#ifdef __LZCNT__
    return _lzcnt_u64(x);
#elif defined(__POPCNT__)
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return popcnt64(~x);
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
        c |= 1;
    }
    return c ^ 63;
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
inline int sel(uint64_t x, size_t i) {
#ifdef __BMI2__
    return ctz(_pdep_u64(1ull << (i-1), x)) + 1; // for 1 index
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


// MARK: - swap

inline uint64_t swap_pi1(uint64_t x) {
    return ((x & 0xAAAAAAAAAAAAAAAA) >> 1) | ((x & 0x5555555555555555) << 1);
}

inline uint64_t swap_pi2(uint64_t x) {
    return ((x & 0xCCCCCCCCCCCCCCCC) >> 2) | ((x & 0x3333333333333333) << 2);
}

inline uint64_t swap_pi4(uint64_t x) {
    return ((x & 0xF0F0F0F0F0F0F0F0) >> 4) | ((x & 0x0F0F0F0F0F0F0F0F) << 4);
}

inline uint64_t swap_pi8(uint64_t x) {
#ifdef __MMX__
    __m64 xx = (__m64) x;
    return (uint64_t) _mm_or_si64(_mm_slli_pi16(xx, 8), _mm_srli_pi16(xx, 8));
#else
    return ((x & 0xFF00FF00FF00FF00) >> 8) | ((x & 0x00FF00FF00FF00FF) << 8);
#endif
}

inline uint64_t swap_pi16(uint64_t x) {
#ifdef __MMX__
    __m64 xx = (__m64) x;
    return (uint64_t) _mm_or_si64(_mm_slli_pi32(xx, 16), _mm_srli_pi32(xx, 16));
#else
    return ((x & 0xFFFF0000FFFF0000) >> 16) | ((x & 0x0000FFFF0000FFFF) << 16);
#endif
}

inline uint64_t swap_pi32(uint64_t x) {
    return (x >> 32) | (x << 32);
}


} // namespace sim_ds

#endif /* bit_tools_hpp */
