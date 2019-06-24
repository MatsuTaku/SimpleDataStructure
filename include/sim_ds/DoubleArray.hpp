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


template <class IndexType>
class _DoubleArrayUnit {
public:
    using _index_type = IndexType;
    using _char_type = uint8_t;
    
    static constexpr size_t kIndexBits = (sizeof(_index_type) == 8 ? 56 :
                                          sizeof(_index_type) == 4 ? 24 :
                                          sizeof(_index_type) == 2 ? 16 :
                                          8);
    static constexpr size_t kCharBits = 8;
    
    static constexpr _index_type kEmptyFlag = 1ull << (kIndexBits - 1);
    static constexpr _char_type kEmptyChar = 0xFF;
    
private:
    _index_type check_ : kIndexBits;
    _char_type sibling_ : kCharBits;
    _index_type base_ : kIndexBits;
    _char_type child_ : kCharBits;
    
public:
    _DoubleArrayUnit() : check_(kEmptyFlag), sibling_(kEmptyChar), base_(kEmptyFlag), child_(kEmptyChar) {}
    
    ~_DoubleArrayUnit() = default;
    
    void init() {
        check_ = kEmptyFlag;
        sibling_ = kEmptyChar;
        base_ = kEmptyFlag;
        child_ = kEmptyChar;
    }
    
    _index_type check() const {return check_;}
    
    void set_check(_index_type check) {
        check_ = check;
    }
    
    _char_type sibling() const {return sibling_;}
    
    void set_sibling(_char_type sibling) {
        sibling_ = sibling;
    }
    
    _index_type base() const {return base_;}
    
    void set_base(_index_type base) {
        base_ = base;
    }
    
    _char_type child() const {return child_;}
    
    void set_child(_char_type child) {
        child_ = child;
    }
    
};


template <typename IndexType, class BlockType>
class _DoubleArrayCommon;


template <typename IndexType>
class _DoubleArrayBlock {
    using _self = _DoubleArrayBlock<IndexType>;
public:
    using _unit_type = _DoubleArrayUnit<IndexType>;
    using _index_type = typename _unit_type::_index_type;
    using _char_type = typename _unit_type::_char_type;
    
    static constexpr size_t kIndexBits = _unit_type::kIndexBits;
    
    static constexpr unsigned kBlockSize = 0x100;
    
    static constexpr uint16_t kFrozenFlag = 1ull << 15;
    
protected:
    friend class _DoubleArrayCommon<IndexType, _self>;
    
    uint64_t field_[4];
    _index_type link_[2];
    uint16_t num_empties_; // 0 <= n <= 256. using 9bits.
    _unit_type storage_[kBlockSize];
    
public:
    _DoubleArrayBlock() : num_empties_(kBlockSize) {}
    
    _DoubleArrayBlock(_index_type prev, _index_type next) : link_{prev, next}, num_empties_(kBlockSize) {
        bit_util::set256_epi1(1, field_);
    }
    
    virtual ~_DoubleArrayBlock() = default;
    
    bool filled() const {return (num_empties_ & compl(kFrozenFlag)) == 0;}
    
    const uint64_t* field_ptr() {
        return field_;
    }
    
    _index_type prev() const {return link_[0];}
    
    void set_prev(_index_type prev) {
        link_[0] = prev;
    }
    
    _index_type next() const {return link_[1];}
    
    void set_next(_index_type next) {
        link_[1] = next;
    }
    
    bool frozen() const {return num_empties_ & kFrozenFlag;}
    
    void freeze() {
        num_empties_ |= kFrozenFlag;
    }
    
    void thaw() {
        num_empties_ &= compl kFrozenFlag;
    }
    
    void consume(size_t num = 1) {
        assert((num_empties_ & compl(kFrozenFlag)) >= num);
        num_empties_ = (num_empties_ & kFrozenFlag) | ((num_empties_ & compl(kFrozenFlag)) - num);
    }
    
    void refill(size_t num = 1) {
        assert((num_empties_ & compl(kFrozenFlag)) + num <= kBlockSize);
        num_empties_ = (num_empties_ & kFrozenFlag) | ((num_empties_ & compl(kFrozenFlag)) + num);
    }
    
    bool empty_at(_index_type index) const {
        return field_[index/64] & bit_util::OffsetMask(index%64);
    }
    
    void freeze_at(_index_type index) {
        field_[index/64] &= compl bit_util::OffsetMask(index%64);
        consume();
    }
    
    void thaw_at(_index_type index) {
        field_[index/64] |= bit_util::OffsetMask(index%64);
        refill();
    }
    
};


template <typename IndexType>
class _DoubleArrayBlockLegacy : public _DoubleArrayBlock<IndexType> {
    using _base = _DoubleArrayBlock<IndexType>;
    using _self = _DoubleArrayBlockLegacy<IndexType>;
public:
    using _inset_type = uint16_t;
    static constexpr _inset_type kInitialEmptyHead = std::numeric_limits<_inset_type>::max();
    
private:
    friend class _DoubleArrayCommon<IndexType, _self>;
    
    _inset_type empty_head_;
    
public:
    _DoubleArrayBlockLegacy() : _base(), empty_head_(0) {}
    
    _DoubleArrayBlockLegacy(_inset_type prev, _inset_type next) : _base(prev, next), empty_head_(0) {}
    
    ~_DoubleArrayBlockLegacy() = default;
    
    _inset_type empty_head() const {return empty_head_;}
    
    void set_empty_head(_inset_type index) {
        empty_head_ = index;
    }
    
};


template <typename IndexType, class BlockType>
class _DoubleArrayCommon {
public:
    using _self = _DoubleArrayCommon<IndexType, BlockType>;
    using _index_type = IndexType;
    using _char_type = uint8_t;
    using _block_type = BlockType;
    using _unit_type = typename _block_type::_unit_type;
    
    static constexpr _index_type kRootIndex = 0;
    
    static constexpr _char_type kLeafChar = graph_util::kLeafChar;
    
    static constexpr _index_type kEmptyFlag = _unit_type::kEmptyFlag;
    static constexpr _char_type kEmptyChar = _unit_type::kEmptyChar;
    
    static constexpr unsigned kBlockSize = _block_type::kBlockSize;
    static constexpr _index_type kInitialEmptyBlockHead = std::numeric_limits<_index_type>::max();
    
    static constexpr size_t kIndexBits = _unit_type::kIndexBits;
    
    using _input_trie = graph_util::Trie<char>;
    
protected:
    _index_type empty_block_head_;
    std::vector<_block_type> container_;
    std::vector<_char_type> tail_;
    
    _DoubleArrayCommon() : empty_block_head_(kInitialEmptyBlockHead) {}
    
    virtual ~_DoubleArrayCommon() = default;
    
    size_t _size_in_bytes() const {
        return sizeof(empty_block_head_) + size_vec(container_);
    }
    
    size_t _num_elements() const {return container_.size() * kBlockSize;}
    
    virtual void _expand() {
        assert(container_.size() < 1ull << (kIndexBits-9));
        if (empty_block_head_ == kInitialEmptyBlockHead) {
            auto front = container_.size();
            container_.emplace_back(front, front);
            empty_block_head_ = front;
        } else {
            auto prev = _block(empty_block_head_).prev();
            auto back = container_.size();
            container_.emplace_back(prev, empty_block_head_);
            _block(empty_block_head_).set_prev(back);
            _block(prev).set_next(back);
        }
    }
    
    _block_type& _block(_index_type block) {
        return container_[block];
    }
    
    const _block_type& _block(_index_type block) const {
        return container_[block];
    }
    
    bool _frozen_block(_index_type block) const {return _block(block).frozen();}
    
    void _freeze_block(_index_type block) {
        auto& b = _block(block);
        auto next = b.next();
        auto prev = b.prev();
        _block(next).set_prev(prev);
        _block(prev).set_next(next);
        if (next == block)
            empty_block_head_ = kInitialEmptyBlockHead;
        else if (block == empty_block_head_)
            empty_block_head_ = next;
        
        b.freeze();
    }
    
    void _thaw_block(_index_type block) {
        auto& b = _block(block);
        if (empty_block_head_ == kInitialEmptyBlockHead) {
            b.set_next(block);
            b.set_prev(block);
            empty_block_head_ = block;
        } else {
            auto& front_b = _block(empty_block_head_);
            auto back = front_b.prev();
            _block(back).set_next(block);
            front_b.set_prev(block);
            b.set_next(empty_block_head_);
            b.set_prev(back);
            if (block < empty_block_head_)
                empty_block_head_ = block;
        }
        
        b.thaw();
    }
    
    void _confirm_block(_index_type block) {
        auto& b = _block(block);
        if (not _frozen_block(block) and b.filled()) {
            _freeze_block(block);
        } else if (_frozen_block(block) and not b.filled()) {
            _thaw_block(block);
        }
    }
    
    _unit_type& _unit(_index_type index) {
        return _block(index/kBlockSize).storage_[index%kBlockSize];
    }
    
    const _unit_type& _unit(_index_type index) const {
        return _block(index/kBlockSize).storage_[index%kBlockSize];
    }
    
    bool _empty(size_t index) const {
        return _block(index/kBlockSize).empty_at(index%kBlockSize);
    }
    
    virtual void _freeze(size_t index) {
        assert(_empty(index));
        _block(index/kBlockSize).freeze_at(index%kBlockSize);
    }
    
    virtual void _thaw(size_t index) {
        assert(not _empty(index));
        _block(index/kBlockSize).thaw_at(index%kBlockSize);
    }
    
    _index_type _check(_index_type index) const {
        return _unit(index).check();
    }
    
    void _set_check(size_t index, _index_type check) {
        _unit(index).set_check(check);
    }
    
    _char_type _sibling(_index_type index) const {
        assert(_unit(index).sibling() != kEmptyChar);
        return _unit(index).sibling();
    }
    
    void _set_sibling(_index_type index, _char_type sibling) {
        _unit(index).set_sibling(sibling);
    }
    
    void _set_check_sibling(_index_type index, _index_type check, _char_type sibling) {
        _set_check(index, check);
        _set_sibling(index, sibling);
    }
    
    _index_type _base(_index_type node) const {
        return _unit(node).base();
    }
    
    void _set_base(_index_type node, _index_type base) {
        _unit(node).set_base(base);
    }
    
    _char_type _child(_index_type node) const {
        return _unit(node).child();
    }
    
    void _set_child(_index_type node, _char_type c) {
        _unit(node).set_child(c);
    }
    
    bool _leaf(_index_type node) const {
        return _base(node) & kEmptyFlag;
    }
    
    void _set_base_child(_index_type node, _index_type base, _char_type child) {
        assert(not _empty(node));
        _set_base(node, base);
        _set_child(node, child);
    }
    
    _index_type _tail_index(_index_type node) const {
        assert(_leaf(node));
        return _base(node) & compl(kEmptyFlag);
    }
    
    void _set_tail_index(_index_type node, _index_type tail_index) {
        assert(_leaf(node));
        _set_base(node, tail_index | kEmptyFlag);
    }
    
    std::string_view _suffix_in_tail(_index_type tail_index) const {
        return std::string_view((char*)tail_.data() + tail_index);
    }
    
    // Must call after thaw target element.
    void _clean(size_t index) {
        _unit(index).init();
    }
    
    void _setup(size_t index) {
        _freeze(index);
        _clean(index);
    }
    
    void _erase(size_t index) {
        _clean(index);
        _thaw(index);
    }
    
    template <class Action>
    void _for_each_children(_index_type node, Action action) {
        assert(not _leaf(node));
        auto base = _base(node);
        auto first_child = _child(node);
        for (auto child = first_child; ; ) {
            auto next = base xor child;
            assert(_check(next) == node);
            auto sibling = _sibling(next);
            action(child, sibling);
            if (sibling == first_child)
                break;
            child = sibling;
        }
    }
    
    template <class Action>
    void _for_each_children(_index_type node, Action action) const {
        assert(not _leaf(node));
        auto base = _base(node);
        auto first_child = _child(node);
        for (auto child = first_child; ; ) {
            auto next = base xor child;
            assert(_check(next) == node);
            auto sibling = _sibling(next);
            action(child, sibling);
            if (sibling == first_child)
                break;
            child = sibling;
        }
    }
    
    size_t _num_of_children(_index_type node) const {
        size_t cnt = 0;
        _for_each_children(node, [&cnt](auto, auto) {++cnt;});
        return cnt;
    }
    
    virtual _index_type _find_base(const std::vector<_char_type>& children) = 0;
    
    // MARK: Dynamic construction methods
    
    _index_type _grow(_index_type node, _index_type base, _char_type c) {
        assert(_leaf(node));
        auto next = base xor c;
        assert(_empty(next));
        _setup(next);
        _set_check_sibling(next, node, c);
        _set_base_child(node, base, c);
        _confirm_block(base/kBlockSize);
        return next;
    }
    
    struct _moving_luggage {
        _index_type base;
        uint8_t child;
        _moving_luggage(_index_type base, uint8_t child) : base(base), child(child) {}
    };
    
    _index_type _move_nodes(_index_type node,
                                    std::vector<_char_type>& children,
                                    std::vector<_moving_luggage>& luggages,
                                    _index_type new_base,
                                    _index_type monitoring_node = kRootIndex) {
        auto base = _base(node);
        for (size_t i = 0; i < children.size(); i++) {
            auto child = children[i];
            auto sibling = children[(i+1)%children.size()];
            auto next = base xor child;
            auto new_next = new_base xor child;
            assert(next != new_next);
            _setup(new_next);
            _set_check_sibling(new_next, node, sibling);
            auto next_base = luggages[i].base;
            if (not (next_base & kEmptyFlag)) {
                auto grand_first_child = luggages[i].child;
                _set_base_child(new_next, next_base, grand_first_child);
                for (auto grand_child = grand_first_child; ; ) {
                    _set_check(next_base xor grand_child, new_next);
                    auto sibling = _sibling(next_base xor grand_child);
                    if (sibling == grand_first_child)
                        break;
                    grand_child = sibling;
                }
            }
            if (next == monitoring_node) {
                monitoring_node = new_next;
            }
        }
        _set_base(node, new_base);
        
        _confirm_block(new_base/kBlockSize);
        
        return monitoring_node;
    }
    
    _index_type _solve_collision(_index_type node, _char_type c) {
        auto base = _base(node);
        auto conflicting_index = base xor c;
        auto competitor = _check(conflicting_index);
        if (conflicting_index != kRootIndex and
            _num_of_children(competitor) <= _num_of_children(node)) {
            auto competitor_base = _base(competitor);
            assert(competitor_base / 256 == base / 256);
            std::vector<_char_type> children;
            std::vector<_moving_luggage> luggages;
            _for_each_children(competitor, [&](auto child, auto) {
                auto next = competitor_base xor child;
                children.push_back(child);
                luggages.emplace_back(_base(next), _child(next));
                _erase(next);
            });
            _freeze(conflicting_index);
            _confirm_block(competitor_base/kBlockSize);
            auto new_base = _find_base(children);
            _thaw(conflicting_index);
            node = _move_nodes(competitor, children, luggages, new_base, node);
        } else {
            std::vector<_char_type> children;
            std::vector<_moving_luggage> luggages;
            _for_each_children(node, [&](auto child, auto) {
                auto next = base xor child;
                children.push_back(child);
                luggages.emplace_back(_base(next), _child(next));
                _erase(next);
            });
            _confirm_block(base/kBlockSize);
            children.push_back(c);
            auto new_base = _find_base(children);
            children.pop_back();
            _move_nodes(node, children, luggages, new_base);
        }
        return node;
    }
    
    _index_type _insert_trans(_index_type node, _char_type c) {
        if (_leaf(node)) {
            return _grow(node, _find_base({c}), c);
        }
        auto base = _base(node);
        if (not _empty(base xor c)) {
            node = _solve_collision(node, c);
            base = _base(node);
        }
        auto next = base xor c;
        assert(_empty(next));
        _setup(next);
        _set_check(next, node);
        auto first_child = _child(node);
        assert(c != first_child);
        if (c < first_child) {
            _set_child(node, c);
            _set_sibling(next, first_child);
            for (auto child = first_child; ; ) {
                auto sibling = _sibling(base xor child);
                if (sibling == first_child) {
                    _set_sibling(base xor child, c);
                    break;
                }
                child = sibling;
            }
        } else {
            for (auto child = first_child; ; ) {
                auto sibling = _sibling(base xor child);
                if (sibling > c or sibling == first_child) {
                    _set_sibling(base xor child, c);
                    _set_sibling(next, sibling);
                    break;
                }
                child = sibling;
            }
        }
        
        _confirm_block(base/kBlockSize);
        
        return next;
    }
    
    _index_type _insert_suffix(std::string_view suffix) {
        _index_type index = tail_.size();
        for (_char_type c : suffix)
            tail_.push_back(c);
        tail_.push_back(kLeafChar);
        return index;
    }
    
    void _insert_in_bc(_index_type node, std::string_view suffix) {
        if (suffix.size() > 0) {
            node = _insert_trans(node, suffix.front());
            auto tail_index = _insert_suffix(suffix.size() > 1 ? suffix.substr(1) : "");
            _set_tail_index(node, tail_index);
        } else {
            node = _insert_trans(node, kLeafChar);
        }
    }
    
    void _insert_in_tail(_index_type node, _index_type tail_pos, std::string_view suffix) {
        auto tail_index = _tail_index(node);
        while (tail_index < tail_.size() and tail_index <= tail_pos) {
            auto c = tail_[tail_index++];
            node = _grow(node, _find_base({c}), c);
        }
        _insert_in_bc(node, suffix);
    }
    
};


template <typename IndexType, bool LegacyBuild>
class _DoubleArrayBase;


template <typename IndexType>
class _DoubleArrayBase<IndexType, true> : protected _DoubleArrayCommon<IndexType, _DoubleArrayBlockLegacy<IndexType>> {
public:
    using _common = _DoubleArrayCommon<IndexType, _DoubleArrayBlockLegacy<IndexType>>;
    using _block_type = typename _common::_block_type;
    using _index_type = typename _common::_index_type;
    using _char_type = typename _common::_char_type;
    using _inset_type = typename _block_type::_inset_type;
    using _input_trie = graph_util::Trie<char>;
    
    static constexpr _index_type kRootIndex = _common::kRootIndex;
    
    static constexpr unsigned kBlockSize = _block_type::kBlockSize;
    static constexpr _index_type kInitialEmptyBlockHead = _common::kInitialEmptyBlockHead;
    static constexpr _inset_type kInitialEmptyHead = _block_type::kInitialEmptyHead;
    
protected:
    _DoubleArrayBase() : _common() {
        _expand();
        _common::_setup(kRootIndex);
        _common::_set_check_sibling(kRootIndex, 0, 0); // set root
    }
    
    virtual ~_DoubleArrayBase() = default;
    
    size_t _size_in_bytes() const {
        return _common::_size_in_bytes();
    }
    
    _index_type _next(size_t index) const {
        assert(_common::_empty(index));
        return _common::_base(index) bitand compl(_common::kEmptyFlag);
    }
    
    void _set_next(size_t index, _index_type next) {
        assert(_common::_empty(index));
        _common::_set_base(index, next bitor _common::kEmptyFlag);
    }
    
    _index_type _prev(size_t index) const {
        assert(_common::_empty(index));
        return _common::_check(index) bitand compl(_common::kEmptyFlag);
    }
    
    void _set_prev(size_t index, _index_type prev) {
        assert(_common::_empty(index));
        _common::_set_check(index, prev bitor _common::kEmptyFlag);
    }
    
    void _freeze(size_t index) override {
        auto next = _next(index);
        auto prev = _prev(index);
        auto offset = kBlockSize * (index/kBlockSize);
        _set_prev(offset+next, prev);
        _set_next(offset+prev, next);
        auto& b = _common::_block(index/kBlockSize);
        auto inset = index % kBlockSize;
        if (next == inset) {
            b.set_empty_head(kInitialEmptyHead);
        } else if (inset == b.empty_head()) {
            b.set_empty_head(next);
        }
        
        _common::_freeze(index);
    }
    
    void _thaw(size_t index) override {
        _common::_thaw(index);
        
        auto& b = _common::_block(index/kBlockSize);
        auto inset = index % kBlockSize;
        if (b.empty_head() == kInitialEmptyHead) {
            _set_next(index, inset);
            _set_prev(index, inset);
            b.set_empty_head(inset);
        } else {
            auto offset = kBlockSize * (index/kBlockSize);
            auto back = _prev(offset+b.empty_head());
            _set_next(offset+back, inset);
            _set_prev(offset+b.empty_head(), inset);
            _set_next(index, b.empty_head());
            _set_prev(index, back);
        }
    }
    
    void _expand() override {
        auto front = _common::_num_elements();
        _common::_expand();
        // empty-element linking
        auto inset_back = kBlockSize - 1;
        _set_prev(front, inset_back);
        _set_next(front, 1);
        for (size_t i = 1; i < inset_back; i++) {
            _set_prev(front+i, i-1);
            _set_next(front+i, i+1);
        }
        _set_prev(front+inset_back, inset_back-1);
        _set_next(front+inset_back, 0);
    }
    
    _index_type _find_base(const std::vector<_char_type>& children) override {
        _index_type b = _common::empty_block_head_;
        if (b == kInitialEmptyBlockHead) {
            _expand();
            b = _common::empty_block_head_;
        }
        while (true) {
            auto& block = _common::_block(b);
            assert(block.empty_head() != kInitialEmptyHead);
            const auto offset = kBlockSize * b;
            for (auto index = offset + block.empty_head(); ; ) {
                _index_type n = index xor children.front();
                bool skip = false;
                for (_char_type c : children) {
                    if (not _common::_empty(n xor c)) {
                        skip = true;
                        break;
                    }
                }
                if (not skip) {
                    return n;
                }
                if (_next(index) == block.empty_head())
                    break;
                index = offset + _next(index);
            }
            if (block.next() == _common::empty_block_head_) {
                _expand();
                b = _common::_block(b).next();
            } else {
                b = block.next();
            }
        }
        throw "Not found base!";
    }
    
};


template <typename IndexType>
class _DoubleArrayBase<IndexType, false> : protected _DoubleArrayCommon<IndexType, _DoubleArrayBlock<IndexType>> {
public:
    using _common = _DoubleArrayCommon<IndexType, _DoubleArrayBlock<IndexType>>;
    using _index_type = typename _common::_index_type;
    using _char_type = typename _common::_char_type;
    using _input_trie = graph_util::Trie<char>;
    
    static constexpr _index_type kRootIndex = _common::kRootIndex;
    
    static constexpr unsigned kBlockSize = _common::kBlockSize;
    static constexpr _index_type kInitialEmptyBlockHead = _common::kInitialEmptyBlockHead;
    
protected:
    _DoubleArrayBase() : _common() {
        _common::_expand();
        _common::_setup(kRootIndex);
        _common::_set_check_sibling(kRootIndex, 0, 0); // set root
    }
    
    virtual ~_DoubleArrayBase() = default;
    
    size_t _size_in_bytes() const {
        return _common::_size_in_bytes();
    }
    
    _index_type _find_base(const std::vector<_char_type>& children) override {
        size_t b = _common::empty_block_head_;
        if (b == kInitialEmptyBlockHead) {
            _common::_expand();
            b = _common::empty_block_head_;
        }
        if (children.size() == 1) {
            return kBlockSize * b + (bit_util::ctz256(_common::_block(b).field_ptr()) xor children.front());
        } else {
            while (true) {
                auto& block = _common::_block(b);
                alignas(32) uint64_t field[4];
                bit_util::set256_epi1(1, field);
                for (auto c : children) {
                    bit_util::mask_xor_idx_and256(field, _common::_block(b).field_ptr(), c, field);
                }
                auto ctz = bit_util::ctz256(field);
                if (ctz < kBlockSize) {
                    return kBlockSize * b + ctz;
                }
                if (block.next() == _common::empty_block_head_) {
                    _common::_expand();
                    b = _common::_block(b).next();
                } else {
                    b = block.next();
                }
            }
        }
        throw "Not found base!";
    }
    
};


template <typename IndexType, bool LegacyBuild = false>
class DoubleArray : private _DoubleArrayBase<IndexType, LegacyBuild> {
    using _base = _DoubleArrayBase<IndexType, LegacyBuild>;
    using _self = DoubleArray<IndexType, LegacyBuild>;
public:
    using input_trie = typename _base::_input_trie;
    using index_type = typename _base::_index_type;
    using char_type = typename _base::_char_type;
    
    static constexpr index_type kRootIndex = _base::kRootIndex;
    static constexpr char_type kLeafChar = graph_util::kLeafChar;
    
    DoubleArray() = default;
    
    DoubleArray(std::vector<std::string>& key_set) {
        std::sort(key_set.begin(), key_set.end());
        for (std::string_view s : key_set)
            insert(s);
        rebuild();
    }
    
    ~DoubleArray() = default;
    
    DoubleArray& rebuild() {
        DoubleArray new_da;
        new_da._arrange_da(*this, kRootIndex, kRootIndex);
        *this = new_da;
        return *this;
    }
    
    size_t size_in_bytes() const {
        return _base::_size_in_bytes();
    }
    
    void insert(std::string_view key) {
        index_type node = kRootIndex;
        size_t pos = 0;
        for (; pos < key.size(); pos++) {
            if (_base::_leaf(node)) {
                break;
            }
            if (not _transition(node, key[pos])) {
                _base::_insert_in_bc(node, key.substr(pos));
                return;
            }
        }
        if (_base::_leaf(node)) {
            auto tail_index = _base::_tail_index(node);
            for (; pos < key.size(); pos++, tail_index++) {
                if (tail_index >= _base::tail_.size() or _base::tail_[tail_index] != key[pos]) {
                    _base::_insert_in_tail(node, tail_index, key.substr(pos));
                    return;
                }
            }
            if (_base::tail_[tail_index] != kLeafChar) {
                _base::_insert_in_tail(node, tail_index, "");
            }
        } else {
            if (not _transition(node, kLeafChar)) {
                _base::_insert_in_bc(node, "");
            }
        }
    }
    
    bool accept(std::string_view key) const {
        index_type node = kRootIndex;
        size_t pos = 0;
        for (; pos < key.size(); pos++) {
            if (_leaf(node)) {
                break;
            }
            if (not _transition(node, key[pos])) {
                return false;
            }
        }
        if (_leaf(node)) {
            auto tail_index = _tail_index(node);
            for (; pos < key.size(); pos++, tail_index++) {
                if (_base::tail_[tail_index] != key[pos])
                    return false;
            }
            return _base::tail_[tail_index] == kLeafChar;
        } else {
            return _transition(node, kLeafChar);
        }
    }
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex] \texists, \tcheck, \tsibling, \tbase, \tchild"  << std::endl;
        for (size_t i = 0; i < _base::storage_.size(); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            auto empty =_base::_empty(i);
            if (empty) {
                std::cout << "\t\t"<<i<<"] \t"<<0<< std::endl;
            } else {
                std::cout << "\t\t"<<i<<"] \t"<<1<<", \t"<<_base::_check(i)<<", \t"<<_base::_sibling(i)<<", \t"<<_base::_base(i)<<", \t"<<_base::_child(i)<< std::endl;
            }
        }
    }
    
private:
    bool _transition(index_type& node, char_type c) const {
        if (_base::_leaf(node))
            return false;
        assert(not _base::_leaf(node));
        auto next = _base::_base(node) xor c;
        if (_base::_empty(next) or
            _base::_check(next) != node)
            return false;
        node = next;
        return true;
    }
    
    void _arrange_da(const _self& da, index_type node, index_type co_node) {
        if (da._base::_leaf(node)) {
            auto tail_index = _base::_insert_suffix(da._base::_suffix_in_tail(da._base::_tail_index(node)));
            _base::_set_tail_index(co_node, tail_index);
        } else {
            std::vector<char_type> children;
            da._base::_for_each_children(node, [&](auto child, auto) {
                children.push_back(child);
            });
            auto new_base = _base::_find_base(children);
            _base::_set_base_child(co_node, new_base, children.front());
            for (size_t i = 0; i < children.size(); i++) {
                auto c = children[i];
                auto next = new_base xor c;
                _base::_setup(next);
                _base::_set_check_sibling(next, co_node, children[(i+1)%children.size()]);
            }
            _base::_confirm_block(new_base/_base::kBlockSize);
            for (auto c : children) {
                auto target = node;
                da._transition(target, c);
                _arrange_da(da, target, new_base xor c);
            }
        }
    }
    
};

} // namespace sim_ds

#endif /* DoubleArray_hpp */
