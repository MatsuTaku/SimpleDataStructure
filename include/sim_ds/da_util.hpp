//
//  da_util.hpp
//  dynamic
//
//  Created by 松本拓真 on 2019/07/01.
//

#ifndef da_util_hpp
#define da_util_hpp

#include "bit_util256.hpp"

#if defined(__AVX2__)
#define BLOCK_UTIL_BUILTIN
#endif

namespace sim_ds::da_util {

#ifdef BLOCK_UTIL_BUILTIN
int _xcheck_in_da_block_builtin(const uint64_t* unit_field, const std::vector<uint8_t>& cs, __m256i temp) {
    __m256i x = _mm256_load_si256(reinterpret_cast<const __m256i*>(unit_field));
    for (uint8_t c : cs) {
        temp = _mm256_and_si256(temp, bit_util::xor_map_intrinsics(x, c));
        if (bit_util::is_zero256_intrinsics(temp))
            return 256;
    }
    alignas(32) uint64_t dst[4];
    _mm256_store_si256(reinterpret_cast<__m256i*>(dst), temp);
    return bit_util::ctz256(dst);
}
#endif

int _xcheck_in_da_block(const uint64_t* unit_field, const std::vector<uint8_t>& cs, uint64_t temp[4]) {
  for (uint8_t mask : cs) {
    uint64_t mapped[4];
    bit_util::xor_map(unit_field, mask, mapped);
    for (int i = 0; i < 4; i++) {
      temp[i] &= mapped[i];
    }
    if (bit_util::is_zero256(temp))
      return 256;
  }
  return bit_util::ctz256(temp);
}

int xcheck_in_da_block(const uint64_t* unit_field, const std::vector<uint8_t>& cs) {
#ifdef BLOCK_UTIL_BUILTIN
  return _xcheck_in_da_block_builtin(unit_field, cs, _mm256_set1_epi64x(0xFFFFFFFFFFFFFFFF));
#else
  alignas(32) uint64_t temp[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
  return _xcheck_in_da_block(unit_field, cs, temp);
#endif
}

int xcheck_in_da_block(const uint64_t* unit_field, const std::vector<uint8_t>& cs, const uint64_t* base_field) {
#ifdef BLOCK_UTIL_BUILTIN
  return _xcheck_in_da_block_builtin(unit_field, cs, _mm256_load_si256(reinterpret_cast<const __m256i*>(base_field)));
#else
  alignas(32) uint64_t temp[4] = {*(base_field+0), *(base_field+1), *(base_field+2), *(base_field+3)};
  return _xcheck_in_da_block(unit_field, cs, temp);
#endif
}

} // namespace sim_ds::da_util

#endif /* da_util_hpp */
