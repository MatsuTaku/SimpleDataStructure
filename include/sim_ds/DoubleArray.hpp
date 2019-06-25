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


template <class _Dac>
class _DoubleArrayBlockConstReference;


template <class _Dac>
class _DoubleArrayBlockReference {
protected:
    using _index_type = typename _Dac::_index_type;
    using _block_word_type = typename _Dac::_block_word_type;
    using _block_pointer = typename _Dac::_block_pointer;
    using _const_block_pointer = typename _Dac::_block_pointer;
    
    static constexpr size_t kWordBits = sizeof(_block_word_type)*8;
    static constexpr size_t kBlockSize = _Dac::kBlockSize;
    
    static constexpr size_t kPrevOffset = 0;
    static constexpr size_t kNextOffset = kPrevOffset + 1;
    static constexpr size_t kNumEmptiesOffset = kNextOffset + 1;
    static constexpr size_t kFieldSize = kBlockSize / kWordBits; // 4
    
    static constexpr _block_word_type kFrozenFlag = 1ull << 9;
    static constexpr _block_word_type kNumEmptiesMask = 0x1FF;
    
    _block_pointer block_pointer_;
    
    friend typename _Dac::_self;
    
    friend class _DoubleArrayBlockConstReference<_Dac>;
    
public:
    _DoubleArrayBlockReference& init(_index_type prev, _index_type next) {
        set_prev(prev);
        set_next(next);
        *(basic_ptr()+kNumEmptiesOffset) = kBlockSize;
        bit_util::set256_epi1(1, field_ptr());
        return *this;
    }
    
    _block_pointer basic_ptr() {return block_pointer_ + kFieldSize;}
    
    _const_block_pointer basic_ptr() const {return block_pointer_ + kFieldSize;}
    
    _block_pointer field_ptr() {return block_pointer_;}
    
    _const_block_pointer field_ptr() const {return block_pointer_;}
    
    _index_type prev() const {return _index_type(*(basic_ptr()+kPrevOffset));}
    
    void set_prev(_index_type prev) {
        *(basic_ptr()+kPrevOffset) = prev;
    }
    
    _index_type next() const {return _index_type(*(basic_ptr()+kNextOffset));}
    
    void set_next(_index_type next) {
        *(basic_ptr()+kNextOffset) = next;
    }
    
    bool frozen() const {return *(basic_ptr()+kNumEmptiesOffset) bitand kFrozenFlag;}
    
    void freeze() {
        *(basic_ptr()+kNumEmptiesOffset) |= kFrozenFlag;
    }
    
    void thaw() {
        *(basic_ptr()+kNumEmptiesOffset) &= compl kFrozenFlag;
    }
    
    size_t num_empties() const {return *(basic_ptr()+kNumEmptiesOffset) bitand kNumEmptiesMask;}
    
    bool filled() const {return num_empties() == 0;}
    
    void consume(size_t num = 1) {
        assert(num_empties() >= num);
        auto& t = *(basic_ptr()+kNumEmptiesOffset);
        t = (t bitand compl kNumEmptiesMask) | (num_empties() - num);
        assert(num_empties() == bit_util::popcnt256(field_ptr()));
    }
    
    void refill(size_t num = 1) {
        assert(num_empties() + num <= 256);
        auto& t = *(basic_ptr()+kNumEmptiesOffset);
        t = (t bitand compl kNumEmptiesMask) | (num_empties() + num);
        assert(num_empties() == bit_util::popcnt256(field_ptr()));
    }
    
    bool empty_element_at(size_t index) const {
        return *(field_ptr()+(index/kWordBits)) bitand bit_util::OffsetMask(index%kWordBits);
    }
    
    void freeze_element_at(size_t index) {
        assert(empty_element_at(index));
        *(field_ptr()+(index/kWordBits)) &= compl bit_util::OffsetMask(index%kWordBits);
    }
    
    void thaw_element_at(size_t index) {
        assert(not empty_element_at(index));
        *(field_ptr()+(index/kWordBits)) |= bit_util::OffsetMask(index%kWordBits);
    }
    
protected:
    _DoubleArrayBlockReference(_block_pointer pointer) : block_pointer_(pointer) {}
    
};


template <class _Dac>
class _DoubleArrayBlockConstReference {
protected:
    using _index_type = typename _Dac::_index_type;
    using _block_word_type = typename _Dac::_block_word_type;
    using _block_pointer = typename _Dac::_const_block_pointer;
    
    static constexpr size_t kWordBits = sizeof(_block_word_type)*8;
    static constexpr size_t kBlockSize = _Dac::kBlockSize;
    
    static constexpr size_t kPrevOffset = 0;
    static constexpr size_t kNextOffset = kPrevOffset + 1;
    static constexpr size_t kNumEmptiesOffset = kNextOffset + 1;
    static constexpr size_t kFieldSize = kBlockSize / kWordBits; // 4
    
    static constexpr _block_word_type kFrozenFlag = 1ull << 9;
    static constexpr _block_word_type kNumEmptiesMask = 0x1FF;
    
    _block_pointer block_pointer_;
    
    friend typename _Dac::_self;
    
public:
    _DoubleArrayBlockConstReference(const _DoubleArrayBlockReference<_Dac> x) : block_pointer_(x.block_pointer_) {}
    
    _block_pointer basic_ptr() const {return block_pointer_ + kFieldSize;}
    
    _block_pointer field_ptr() const {return block_pointer_;}
    
    _index_type prev() const {return _index_type(*(basic_ptr()+kPrevOffset));}
    
    _index_type next() const {return _index_type(*(basic_ptr()+kNextOffset));}
    
    bool frozen() const {return *(basic_ptr()+kNumEmptiesOffset) & kFrozenFlag;}
    
    size_t num_empties() const {return *(basic_ptr()+kNumEmptiesOffset) & kNumEmptiesMask;}
    
    bool filled() const {return num_empties() == 0;}
    
    bool empty_element_at(size_t index) const {
        return *(field_ptr()+(index/kWordBits)) bitand bit_util::OffsetMask(index%kWordBits);
    }
    
protected:
    _DoubleArrayBlockConstReference(_block_pointer pointer) : block_pointer_(pointer) {}
    
};


template <class _Dac>
class _DoubleArrayBlockLegacyConstReference;


template <class _Dac>
class _DoubleArrayBlockLegacyReference : public _DoubleArrayBlockReference<_Dac> {
    using _base = _DoubleArrayBlockReference<_Dac>;
    using _index_type = typename _base::_index_type;
    using _block_word_type = typename _base::_block_word_type;
    using _block_pointer = typename _base::_block_pointer;
    
    using _inset_type = typename _Dac::_inset_type;
    
    static constexpr _block_word_type kDisabledFlag = 1ull << sizeof(_inset_type)*8;
    static constexpr _block_word_type kEmptyHeadMask = kDisabledFlag-1;
    
    static constexpr size_t kEmptyHeadOffset = _base::kNumEmptiesOffset + 1;
    
    friend typename _Dac::_self;
    
    friend class _DoubleArrayBlockLegacyConstReference<_Dac>;
    
public:
    _DoubleArrayBlockLegacyReference& init(_index_type prev, _index_type next) {
        _base::init(prev, next);
        set_empty_head(0);
        return *this;
    }
    
    _inset_type empty_head() const {
        return *(_base::basic_ptr()+kEmptyHeadOffset) & kEmptyHeadMask;
    }
    
    void set_empty_head(_inset_type empty_head) {
        *(_base::basic_ptr()+kEmptyHeadOffset) = empty_head;
    }
    
    bool link_enabled() const {return not (*(_base::basic_ptr()+kEmptyHeadOffset) bitand kDisabledFlag);}
    
    void disable_link() {
        *(_base::basic_ptr()+kEmptyHeadOffset) = kDisabledFlag;
    }
    
private:
    _DoubleArrayBlockLegacyReference(_block_pointer pointer) : _base(pointer) {}
    
};


template <class _Dac>
class _DoubleArrayBlockLegacyConstReference : public _DoubleArrayBlockConstReference<_Dac> {
    using _base = _DoubleArrayBlockReference<_Dac>;
    using _index_type = typename _base::_index_type;
    using _block_word_type = typename _base::_block_word_type;
    using _block_pointer = typename _base::_block_pointer;
    
    using _inset_type = typename _Dac::_inset_type;
    
    static constexpr _block_word_type kDisabledFlag = 1ull << sizeof(_inset_type)*8;
    static constexpr _block_word_type kEmptyHeadMask = kDisabledFlag-1;
    
    static constexpr size_t kEmptyHeadOffset = _base::kNumEmptiesOffset + 1;
    
    friend typename _Dac::_self;
    
public:
    _DoubleArrayBlockLegacyConstReference(const _DoubleArrayBlockLegacyReference<_Dac> x) : _base(x) {}
    
    _inset_type empty_head() const {
        return *(_base::basic_ptr()+kEmptyHeadOffset) & kEmptyHeadMask;
    }
    
    bool link_enabled() const {return not (*(_base::basic_ptr()+kEmptyHeadOffset) bitand kDisabledFlag);}
    
private:
    _DoubleArrayBlockLegacyConstReference(_block_pointer pointer) : _base(pointer) {}
    
};


template <typename IndexType>
class _DoubleArrayCommon {
public:
    using _self = _DoubleArrayCommon<IndexType>;
    using _index_type = IndexType;
    using _char_type = uint8_t;
    using _unit_type = _DoubleArrayUnit<IndexType>;
    using _block_word_type = uint64_t;
    using _block_pointer = _block_word_type*;
    using _const_block_pointer = const _block_word_type*;
    
    using _block_reference = _DoubleArrayBlockReference<_self>;
    using _block_const_reference = _DoubleArrayBlockConstReference<_self>;
    
    static constexpr _index_type kRootIndex = 0;
    static constexpr _char_type kLeafChar = graph_util::kLeafChar;
    
    static constexpr _index_type kEmptyFlag = _unit_type::kEmptyFlag;
    static constexpr _char_type kEmptyChar = _unit_type::kEmptyChar;
    
    static constexpr size_t kBlockQBytes = 8;
    static constexpr unsigned kBlockSize = 0x100;
    static constexpr _index_type kInitialEmptyBlockHead = std::numeric_limits<_index_type>::max();
    
    static constexpr size_t kIndexBits = _unit_type::kIndexBits;
    
    using _input_trie = graph_util::Trie<char>;
    
protected:
    _index_type empty_block_head_;
    aligned_vector<_block_word_type> basic_block_;
    std::vector<_unit_type> container_;
    std::vector<_char_type> tail_;
    
    _DoubleArrayCommon() : empty_block_head_(kInitialEmptyBlockHead) {}
    
    virtual ~_DoubleArrayCommon() = default;
    
    size_t _size_in_bytes() const {
        return sizeof(empty_block_head_) + size_vec(basic_block_) + size_vec(container_) + size_vec(tail_);
    }
    
    size_t _num_elements() const {return container_.size();}
    
    size_t _num_blocks() const {return basic_block_.size() / kBlockQBytes;}
    
    _block_reference _block_at(_index_type block) {
        assert(block < basic_block_.size());
        return _block_reference(basic_block_.data() + kBlockQBytes * block);
    }
    
    _block_const_reference _block_at(_index_type block) const {
        assert(block < basic_block_.size());
        return _block_const_reference(basic_block_.data() + kBlockQBytes * block);
    }
    
    virtual void _expand() {
        if (container_.size() >= (1ull << (kIndexBits-1))) {
            throw "Index out-of-range! You should set large byte-size of template parameter.";
        }
        container_.resize(container_.size() + kBlockSize);
        
        auto back = _num_blocks();
        basic_block_.resize(basic_block_.size() + kBlockQBytes);
        if (empty_block_head_ == kInitialEmptyBlockHead) {
            _block_at(back).init(back, back);
            empty_block_head_ = back;
        } else {
            auto head_b = _block_at(empty_block_head_);
            auto prev = head_b.prev();
            head_b.set_prev(back);
            _block_at(prev).set_next(back);
            _block_at(back).init(prev, empty_block_head_);
        }
    }
    
    void _freeze_block(_index_type block) {
        auto b = _block_at(block);
        auto next = b.next();
        auto prev = b.prev();
        _block_at(next).set_prev(prev);
        _block_at(prev).set_next(next);
        if (next == block)
            empty_block_head_ = kInitialEmptyBlockHead;
        else if (block == empty_block_head_)
            empty_block_head_ = next;
        
        b.freeze();
    }
    
    void _thaw_block(_index_type block) {
        auto b = _block_at(block);
        if (empty_block_head_ == kInitialEmptyBlockHead) {
            b.set_prev(block);
            b.set_next(block);
            empty_block_head_ = block;
        } else {
            auto front_b = _block_at(empty_block_head_);
            auto back = front_b.prev();
            front_b.set_prev(block);
            _block_at(back).set_next(block);
            b.set_prev(back);
            b.set_next(empty_block_head_);
            if (block < empty_block_head_)
                empty_block_head_ = block;
        }
        
        b.thaw();
    }
    
    void _confirm_freeze_block(_index_type block) {
        auto b = _block_at(block);
        if (b.filled() and not b.frozen())
            _freeze_block(block);
    }
    
    void _confirm_thaw_block(_index_type block) {
        auto b = _block_at(block);
        if (b.frozen() and not b.filled())
            _thaw_block(block);
    }
    
    bool _empty(size_t index) const {
        return _block_at(index/kBlockSize).empty_element_at(index%kBlockSize);
    }
    
    virtual void _freeze(size_t index) {
        assert(_empty(index));
        _block_at(index/kBlockSize).freeze_element_at(index%kBlockSize);
    }
    
    virtual void _thaw(size_t index) {
        assert(not _empty(index));
        _block_at(index/kBlockSize).thaw_element_at(index%kBlockSize);
    }
    
    _index_type _check(_index_type index) const {
        return container_[index].check();
    }
    
    void _set_check(size_t index, _index_type check) {
        container_[index].set_check(check);
    }
    
    _char_type _sibling(_index_type index) const {
        assert(container_[index].sibling() != kEmptyChar);
        return container_[index].sibling();
    }
    
    void _set_sibling(_index_type index, _char_type sibling) {
        container_[index].set_sibling(sibling);
    }
    
    void _set_check_sibling(_index_type index, _index_type check, _char_type sibling) {
        _set_check(index, check);
        _set_sibling(index, sibling);
    }
    
    _index_type _base(_index_type node) const {
        return container_[node].base();
    }
    
    void _set_base(_index_type node, _index_type base) {
        container_[node].set_base(base);
    }
    
    _char_type _child(_index_type node) const {
        return container_[node].child();
    }
    
    void _set_child(_index_type node, _char_type c) {
        container_[node].set_child(c);
    }
    
    bool _leaf(_index_type node) const {
        return _base(node) bitand kEmptyFlag;
    }
    
    void _set_base_child(_index_type node, _index_type base, _char_type child) {
        assert(not _empty(node));
        _set_base(node, base);
        _set_child(node, child);
    }
    
    _index_type _tail_index(_index_type node) const {
        assert(_leaf(node));
        return _base(node) bitand compl kEmptyFlag;
    }
    
    void _set_tail_index(_index_type node, _index_type tail_index) {
        assert(_leaf(node));
        _set_base(node, tail_index bitor kEmptyFlag);
    }
    
    std::string_view _suffix_in_tail(_index_type tail_index) const {
        return std::string_view((char*)tail_.data() + tail_index);
    }
    
    // Must call after thaw target element.
    void _clean(size_t index) {
        container_[index].init();
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
        _set_base_child(node, base, c);
        _setup(next);
        _set_check_sibling(next, node, c);
        _block_at(base/kBlockSize).consume();
        _confirm_freeze_block(base/kBlockSize);
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
        auto old_base = _base(node);
        _set_base(node, new_base);
        for (size_t i = 0; i < children.size(); i++) {
            auto child = children[i];
            auto sibling = children[(i+1)%children.size()];
            auto old_next = old_base xor child;
            auto new_next = new_base xor child;
            _setup(new_next);
            _set_check_sibling(new_next, node, sibling);
            auto next_base = luggages[i].base;
            if (not (next_base & kEmptyFlag)) { // In BC
                auto grand_first_child = luggages[i].child;
                _set_base_child(new_next, next_base, grand_first_child);
                for (auto grand_child = grand_first_child; ; ) {
                    _set_check(next_base xor grand_child, new_next);
                    auto sibling = _sibling(next_base xor grand_child);
                    if (sibling == grand_first_child)
                        break;
                    grand_child = sibling;
                }
            } else { // In tail
                _set_tail_index(new_next, next_base bitand compl kEmptyFlag);
            }
            if (old_next == monitoring_node) {
                monitoring_node = new_next;
            }
        }
        _block_at(new_base/kBlockSize).consume(children.size());
        _confirm_freeze_block(new_base/kBlockSize);
        
        return monitoring_node;
    }
    
    _index_type _solve_collision(_index_type node, _char_type c) {
        auto base = _base(node);
        auto conflicting_index = base xor c;
        auto competitor = _check(conflicting_index);
        if (conflicting_index != kRootIndex and
            _num_of_children(competitor) <= _num_of_children(node)) {
            auto competitor_base = _base(competitor);
#ifndef NDEBUG
            bool hit = false;
#endif
            std::vector<_char_type> children;
            std::vector<_moving_luggage> luggages;
            _for_each_children(competitor, [&](auto child, auto) {
                children.push_back(child);
                auto next = competitor_base xor child;
                luggages.emplace_back(_base(next), _child(next));
                _erase(next);
#ifndef NDEBUG
                hit |= next == conflicting_index;
#endif
            });
#ifndef NDEBUG
            assert(hit);
#endif
            _freeze(conflicting_index);
            auto block_i = competitor_base/kBlockSize;
            _block_at(block_i).refill(children.size()-1);
            _confirm_thaw_block(block_i);
            auto new_base = _find_base(children);
            _thaw(conflicting_index);
            assert(_block_at(block_i).empty_element_at(conflicting_index%256));
            _block_at(block_i).refill();
            node = _move_nodes(competitor, children, luggages, new_base, node);
        } else {
            std::vector<_char_type> children;
            std::vector<_moving_luggage> luggages;
            _for_each_children(node, [&](auto child, auto) {
                children.push_back(child);
                auto next = base xor child;
                luggages.emplace_back(_base(next), _child(next));
                _erase(next);
            });
            _block_at(base/kBlockSize).refill(children.size());
            _confirm_thaw_block(base/kBlockSize);
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
        
        _block_at(base/kBlockSize).consume();
        _confirm_freeze_block(base/kBlockSize);
        
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
        if (tail_index < tail_.size()) {
            while (tail_index < tail_pos) {
                auto c = tail_[tail_index++];
                node = _grow(node, _find_base({c}), c);
            }
            auto c = tail_[tail_index];
            auto next = _grow(node, _find_base({c}), c);
            _set_tail_index(next, tail_index+1);
        }
        _insert_in_bc(node, suffix);
    }
    
};


template <typename IndexType, bool LegacyBuild>
class _DoubleArrayBase;


template <typename IndexType>
class _DoubleArrayBase<IndexType, true> : protected _DoubleArrayCommon<IndexType> {
public:
    using _self = _DoubleArrayBase<IndexType, true>;
    using _common = _DoubleArrayCommon<IndexType>;
    using _index_type = typename _common::_index_type;
    using _char_type = typename _common::_char_type;
    using _inset_type = uint8_t;
    
    using _block_word_type = typename _common::_block_word_type;
    using _block_pointer = typename _common::_block_pointer;
    using _const_block_pointer = typename _common::_const_block_pointer;
    
    using _block_reference = _DoubleArrayBlockLegacyReference<_self>;
    using _const_block_reference = _DoubleArrayBlockLegacyConstReference<_self>;
    
    static constexpr _index_type kRootIndex = _common::kRootIndex;
    
    static constexpr size_t kBlockQBytes = _common::kBlockQBytes;
    static constexpr unsigned kBlockSize = _common::kBlockSize;
    static constexpr _index_type kInitialEmptyBlockHead = _common::kInitialEmptyBlockHead;
    
protected:
    _DoubleArrayBase() : _common() {
        _expand();
        _common::_setup(kRootIndex);
        _common::_set_check_sibling(kRootIndex, 0, 0); // set root
        _block_at(kRootIndex/kBlockSize).consume();
    }
    
    virtual ~_DoubleArrayBase() = default;
    
    size_t _size_in_bytes() const {
        return _common::_size_in_bytes();
    }
    
    _block_reference _block_at(_index_type block) {
        return _block_reference(_common::basic_block_.data() + kBlockQBytes * block);
    }
    
    _const_block_reference _block_at(_index_type block) const {
        return _const_block_reference(_common::basic_block_.data() + kBlockQBytes * block);
    }
    
    _index_type _next(size_t index) const {
        assert(_common::_empty(index));
        return _common::_base(index) bitand compl(_common::kEmptyFlag);
    }
    
    void _set_next(size_t index, _inset_type next) {
        assert(_common::_empty(index));
        _common::_set_base(index, next bitor _common::kEmptyFlag);
    }
    
    _index_type _prev(size_t index) const {
        assert(_common::_empty(index));
        return _common::_check(index) bitand compl(_common::kEmptyFlag);
    }
    
    void _set_prev(size_t index, _inset_type prev) {
        assert(_common::_empty(index));
        _common::_set_check(index, prev bitor _common::kEmptyFlag);
    }
    
    void _freeze(size_t index) override {
        auto next = _next(index);
        auto prev = _prev(index);
        auto offset = kBlockSize * (index/kBlockSize);
        _set_prev(offset+next, prev);
        _set_next(offset+prev, next);
        auto b = _block_at(index/kBlockSize);
        auto inset = index % kBlockSize;
        if (next == inset) {
            b.disable_link();
        } else if (inset == b.empty_head()) {
            b.set_empty_head(next);
        }
        
        _common::_freeze(index);
    }
    
    void _thaw(size_t index) override {
        _common::_thaw(index);
        
        auto b = _block_at(index/kBlockSize);
        auto inset = index % kBlockSize;
        if (not b.link_enabled()) {
            _set_next(index, inset);
            _set_prev(index, inset);
            b.set_empty_head(inset);
        } else {
            auto offset = kBlockSize * (index/kBlockSize);
            auto head = b.empty_head();
            auto tail = _prev(offset+head);
            _set_next(offset+tail, inset);
            _set_prev(offset+head, inset);
            _set_next(index, head);
            _set_prev(index, tail);
        }
    }
    
    void _expand() override {
        auto front = _common::_num_elements();
        _common::_expand();
        _block_at(front/kBlockSize).set_empty_head(0);
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
            auto block = _block_at(b);
            assert(block.link_enabled());
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
                b = _block_at(b).next();
            } else {
                b = block.next();
            }
        }
        throw "Not found base!";
    }
    
};


template <typename IndexType>
class _DoubleArrayBase<IndexType, false> : protected _DoubleArrayCommon<IndexType> {
public:
    using _self = _DoubleArrayBase<IndexType, false>;
    using _common = _DoubleArrayCommon<IndexType>;
    using _index_type = typename _common::_index_type;
    using _char_type = typename _common::_char_type;
    using _block_word_type = typename _common::_block_word_type;
    using _block_pointer = typename _common::_block_pointer;
    using _const_block_pointer = typename _common::_const_block_pointer;
    
    using _block_reference = _DoubleArrayBlockReference<_self>;
    using _const_block_reference = _DoubleArrayBlockConstReference<_self>;
    
    static constexpr _index_type kRootIndex = _common::kRootIndex;
    
    static constexpr size_t kBlockQBytes = _common::kBlockQBytes;
    static constexpr unsigned kBlockSize = _common::kBlockSize;
    static constexpr _index_type kInitialEmptyBlockHead = _common::kInitialEmptyBlockHead;
    
protected:
    _DoubleArrayBase() : _common() {
        _common::_expand();
        _common::_setup(kRootIndex);
        _common::_set_check_sibling(kRootIndex, 0, 0); // set root
        _block_at(0).consume();
    }
    
    virtual ~_DoubleArrayBase() = default;
    
    size_t _size_in_bytes() const {
        return _common::_size_in_bytes();
    }
    
    _block_reference _block_at(_index_type block) {
        return _block_reference(_common::basic_block_.data() + kBlockQBytes * block);
    }
    
    _const_block_reference _block_at(_index_type block) const {
        return _const_block_reference(_common::basic_block_.data() + kBlockQBytes * block);
    }
    
    _index_type _find_base(const std::vector<_char_type>& children) override {
        size_t b = _common::empty_block_head_;
        if (b == kInitialEmptyBlockHead) {
            _common::_expand();
            b = _common::empty_block_head_;
        }
        if (children.size() == 1) {
            return kBlockSize * b + (bit_util::ctz256(_block_at(b).field_ptr()) xor children.front());
        } else {
            while (true) {
                auto block = _block_at(b);
                alignas(32) uint64_t field[4];
                bit_util::set256_epi1(1, field);
                for (auto c : children) {
                    bit_util::mask_xor_idx_and256(field, _block_at(b).field_ptr(), c, field);
                }
                auto ctz = bit_util::ctz256(field);
                if (ctz < kBlockSize) {
                    return kBlockSize * b + ctz;
                }
                if (block.next() == _common::empty_block_head_) {
                    _common::_expand();
                    b = _block_at(b).next();
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
    
    static constexpr size_t kBlockSize = _base::kBlockSize;
    
    DoubleArray() : _base() {}
    
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
                if (tail_index >= _base::tail_.size() or
                    _base::tail_[tail_index] != char_type(key[pos])) {
                    char_type e,cc;
                    if (tail_index < _base::tail_.size()) {
                        e = _base::tail_[tail_index];
                        cc = key[pos];
                    }
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
            return;
        }
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
        auto block_i = new_base/kBlockSize;
        _base::_block_at(block_i).consume(children.size());
        _base::_confirm_freeze_block(block_i);
        for (auto c : children) {
            auto target = node;
            da._transition(target, c);
            _arrange_da(da, target, new_base xor c);
        }
    }
    
};

} // namespace sim_ds

#endif /* DoubleArray_hpp */
