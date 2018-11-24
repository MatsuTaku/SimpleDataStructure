//
//  bit_tools.hpp
//  bench
//
//  Created by 松本拓真 on 2018/05/16.
//

#ifndef bit_tools_hpp
#define bit_tools_hpp

#include "basic.hpp"
#include <popcntintrin.h>

#define BITS_64 64
#define BITS_32 32

namespace sim_ds::bit_tools {
    
#ifndef USE_X86
constexpr size_t _bits_of_id_type = BITS_64;
constexpr id_type _mask_fill = 0xFFFFFFFFFFFFFFFF;
#else
constexpr size_t _bits_of_id_type = BITS_32;
constexpr id_type _mask_fill = 0xFFFFFFFF;
#endif

inline constexpr id_type maskOfBits(size_t bits) {
    assert(bits <= _bits_of_id_type);
    if (bits == 0) {
        return 0;
    } else {
        return _mask_fill >> (_bits_of_id_type - bits);
    }
}

template<size_t _Bits>
constexpr id_type bits_mask = maskOfBits(_Bits);

inline constexpr id_type maskOfOffset(size_t offset) {
    assert(offset < _bits_of_id_type);
#ifndef USE_X86
    return 1ULL << offset;
#else
    return 1UL << offset;
#endif
}

template<size_t _Off>
constexpr id_type offset_mask = maskOfOffset(_Off);

template <unsigned int TYPE_SIZE, unsigned int TYPE>
inline constexpr unsigned long long popCount(uint64_t x) {
    if constexpr (TYPE_SIZE == 1) {
        assert(TYPE < 0b10);
        if constexpr (TYPE == 0b0)
            x = (~x & 0x5555555555555555) + ((~x >> 1) & 0x5555555555555555);
        else if constexpr (TYPE == 0b1)
            x = (x & 0x5555555555555555) + ((x >> 1) & 0x5555555555555555);
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333); // 4:3
        x = (x & 0x0f0f0f0f0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f0f0f0f0f); // 8:4
        x *= 0x0101010101010101;
        return x >> 56;
        
    } else if constexpr (TYPE_SIZE == 2) {
        assert(TYPE < 0b100);
        if constexpr (TYPE == 0b00)
            x = ((~x >> 1) & 0x5555555555555555) + (~x & 0x5555555555555555);
        else if constexpr (TYPE == 0b01)
            x = ((~x >> 1) & 0x5555555555555555) + (x & 0x5555555555555555);
        else if constexpr (TYPE == 0b10)
            x = ((x >> 1) & 0x5555555555555555) + (~x & 0x5555555555555555);
        else if constexpr (TYPE == 0b11)
            x = ((x >> 1) & 0x5555555555555555) + (x & 0x5555555555555555);
        x = ((x >> 1) & 0x1111111111111111) + ((x >> 3) & 0x1111111111111111); // 4:2
        x = (x & 0x0f0f0f0f0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f0f0f0f0f); // 8:3
        x *= 0x0101010101010101;
        return x >> 56;
        
    } else if constexpr (TYPE_SIZE == 3) {
        assert(TYPE < 0b1000);
        if constexpr (TYPE == 0b000)
            x = (~x & 0x9249249249249249) + ((~x >> 1) & 0x9249249249249249) + ((~x >> 2) & 0x9249249249249249);
        else if constexpr (TYPE == 0b001)
            x = ((~x >> 2) & 0x9249249249249249) + ((~x >> 1) & 0x9249249249249249) + (x & 0x9249249249249249);
        else if constexpr (TYPE == 0b010)
            x = ((~x >> 2) & 0x9249249249249249) + ((x >> 1) & 0x9249249249249249) + (~x & 0x9249249249249249);
        else if constexpr (TYPE == 0b011)
            x = ((~x >> 2) & 0x9249249249249249) + ((x >> 1) & 0x9249249249249249) + (x & 0x9249249249249249);
        else if constexpr (TYPE == 0b100)
            x = ((x >> 2) & 0x9249249249249249) + ((~x >> 1) & 0x9249249249249249) + (~x & 0x9249249249249249);
        else if constexpr (TYPE == 0b101)
            x = ((x >> 2) & 0x9249249249249249) + ((~x >> 1) & 0x9249249249249249) + (x & 0x9249249249249249);
        else if constexpr (TYPE == 0b110)
            x = ((x >> 2) & 0x9249249249249249) + ((x >> 1) & 0x9249249249249249) + (~x & 0x9249249249249249);
        else if constexpr (TYPE == 0b111)
            x = ((x >> 2) & 0x9249249249249249) + ((x >> 1) & 0x9249249249249249) + (x & 0x9249249249249249);
        x = ((x & 0x9249249249249249) + ((x >> 1) & 0x9249249249249249)) >> 1;
        x = (x & 0x1041041041041041) + ((x >> 3) & 0x1041041041041041); // 6:2
        x = (x * 0x0041041041041041 >> 54) + (x >> 60);
        return x & 0x3F;
        
    } else {
        abort();
    }
}

const std::function<unsigned long long (uint64_t value)> _pop_cnt_table[3][8] = {
    {
        popCount<1, 0b0>,
        popCount<1, 0b1>
    },
    {
        popCount<2, 0b00>,
        popCount<2, 0b01>,
        popCount<2, 0b10>,
        popCount<2, 0b11>
    },
    {
        popCount<3, 0b000>,
        popCount<3, 0b001>,
        popCount<3, 0b010>,
        popCount<3, 0b011>,
        popCount<3, 0b100>,
        popCount<3, 0b101>,
        popCount<3, 0b110>,
        popCount<3, 0b111>
    }
};

template <unsigned int TYPE_SIZE>
inline constexpr unsigned long long popCount(size_t type, uint64_t x) {
    return _pop_cnt_table[TYPE_SIZE - 1][type](x);
}

inline unsigned long long popCount(uint64_t x) {
    return popCount<1, 1>(x);
}

// inspired by marisa-trie
constexpr uint8_t _select_table[9][256] = {
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

// Use for select
constexpr uint8_t selectTable(size_t word, size_t offset) {
    return _select_table[offset][word];
}
    
    
} // namespace sim_ds

#endif /* bit_tools_hpp */
