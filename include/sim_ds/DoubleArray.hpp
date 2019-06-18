//
//  DoubleArray.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/06/18.
//

#ifndef DoubleArray_hpp
#define DoubleArray_hpp

#include "basic.hpp"
#include "MultipleVector.hpp"
#include "BitVector.hpp"
#include "EmptyLinkedVector.hpp"
#include "graph_util.hpp"
#include "bit_util256.hpp"

namespace sim_ds {


template <typename IndexType, bool LegacyBuild>
class DoubleArray;


template <typename IndexType, bool LegacyBuild>
class _DoubleArrayImpl {};

template <typename IndexType>
class _DoubleArrayImpl<IndexType, true> {
public:
    using _index_type = IndexType;
    using _input_trie = graph_util::Trie<char>;
    
    static constexpr unsigned kBlockSize = 0x100;
    static constexpr size_t kInitialEmptyHead = std::numeric_limits<size_t>::max();
    
protected:
    size_t empty_head_ = kInitialEmptyHead;
    MultipleVector storage_;
    BitVector empty_signs_;
    
    _DoubleArrayImpl() : storage_({(size_t) sizeof(_index_type), (size_t) sizeof(_index_type)}) {
        _expand_block();
        _set_check(0, 0);
    }
    
    _DoubleArrayImpl(_input_trie& trie) : _DoubleArrayImpl() {
        std::function<void(size_t, _index_type)> dfs = [&](size_t node_i, _index_type base) {
            auto node = trie.node(node_i);
            std::vector<uint8_t> children;
            node.for_each_edge([&](uint8_t c, auto e) {
                children.push_back(c);
            });
            auto target_base = _find_base(children);
            _set_base(base, target_base);
            for (uint8_t c : children) {
                _set_check(target_base xor c, base);
            }
            for (uint8_t c : children) {
                dfs(node.target(c), target_base xor c);
            }
        };
        dfs(trie.root(), 0);
    }
    
    _index_type _next(size_t index) const {
        return storage_.nested_element<0>(index);
    }
    
    _index_type _prev(size_t index) const {
        return storage_.nested_element<1>(index);
    }
    
    void _set_next(size_t index, _index_type next) {
        storage_.set_nested_element<0>(index, next);
    }
    
    void _set_prev(size_t index, _index_type prev) {
        storage_.set_nested_element<1>(index, prev);
    }
    
    _index_type _base(size_t index) const {
        return storage_.nested_element<0>(index);
    }
    
    _index_type _check(size_t index) const {
        return storage_.nested_element<1>(index);
    }
    
    void _set_check(size_t index, _index_type check) {
        auto next = _next(index);
        auto prev = _prev(index);
        _set_next(prev, next);
        _set_prev(next, prev);
        storage_.set_nested_element<1>(index, check);
    }
    
    void _set_base(size_t index, _index_type base) {
        storage_.set_nested_element<0>(index, base);
    }
    
    void _resize(size_t new_size) {
        storage_.resize(new_size);
        empty_signs_.resize(new_size, 1);
    }
    
    void _expand_block() {
        size_t begin = storage_.size();
        resize(storage_.size() + kBlockSize);
        size_t end = storage_.size();
        // empty-element linking
        if (empty_head_ != kInitialEmptyHead) {
            auto old_back = _prev(empty_head_);
            _set_prev(begin, old_back);
            _set_next(old_back, begin);
            _set_prev(empty_head_, end-1);
        } else {
            _set_prev(begin, end-1);
            empty_head_ = begin;
        }
        _set_next(begin, begin+1);
        for (size_t i = begin + 1; i < end - 1; i++) {
            _set_next(i, i+1);
            _set_prev(i, i-1);
        }
        _set_next(end-1, empty_head_);
        _set_prev(end-1, end-2);
    }
    
    _index_type _find_base(const std::vector<uint8_t>& children) {
        for (auto index = empty_head_;; index = _next(index)) {
            if (index == kInitialEmptyHead) {
                _expand_block();
                index = empty_head_;
            }
            _index_type b = index xor children.front();
            bool skip = false;
            for (uint8_t c : children) {
                if (not empty_signs_[b xor c]) {
                    skip = true;
                    break;
                }
            }
            if (not skip) {
                return b;
            }
        }
        return 0;
    }
    
};


template <typename IndexType>
class _DoubleArrayImpl<IndexType, false> {
public:
    using _index_type = IndexType;
    using _input_trie = graph_util::Trie<char>;
    
    static constexpr unsigned kBlockSize = 0x100;
    static constexpr size_t kInitialEmptyHead = EmptyLinkedVector<_index_type>::kInitialEmptyHead;
    
protected:
    EmptyLinkedVector<_index_type> empty_blocks_;
    MultipleVector storage_;
    BitVector empty_signs_;
    
    _DoubleArrayImpl() : storage_{(size_t) sizeof(_index_type), (size_t) sizeof(_index_type)} {
        _expand_block();
        _set_check(0, 0); // set root
    }
    
    _DoubleArrayImpl(_input_trie& trie) : _DoubleArrayImpl() {
        std::function<void(size_t, _index_type)> dfs = [&](size_t node_i, _index_type base) {
            auto node = trie.node(node_i);
            std::vector<uint8_t> children;
            node.for_each_edge([&](uint8_t c, auto e) {
                children.push_back(c);
            });
            auto target_base = _find_base(children);
            _set_base(base, target_base);
            for (uint8_t c : children) {
                _set_check(target_base xor c, base);
            }
            for (uint8_t c : children) {
                dfs(node.target(c), target_base xor c);
            }
        };
        dfs(graph_util::kRootIndex, 0);
    }
    
    _index_type _base(size_t index) const {
        return storage_.nested_element<0>(index);
    }
    
    _index_type _check(size_t index) const {
        return storage_.nested_element<1>(index);
    }
    
    void _set_check(size_t index, _index_type check) {
        storage_.set_nested_element<1>(index, check);
        empty_signs_[index] = false;
        auto block = index / kBlockSize;
        if (bit_util::is_zero256(empty_signs_.data() + 4 * block)) {
            empty_blocks_.set_value(block, 0);
        }
    }
    
    void _set_base(size_t index, _index_type base) {
        storage_.set_nested_element<0>(index, base);
    }
    
    void _resize(size_t new_size) {
        storage_.resize(new_size);
        empty_signs_.resize(new_size, 1);
    }
    
    void _expand_block() {
        _resize(storage_.size() + kBlockSize);
        empty_blocks_.resize(empty_blocks_.size() + 1);
    }
    
    _index_type _find_base(const std::vector<uint8_t>& children) {
        for (size_t block = empty_blocks_.empty_front_index(); ; block = empty_blocks_[block].next()) {
            if (block == kInitialEmptyHead) {
                _expand_block();
                block = empty_blocks_.empty_front_index();
            }
            alignas(32) uint64_t field[4];
            bit_util::set256_epi1(true, field);
            for (auto c : children) {
                bit_util::mask_xor_idx_and256(field, empty_signs_.data() + 4 * block, c, field);
            }
            auto ctz = bit_util::ctz256(field);
            if (ctz < kBlockSize) {
                return kBlockSize * block + ctz;
            }
        }
        throw "Not found base!";
    }
    
};


template <typename IndexType, bool LegacyBuild = false>
class DoubleArray : private _DoubleArrayImpl<IndexType, LegacyBuild> {
public:
    using _base = _DoubleArrayImpl<IndexType, LegacyBuild>;
    using input_trie = typename _base::_input_trie;
    using index_type = typename _base::_index_type;
    
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;
    
    DoubleArray() = default;
    
    DoubleArray(input_trie& trie) : _base(trie) {}
    
    void insert(std::string_view key) {
        // TODO: insert key
    }
    
    bool accept(std::string_view key) const {
        index_type node = 0;
        for (uint8_t c : key) {
            index_type target = _base::_base(node) xor c;
            if (_base::_check(target) != node) {
                return false;
            }
            node = target;
        }
        index_type target = _base::_base(node) xor kLeafChar;
        if (_base::_check(target) != node) {
            return false;
        }
        return true;
    }
    
};

} // namespace sim_ds

#endif /* DoubleArray_hpp */
