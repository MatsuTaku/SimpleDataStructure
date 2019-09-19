//
// Created by 松本拓真 on 2019-08-10.
//

#ifndef DOUBLEARRAYTRIEDICT_HPP_
#define DOUBLEARRAYTRIEDICT_HPP_

#include "DoubleArrayMpTrie.hpp"
#include "DoubleArrayPatriciaTrie.hpp"

namespace sim_ds {

using string_set32 = DoubleArrayPatriciaTrie<void, uint32_t, false, true, 1, false>;

using string_set64 = DoubleArrayPatriciaTrie<void, uint64_t, false, true, 1, false>;

template <typename ValueType>
using string_map32 = DoubleArrayPatriciaTrie<ValueType, uint32_t, false, true, 1, false>;

template <typename ValueType>
using string_map64 = DoubleArrayPatriciaTrie<ValueType, uint64_t, false, true, 1, false>;

template <typename ValueType>
using unordered_string_map32 = DoubleArrayPatriciaTrie<ValueType, uint32_t, false, false, 1, false>;

template <typename ValueType>
using unordered_string_map64 = DoubleArrayPatriciaTrie<ValueType, uint64_t, false, false, 1, false>;

}

#endif //DOUBLEARRAYTRIEDICT_HPP_
