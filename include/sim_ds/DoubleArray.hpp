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
class _DoubleArrayBase {};

template <typename IndexType>
class _DoubleArrayBase<IndexType, true> {
public:
    using _index_type = IndexType;
    using _input_trie = graph_util::Trie<char>;
    
    static constexpr unsigned kBlockSize = 0x100;
    static constexpr _index_type kInitialEmptyHead = std::numeric_limits<_index_type>::max();
    
    static constexpr unsigned kCheckId = 0;
    static constexpr unsigned kBaseId = 1;
    
protected:
    _index_type empty_head_ = kInitialEmptyHead;
    BitVector empty_signs_;
    MultipleVector storage_;
    
    _DoubleArrayBase() : storage_({(size_t) sizeof(_index_type), (size_t) sizeof(_index_type)}) {
        _expand_block();
        _set_check(0, 0);
    }
    
    _DoubleArrayBase(const _input_trie& trie) : _DoubleArrayBase() {
        _dfs_build(trie, graph_util::kRootIndex, 0);
    }
    
    void _dfs_build(const _input_trie& trie, size_t node_i, _index_type base) {
        auto node = trie.node(node_i);
        if (node.terminal())
            return;
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
            _dfs_build(trie, node.target(c), target_base xor c);
        }
    }
    
    bool _empty(size_t index) const {
        return empty_signs_[index];
    }
    
    _index_type _next(size_t index) const {
        assert(empty_signs_[index]);
        return storage_.nested_element<0>(index);
    }
    
    _index_type _prev(size_t index) const {
        assert(empty_signs_[index]);
        return storage_.nested_element<1>(index);
    }
    
    void _set_next(size_t index, _index_type next) {
        assert(empty_signs_[index]);
        storage_.set_nested_element<0>(index, next);
    }
    
    void _set_prev(size_t index, _index_type prev) {
        assert(empty_signs_[index]);
        storage_.set_nested_element<1>(index, prev);
    }
    
    _index_type _check(size_t index) const {
        assert(not empty_signs_[index]);
        return storage_.nested_element<kCheckId>(index);
    }
    
    _index_type _base(size_t index) const {
        assert(not empty_signs_[index]);
        return storage_.nested_element<kBaseId>(index);
    }
    
    void _set_check(size_t index, _index_type check) {
        assert(empty_signs_[index]);
        auto next = _next(index);
        if (next == index) {
            empty_head_ = kInitialEmptyHead;
        } else {
            if (index == empty_head_)
                empty_head_ = next;
            auto prev = _prev(index);
            _set_next(prev, next);
            _set_prev(next, prev);
        }
        storage_.set_nested_element<kCheckId>(index, check);
        empty_signs_[index] = false;
    }
    
    void _set_base(size_t index, _index_type base) {
        assert(not empty_signs_[index]);
        storage_.set_nested_element<kBaseId>(index, base);
    }
    
    void _resize(size_t new_size) {
        storage_.resize(new_size);
        empty_signs_.resize(new_size, true);
    }
    
    void _expand_block() {
        size_t begin = storage_.size();
        _resize(storage_.size() + kBlockSize);
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
        _index_type index = empty_head_;
        if (index == kInitialEmptyHead) {
            _expand_block();
            index = empty_head_;
        }
        for (; ; index = _next(index)) {
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
            if (_next(index) == empty_head_)
                _expand_block();
        }
        throw "Not found base!";
    }
    
};


template <typename IndexType>
class _DoubleArrayBase<IndexType, false> {
public:
    using _index_type = IndexType;
    using _input_trie = graph_util::Trie<char>;
    
    static constexpr unsigned kBlockSize = 0x100;
    static constexpr size_t kInitialEmptyHead = EmptyLinkedVector<_index_type>::kInitialEmptyHead;
    
    static constexpr unsigned kCheckId = 0;
    static constexpr unsigned kBaseId = 1;
    
protected:
    EmptyLinkedVector<int> empty_blocks_;
    BitVector empty_signs_;
    MultipleVector storage_;
    
    _DoubleArrayBase() : storage_{(size_t) sizeof(_index_type), (size_t) sizeof(_index_type)} {
        _expand_block();
        _set_check(0, 0); // set root
    }
    
    _DoubleArrayBase(const _input_trie& trie) : _DoubleArrayBase() {
        _dfs_build(trie, graph_util::kRootIndex, 0);
    }
    
    void _dfs_build(const _input_trie& trie, size_t node_i, _index_type base) {
        auto node = trie.node(node_i);
        if (node.terminal())
            return;
        std::vector<uint8_t> children;
        node.for_each_edge([&](uint8_t c, auto e) {
            children.push_back(c);
        });
        auto target_base = _find_base(children);
        _set_base(base, target_base);
        for (uint8_t c : children) {
            _set_check(target_base xor c, base);
        }
        _update_block_status(target_base / kBlockSize);
        for (uint8_t c : children) {
            _dfs_build(trie, node.target(c), target_base xor c);
        }
    }
    
    bool _empty(size_t index) const {
        return empty_signs_[index];
    }
    
    _index_type _check(size_t index) const {
        assert(not empty_signs_[index]);
        return storage_.nested_element<kCheckId>(index);
    }
    
    _index_type _base(size_t index) const {
        assert(not empty_signs_[index]);
        return storage_.nested_element<kBaseId>(index);
    }
    
    void _set_check(size_t index, _index_type check) {
        assert(empty_signs_[index]);
        storage_.set_nested_element<kCheckId>(index, check);
        empty_signs_[index] = false;
    }
    
    void _set_base(size_t index, _index_type base) {
        assert(not empty_signs_[index]);
        storage_.set_nested_element<kBaseId>(index, base);
    }
    
    void _update_block_status(size_t block) {
        if (bit_util::is_zero256(empty_signs_.data() + 4 * block)) {
            empty_blocks_.set_value(block, 0);
        }
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
        size_t block = empty_blocks_.empty_front_index();
        if (block == kInitialEmptyHead) {
            _expand_block();
            block = empty_blocks_.empty_front_index();
        }
        for (; ; block = empty_blocks_[block].next()) {
            alignas(32) uint64_t field[4];
            bit_util::set256_epi1(true, field);
            for (auto c : children) {
                bit_util::mask_xor_idx_and256(field, empty_signs_.data() + 4 * block, c, field);
            }
            auto ctz = bit_util::ctz256(field);
            if (ctz < kBlockSize) {
                return kBlockSize * block + ctz;
            }
            if (empty_blocks_[block].next() == empty_blocks_.empty_front_index()) {
                _expand_block();
            }
        }
        throw "Not found base!";
    }
    
};


template <typename IndexType, bool LegacyBuild = false>
class DoubleArray : private _DoubleArrayBase<IndexType, LegacyBuild> {
    using _base = _DoubleArrayBase<IndexType, LegacyBuild>;
public:
    using input_trie = typename _base::_input_trie;
    using index_type = typename _base::_index_type;
    
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;
    
    DoubleArray() = default;
    
    DoubleArray(const input_trie& trie) : _base(trie) {}
    
    void insert(std::string_view key) {
        // TODO: insert key
    }
    
    bool accept(std::string_view key) const {
        index_type node = 0;
        for (uint8_t c : key) {
#ifndef NDEBUG
            auto base = _base::_base(node);
#endif
            index_type target = _base::_base(node) xor c;
#ifndef NDEBUG
            auto empty = _base::_empty(target);
            auto check = _base::_check(target);
#endif
            if (_base::_empty(target) or
                _base::_check(target) != node) {
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
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex: \texists, \tcheck, \tbase"  << std::endl;
        for (size_t i = 0; i < _base::storage_.size(); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            auto empty =_base::_empty(i);
            if (empty) {
                std::cout << "\t\t"<<i<<": \t"<<0<< std::endl;
            } else {
                std::cout << "\t\t"<<i<<": \t"<<1<<", \t"<<_base::_check(i)<<", \t"<<_base::_base(i)<< std::endl;
            }
        }
    }
    
};

} // namespace sim_ds

#endif /* DoubleArray_hpp */
