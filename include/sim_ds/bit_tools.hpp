//
//  bit_tools.hpp
//  bench
//
//  Created by 松本拓真 on 2018/05/16.
//

#ifndef bit_tools_hpp
#define bit_tools_hpp

#include <popcntintrin.h>
#include <boost/function.hpp>

namespace sim_ds::bit_tools {
    
    template <unsigned int TYPE_SIZE, unsigned int TYPE>
    inline constexpr unsigned long long popCount(uint64_t x) {
        if constexpr (TYPE_SIZE == 1)
        {
            assert(TYPE < 0b10);
            if constexpr (TYPE == 0b0)
                x = (~x & 0x5555555555555555) + ((~x >> 1) & 0x5555555555555555);
            else if constexpr (TYPE == 0b1)
                x = (x & 0x5555555555555555) + ((x >> 1) & 0x5555555555555555);
            x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333); // 4:3
            x = (x & 0x0f0f0f0f0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f0f0f0f0f); // 8:4
            x *= 0x0101010101010101;
            return x >> 56;
        }
        else if constexpr (TYPE_SIZE == 2)
        {
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
        }
        else if constexpr (TYPE_SIZE == 3)
        {
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
    
    const boost::function<unsigned long long (uint64_t value)> popCount_l[3][8] = {
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
        return popCount_l[TYPE_SIZE - 1][type](x);
    }
    
    inline constexpr unsigned long long popCount(uint64_t x) {
        return popCount<1, 1>(x);
    }
    
}

#endif /* bit_tools_hpp */
