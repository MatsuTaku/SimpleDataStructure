//
//  da_util.hpp
//  dynamic
//
//  Created by 松本拓真 on 2019/07/01.
//

#ifndef da_util_hpp
#define da_util_hpp

#include "bit_util256.hpp"

namespace sim_ds {
namespace da_util {

#if defined(__AVX2__)
#define BLOCK_UTIL_BUILTIN
#endif

int xcheck_in_da_block(const uint64_t* field, std::vector<uint8_t> cs) {
#ifdef BLOCK_UTIL_BUILTIN
    __m256i x = _mm256_load_si256(reinterpret_cast<const __m256i*>(field));
    __m256i temp = _mm256_set1_epi64x(uint64_t(-1));
    for (uint8_t c : cs) {
        temp = _mm256_and_si256(temp, bit_util::xor_map_intrinsics(x, c));
        if (bit_util::is_zero256_intrinsics(temp))
            return 256;
    }
    alignas(32) uint64_t dst[4];
    _mm256_store_si256(reinterpret_cast<__m256i*>(dst), temp);
    return bit_util::ctz256(dst);
#else
    alignas(32) uint64_t dst[4] = {uint64_t(-1), uint64_t(-1), uint64_t(-1), uint64_t(-1)};
    for (uint8_t mask : cs) {
        uint64_t temp[4];
        bit_util::xor_map(field, mask, temp);
        for (int i = 0; i < 4; i++) {
            dst[i] &= temp[i];
        }
        if (bit_util::is_zero256(dst))
            return 256;
    }
    return bit_util::ctz256(dst);
#endif
}

} // namespace da_util
} // namespace sim_ds

#endif /* da_util_hpp */
