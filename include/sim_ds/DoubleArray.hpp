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
#include "da_util.hpp"

namespace sim_ds {


template <class IndexType>
class _DoubleArrayBCUnit {
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
    _DoubleArrayBCUnit() : check_(kEmptyFlag), sibling_(kEmptyChar), base_(kEmptyFlag), child_(kEmptyChar) {}
    
    ~_DoubleArrayBCUnit() = default;
    
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
    static constexpr size_t kErrorCountOffset = kNumEmptiesOffset;
    static constexpr size_t kErrorCountInset = 32;
    static constexpr size_t kFieldSize = kBlockSize / kWordBits; // 4
    
    static constexpr _block_word_type kNumEmptiesMask = 0x1FF;
    
    _block_pointer block_pointer_;
    
    friend typename _Dac::_self;
    
    friend class _DoubleArrayBlockConstReference<_Dac>;
    
public:
    _DoubleArrayBlockReference& init(_index_type prev, _index_type next) {
        set_prev(prev);
        set_next(next);
        *(basic_ptr()+kNumEmptiesOffset) = kBlockSize;
        error_reset();
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
    
    size_t error_count() const {return (*(basic_ptr()+kErrorCountOffset) >> kErrorCountInset);}
    
    void errored() {
        auto& v = *(basic_ptr()+kErrorCountOffset) ;
        v = ((v bitand bit_util::width_mask<kErrorCountInset>) bitor
             ((_block_word_type)(error_count()+1) << kErrorCountInset));
    }
    
    void error_reset() {
        *(basic_ptr()+kErrorCountOffset) &= bit_util::width_mask<kErrorCountInset>;
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
    static constexpr size_t kErrorCountOffset = kNumEmptiesOffset;
    static constexpr size_t kErrorCountInset = 32;
    static constexpr size_t kFieldSize = kBlockSize / kWordBits; // 4
    
    static constexpr _block_word_type kNumEmptiesMask = 0x1FF;
    
    _block_pointer block_pointer_;
    
    friend typename _Dac::_self;
    
public:
    _DoubleArrayBlockConstReference(const _DoubleArrayBlockReference<_Dac> x) : block_pointer_(x.block_pointer_) {}
    
    _block_pointer basic_ptr() const {return block_pointer_ + kFieldSize;}
    
    _block_pointer field_ptr() const {return block_pointer_;}
    
    _index_type prev() const {return _index_type(*(basic_ptr()+kPrevOffset));}
    
    _index_type next() const {return _index_type(*(basic_ptr()+kNextOffset));}
    
    size_t num_empties() const {return *(basic_ptr()+kNumEmptiesOffset) & kNumEmptiesMask;}
    
    size_t error_count() const {return (*(basic_ptr()+kErrorCountOffset) >> kErrorCountInset);}
    
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
    using _base = _DoubleArrayBlockConstReference<_Dac>;
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
class _DoubleArrayBCImpl {
public:
    using _self = _DoubleArrayBCImpl<IndexType>;
    using _index_type = IndexType;
    using _char_type = uint8_t;
    using _unit_type = _DoubleArrayBCUnit<IndexType>;
    using _inset_type = uint8_t;
    using _block_word_type = uint64_t;
    using _block_pointer = _block_word_type*;
    using _const_block_pointer = const _block_word_type*;
    
    using _block_reference = _DoubleArrayBlockLegacyReference<_self>;
    using _block_const_reference = _DoubleArrayBlockLegacyConstReference<_self>;
    
    static constexpr _index_type kRootIndex = 0;
    static constexpr _char_type kLeafChar = graph_util::kLeafChar;
    
    static constexpr _index_type kEmptyFlag = _unit_type::kEmptyFlag;
    static constexpr _char_type kEmptyChar = _unit_type::kEmptyChar;
    
    static constexpr size_t kBlockQBytes = 8;
    static constexpr unsigned kBlockSize = 0x100;
    static constexpr _index_type kInitialEmptyBlockHead = std::numeric_limits<_index_type>::max();
    
    static constexpr size_t kIndexBits = _unit_type::kIndexBits;
    
    using _input_trie = graph_util::Trie<char>;
    
    static constexpr size_t kErrorThreshold = 32;
    
protected:
    _index_type general_block_head_;
    _index_type personal_block_head_;
    aligned_vector<_block_word_type, 32> basic_block_;
    std::vector<_unit_type> container_;
    std::vector<_char_type> tail_;
    
    _DoubleArrayBCImpl() : general_block_head_(kInitialEmptyBlockHead), personal_block_head_(kInitialEmptyBlockHead) {
        _expand();
        _setup(kRootIndex);
        _set_check_sibling(kRootIndex, 0, 0); // set root
        _consume_block(_block_index_of(kRootIndex), 1);
    }
    
    virtual ~_DoubleArrayBCImpl() = default;
    
    size_t _size_in_bytes() const {
        return sizeof(general_block_head_) + size_vec(basic_block_) + size_vec(container_) + size_vec(tail_);
    }
    
    size_t _num_elements() const {return container_.size();}
    
    size_t _num_blocks() const {return basic_block_.size() / kBlockQBytes;}
    
    _index_type _block_index_of(_index_type index) const {return index/kBlockSize;}
    
    _block_reference _block_at(_index_type block) {
        assert(block < _num_blocks());
        return _block_reference(basic_block_.data() + kBlockQBytes * block);
    }
    
    _block_const_reference _block_at(_index_type block) const {
        assert(block < _num_blocks());
        return _block_const_reference(basic_block_.data() + kBlockQBytes * block);
    }
    
    void _expand() {
        if (container_.size() >= (1ull << (kIndexBits-1))) {
            throw "Index out-of-range! You should set large byte-size of template parameter.";
        }
        container_.resize(container_.size() + kBlockSize);
        
        // Append blocks linking
        basic_block_.resize(basic_block_.size() + kBlockQBytes);
        auto back = _num_blocks() - 1;
        if (general_block_head_ == kInitialEmptyBlockHead) {
            _block_at(back).init(back, back);
            general_block_head_ = back;
        } else {
            auto prev = _block_at(general_block_head_).prev();
            _block_at(general_block_head_).set_prev(back);
            _block_at(prev).set_next(back);
            _block_at(back).init(prev, general_block_head_);
        }
        
        // Link elements in appended block
        auto front = container_.size() - kBlockSize;
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
    
    void _shrink() {
        if (_num_blocks() == 1)
            return;
        assert(_block_at(_num_elements()-1).num_empties() == kBlockSize);
        container_.resize(container_.size() - kBlockSize);
        basic_block_.resize(basic_block_.size() - kBlockQBytes);
    }
    
    bool _empty(size_t index) const {
        return _block_at(_block_index_of(index)).empty_element_at(index%kBlockSize);
    }
    
    _index_type _next(size_t index) const {
        assert(_empty(index));
        return _base(index) bitand compl(kEmptyFlag);
    }
    
    void _set_next(size_t index, _inset_type next) {
        assert(_empty(index));
        _set_base(index, next bitor kEmptyFlag);
    }
    
    _index_type _prev(size_t index) const {
        assert(_empty(index));
        return _check(index) bitand compl(kEmptyFlag);
    }
    
    void _set_prev(size_t index, _inset_type prev) {
        assert(_empty(index));
        _set_check(index, prev bitor kEmptyFlag);
    }
    
    void _freeze(size_t index) {
        assert(_empty(index));
        // Delete empty-elements linking
        auto next = _next(index);
        auto prev = _prev(index);
        auto block_index = _block_index_of(index);
        auto offset = kBlockSize * (block_index);
        _set_prev(offset+next, prev);
        _set_next(offset+prev, next);
        auto b = _block_at(block_index);
        auto inset = index % kBlockSize;
        if (next == inset) {
            b.disable_link();
        } else if (inset == b.empty_head()) {
            b.set_empty_head(next);
        }
        
        _block_at(_block_index_of(index)).freeze_element_at(index%kBlockSize);
    }
    
    void _thaw(size_t index) {
        assert(not _empty(index));
        auto block_index = _block_index_of(index);
        _block_at(block_index).thaw_element_at(index%kBlockSize);
        
        // Append empty-elements linking
        auto b = _block_at(block_index);
        auto inset = index % kBlockSize;
        if (not b.link_enabled()) {
            _set_next(index, inset);
            _set_prev(index, inset);
            b.set_empty_head(inset);
        } else {
            auto offset = kBlockSize * (block_index);
            auto head = b.empty_head();
            auto tail = _prev(offset+head);
            _set_next(offset+tail, inset);
            _set_prev(offset+head, inset);
            _set_next(index, head);
            _set_prev(index, tail);
        }
    }
    
    void _setup(size_t index) {
        _freeze(index);
        container_[index].init();
    }
    
    void _erase(size_t index) {
        container_[index].init();
        _thaw(index);
    }
    
    _index_type _check(_index_type index) const {return container_[index].check();}
    
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
    
    _index_type _base(_index_type node) const {return container_[node].base();}
    
    void _set_base(_index_type node, _index_type base) {
        container_[node].set_base(base);
    }
    
    _char_type _child(_index_type node) const {return container_[node].child();}
    
    void _set_child(_index_type node, _char_type c) {
        container_[node].set_child(c);
    }
    
    bool _is_leaf(_index_type node) const {return _base(node) bitand kEmptyFlag;}
    
    void _set_base_child(_index_type node, _index_type base, _char_type child) {
        assert(not _empty(node));
        _set_base(node, base);
        _set_child(node, child);
    }
    
    _index_type _tail_index(_index_type node) const {
        assert(_is_leaf(node));
        return _base(node) bitand compl kEmptyFlag;
    }
    
    void _set_tail_index(_index_type node, _index_type tail_index) {
        assert(_is_leaf(node));
        _set_base(node, tail_index bitor kEmptyFlag);
    }
    
    std::string_view _suffix_in_tail(_index_type tail_index) const {
        return std::string_view((char*)tail_.data() + tail_index);
    }
    
    void _freeze_block_in_general(_index_type block) {
        assert(_block_at(block).error_count() < kErrorThreshold);
        auto next = _block_at(block).next();
        if (next == block) {
            assert(general_block_head_ == block);
            general_block_head_ = kInitialEmptyBlockHead;
            return;
        }
        auto prev = _block_at(block).prev();
        assert(prev != block);
        _block_at(next).set_prev(prev);
        _block_at(prev).set_next(next);
        if (block == general_block_head_) {
            general_block_head_ = next;
        }
    }
    
    void _freeze_block_in_personal(_index_type block) {
        assert(_block_at(block).error_count() >= kErrorThreshold);
        auto next = _block_at(block).next();
        if (next == block) {
            assert(personal_block_head_ == block);
            personal_block_head_ = kInitialEmptyBlockHead;
            return;
        }
        auto prev = _block_at(block).prev();
        assert(prev != block);
        _block_at(next).set_prev(prev);
        _block_at(prev).set_next(next);
        if (block == personal_block_head_) {
            personal_block_head_ = next;
        }
    }
    
    void _modify_block_to_general(_index_type block) {
        if (general_block_head_ == kInitialEmptyBlockHead) {
            _block_at(block).set_next(block);
            _block_at(block).set_prev(block);
            general_block_head_ = block;
        } else {
            auto tail = _block_at(general_block_head_).prev();
            assert(tail != block);
            _block_at(general_block_head_).set_prev(block);
            _block_at(tail).set_next(block);
            _block_at(block).set_next(general_block_head_);
            _block_at(block).set_prev(tail);
        }
    }
    
    void _modify_block_to_personal(_index_type block) {
        if (personal_block_head_ == kInitialEmptyBlockHead) {
            _block_at(block).set_next(block);
            _block_at(block).set_prev(block);
            personal_block_head_ = block;
        } else {
            auto tail = _block_at(personal_block_head_).prev();
            _block_at(personal_block_head_).set_prev(block);
            _block_at(tail).set_next(block);
            _block_at(block).set_next(personal_block_head_);
            _block_at(block).set_prev(tail);
        }
    }
    
    void _error_block(_index_type block) {
        assert(_block_at(block).error_count() < kErrorThreshold);
        if (_block_at(block).error_count() + 1 >= kErrorThreshold) {
            _freeze_block_in_general(block);
            _modify_block_to_personal(block);
        }
        _block_at(block).errored();
    }
    
    void _consume_block(_index_type block, size_t num) {
        auto b = _block_at(block);
        b.consume(num);
        if (b.filled()) {
            if (b.error_count() >= kErrorThreshold) {
                _freeze_block_in_personal(block);
            } else {
                _freeze_block_in_general(block);
            }
        }
    }
    
    void _refill_block(_index_type block, size_t num) {
        if (num == 0)
            return;
        if (_block_at(block).filled()) {
            _modify_block_to_general(block);
        } else if (_block_at(block).error_count() >= kErrorThreshold) {
            _freeze_block_in_personal(block);
            _modify_block_to_general(block);
        }
        _block_at(block).error_reset();
        _block_at(block).refill(num);
    }
    
    template <class Action>
    void _for_each_children(_index_type node, Action action) {
        assert(not _is_leaf(node));
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
        assert(not _is_leaf(node));
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
    
    _index_type _new_base(_char_type c) const {
        return _num_elements() xor c;
    }
    
    void _insert_nodes(_index_type node, std::vector<_char_type>& children, _index_type base) {
        if (base >= _num_elements())
            _expand();
        _set_base_child(node, base, children.front());
        for (size_t i = 0; i < children.size(); i++) {
            auto c = children[i];
            auto next = base xor c;
            _setup(next);
            _set_check_sibling(next, node, children[(i+1)%children.size()]);
        }
        _consume_block(_block_index_of(base), children.size());
    }
    
    _index_type _insert_suffix(std::string_view suffix) {
        _index_type index = tail_.size();
        for (_char_type c : suffix)
            tail_.push_back(c);
        tail_.push_back(kLeafChar);
        return index;
    }
    
    virtual _index_type _find_base(const std::vector<_char_type>& children) {
        if (children.size() == 1 and personal_block_head_ != kInitialEmptyBlockHead) {
            _index_type pbh = personal_block_head_;
            return (kBlockSize * pbh + _block_at(pbh).empty_head()) xor children.front();
        }
        
        if (general_block_head_ == kInitialEmptyBlockHead) {
            return _new_base(children.front());
        }
        _index_type b = general_block_head_;
        while (true) {
            auto block = _block_at(b);
            assert(block.link_enabled());
            if (block.num_empties() >= children.size()) {
                const auto offset = kBlockSize * b;
                for (auto index = offset + block.empty_head(); ; ) {
                    _index_type n = index xor children.front();
                    bool skip = false;
                    for (_char_type c : children) {
                        if (not _empty(n xor c)) {
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
            }
            auto new_b = block.next();
            if (new_b == general_block_head_) {
                return _new_base(children.front());
            }
            _error_block(b);
            b = new_b;
        }
        throw "Not found base!";
    }
    
};


template <typename IndexType>
class _DynamicDoubleArrayBCImpl : protected _DoubleArrayBCImpl<IndexType> {
public:
    using _static_impl = _DoubleArrayBCImpl<IndexType>;
    using _index_type = typename _static_impl::_index_type;
    using _char_type = typename _static_impl::_char_type;
    
    using _static_impl::kBlockSize;
    using _static_impl::kEmptyFlag;
    using _static_impl::kRootIndex;
    using _static_impl::kInitialEmptyBlockHead;
    using _static_impl::kLeafChar;
    using _static_impl::kEmptyChar;
    
protected:
    _DynamicDoubleArrayBCImpl() : _static_impl() {}
    
    virtual ~_DynamicDoubleArrayBCImpl() = default;
    
    _index_type _grow(_index_type node, _index_type base, _char_type c) {
        if (base >= _static_impl::_num_elements())
            _static_impl::_expand();
        assert(_static_impl::_is_leaf(node));
        auto next = base xor c;
        assert(_static_impl::_empty(next));
        _static_impl::_set_base_child(node, base, c);
        _static_impl::_setup(next);
        _static_impl::_set_check_sibling(next, node, c);
        _static_impl::_consume_block(base/kBlockSize, 1);
        return next;
    }
    
    void _insert_in_bc(_index_type node, std::string_view suffix) {
        if (suffix.size() > 0) {
            node = _insert_trans(node, suffix.front());
            auto tail_index = _static_impl::_insert_suffix(suffix.size() > 1 ? suffix.substr(1) : "");
            _static_impl::_set_tail_index(node, tail_index);
        } else {
            node = _insert_trans(node, kLeafChar);
        }
    }
    
    void _insert_in_tail(_index_type node, _index_type tail_pos, std::string_view suffix) {
        auto tail_index = _static_impl::_tail_index(node);
        if (tail_index < _static_impl::tail_.size()) {
            while (tail_index < tail_pos) {
                auto c = _static_impl::tail_[tail_index++];
                node = _grow(node, this->_find_base({c}), c);
            }
            auto c = _static_impl::tail_[tail_index];
            auto next = _grow(node, this->_find_base({c}), c);
            _static_impl::_set_tail_index(next, tail_index+1);
        }
        _insert_in_bc(node, suffix);
    }
    
    void _delete_leaf(_index_type node) {
        assert(_is_leaf(node));
        size_t counts_children;
        do {
            _index_type prev = _check(node);
            counts_children = _num_of_children(prev);
            // Erase current char from siblings link.
            auto base = _base(prev);
            _char_type first_child = _child(prev);
            if ((base xor first_child) == node) {
                _char_type child = _sibling(base xor first_child);
                _index_type tail_index = base xor first_child;
                while (child != first_child) {
                    tail_index = base xor child;
                    child = _sibling(tail_index);
                }
                _char_type second_child = _sibling(base xor first_child);
                _set_sibling(tail_index, second_child);
                _set_child(prev, counts_children != 1 ? second_child : kEmptyChar);
            } else {
                _index_type prev_index = base xor first_child;
                for (_char_type child = _sibling(prev_index); ; ) {
                    auto next = base xor child;
                    auto sibling = _sibling(next);
                    if (next == node) {
                        _set_sibling(prev_index, sibling);
                        break;
                    }
                    child = sibling;
                }
            }
            
            _erase(node);
            node = prev;
        } while (node != kRootIndex and counts_children == 1);
        _reduce();
    }
    
private:
    struct _moving_luggage {
        _index_type base;
        uint8_t child;
        _moving_luggage(_index_type base, uint8_t child) : base(base), child(child) {}
    };
    
    struct _shelter {
        _index_type node;
        std::vector<_char_type> children;
        std::vector<_moving_luggage> luggages;
    };
    
    void _evacuate(_index_type node, _shelter& shelter) {
        shelter.node = node;
        auto base = _static_impl::_base(node);
        _static_impl::_for_each_children(node, [&](auto child, auto) {
            shelter.children.push_back(child);
            auto index = base xor child;
            shelter.luggages.emplace_back(_static_impl::_base(index), _static_impl::_child(index));
            _static_impl::_erase(index);
        });
        _static_impl::_refill_block(_static_impl::_block_index_of(base), shelter.children.size());
    }
    
    void _update_node(_index_type index, _index_type check, _index_type sibling, _index_type base, _index_type child) {
        assert(_static_impl::_empty(index));
        _static_impl::_setup(index);
        _static_impl::_set_check_sibling(index, check, sibling);
        if (not (base & kEmptyFlag)) { // In BC
            _static_impl::_set_base_child(index, base, child);
            for (auto c = child; ; ) {
                auto cur_index = base xor c;
                _static_impl::_set_check(cur_index, index);
                auto sibling = _static_impl::_sibling(cur_index);
                if (sibling == child)
                    break;
                c = sibling;
            }
        } else { // In TAIL
            _static_impl::_set_tail_index(index, base bitand compl kEmptyFlag);
        }
    }
    
    _index_type _move_nodes(_shelter& shelter, _index_type new_base,
                            _index_type monitoring_node = kRootIndex) {
        if (new_base >= _static_impl::_num_elements())
            _static_impl::_expand();
        auto old_base = _static_impl::_base(shelter.node);
        _static_impl::_set_base(shelter.node, new_base);
        for (size_t i = 0; i < shelter.children.size(); i++) {
            auto child = shelter.children[i];
            auto sibling = shelter.children[(i+1)%shelter.children.size()];
            auto new_next = new_base xor child;
            _update_node(new_next, shelter.node, sibling, shelter.luggages[i].base, shelter.luggages[i].child);
            if ((old_base xor child) == monitoring_node) {
                monitoring_node = new_next;
            }
        }
        _static_impl::_consume_block(_static_impl::_block_index_of(new_base), shelter.children.size());
        
        return monitoring_node;
    }
    
    void _compress_nodes(_shelter& shelter, _index_type target_base) {
        auto old_base = _base(shelter.node);
        _set_base(shelter.node, target_base);
        std::vector<_shelter> shelters;
        for (size_t i = 0; i < shelter.children.size(); i++) {
            auto child = shelter.children[i];
            auto sibling = shelter.children[(i+1)%shelter.children.size()];
            auto new_next = target_base xor child;
            if (not _empty(new_next)) {
                // Evacuate already placed siblings membering element at conflicting index to shelter.
                shelters.emplace_back();
                auto conflicting_node = _check(new_next);
                _evacuate(conflicting_node, shelters.back());
            }
            _update_node(new_next, shelter.node, sibling, shelter.luggages[i].base, shelter.luggages[i].child);
        }
        _consume_block(_block_index_of(target_base), shelter.children.size());
        
        // Compress sheltered siblings recursively.
        for (auto& sht : shelters) {
            _compress_nodes(sht.check, sht.children, sht.luggages, _find_compression_target(sht.children));
        }
        
        return;
    }
    
    _index_type _find_compression_target(std::vector<_char_type>& siblings) const {
        if (siblings.size() == 1 and _static_impl::personal_block_head_ != kInitialEmptyBlockHead) {
            _index_type pbh = _static_impl::personal_block_head_;
            return (kBlockSize * pbh + _block_at(pbh).empty_head()) xor siblings.front();
        }
        if (_static_impl::general_block_head_ == kInitialEmptyBlockHead) {
            return _static_impl::_num_elements();
        }
        _index_type b = _static_impl::general_block_head_;
        while (true) {
            if (b == _block_index_of(_static_impl::_num_elements()-1)) {
                b = _block_at(_block_index_of(_static_impl::_num_elements()-1)).next();
                if (b == _static_impl::general_block_head_)
                    return _static_impl::_num_elements();
                continue;
            }
            auto block = _block_at(b);
            assert(block.link_enabled());
            const auto offset = kBlockSize * b;
            for (auto index = offset + block.empty_head(); ; ) {
                _index_type n = index xor siblings.front();
                bool skip = false;
                for (_char_type c : siblings) {
                    auto target = n xor c;
                    if (not _empty(target) and
                        _num_of_children(_check(target)) >= siblings.size()) {
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
            if (block.next() == _static_impl::general_block_head_) {
                return _static_impl::_num_elements();
            }
            b = block.next();
        }
    }
    
    void _reduce() {
        using bit_util::ctz256;
        for (_index_type b = _block_index_of(_static_impl::_num_elements()-1); b > 0; b--) {
            for (size_t ctz = ctz256(_block_at(b).field_ptr()); ctz < 256; ctz = ctz256(_block_at(b).field_ptr())) {
                std::vector<_char_type> children;
                auto parent = _check(kBlockSize*b+ctz);
                auto base = _base(parent);
                _for_each_children(parent, [&](auto child, auto) {
                    children.push_back(child);
                });
                auto compression_target = _find_compression_target(children);
                if (_block_index_of(compression_target) >= b)
                    return;
                std::vector<_moving_luggage> luggages;
                _for_each_children(parent, [&](auto child, auto) {
                    auto next = base xor child;
                    luggages.emplace_back(_base(next), _child(next));
                    _erase(next);
                });
                _compress_nodes(parent, children, luggages, compression_target);
            }
            _static_impl::_shrink();
        }
    }
    
    _index_type _solve_collision(_index_type node, _char_type c) {
        auto base = _static_impl::_base(node);
        auto conflicting_index = base xor c;
        auto competitor = _static_impl::_check(conflicting_index);
        if (conflicting_index != kRootIndex and
            _static_impl::_num_of_children(competitor) <= _static_impl::_num_of_children(node)) {
            _shelter shelter;
            _evacuate(competitor, shelter);
            _static_impl::_freeze(conflicting_index);
            auto new_base = this->_find_base(shelter.children);
            _static_impl::_thaw(conflicting_index);
            node = _move_nodes(shelter, new_base, node);
        } else {
            _shelter shelter;
            _evacuate(node, shelter);
            shelter.children.push_back(c);
            auto new_base = this->_find_base(shelter.children);
            shelter.children.pop_back();
            _move_nodes(shelter, new_base);
        }
        return node;
    }
    
    _index_type _insert_trans(_index_type node, _char_type c) {
        if (_static_impl::_is_leaf(node)) {
            return _grow(node, _static_impl::_find_base({c}), c);
        }
        auto base = _static_impl::_base(node);
        if (not _static_impl::_empty(base xor c)) {
            node = _solve_collision(node, c);
            base = _static_impl::_base(node);
        }
        auto next = base xor c;
        assert(_static_impl::_empty(next));
        _static_impl::_setup(next);
        _static_impl::_set_check(next, node);
        // Insert c to siblings link
        auto first_child = _static_impl::_child(node);
        assert(c != first_child);
        if (c < first_child) {
            _static_impl::_set_child(node, c);
            _static_impl::_set_sibling(next, first_child);
            for (auto child = first_child; ; ) {
                auto sibling = _static_impl::_sibling(base xor child);
                if (sibling == first_child) {
                    _static_impl::_set_sibling(base xor child, c);
                    break;
                }
                child = sibling;
            }
        } else {
            for (auto child = first_child; ; ) {
                auto sibling = _static_impl::_sibling(base xor child);
                if (sibling > c or sibling == first_child) {
                    _static_impl::_set_sibling(base xor child, c);
                    _static_impl::_set_sibling(next, sibling);
                    break;
                }
                child = sibling;
            }
        }
        
        _static_impl::_consume_block(base/kBlockSize, 1);
        
        return next;
    }
    
};


template <typename IndexType>
class _BitOperationalDynamicDoubleArrayBCImpl : protected _DynamicDoubleArrayBCImpl<IndexType> {
public:
    using _base_impl = _DynamicDoubleArrayBCImpl<IndexType>;
    using _index_type = typename _base_impl::_index_type;
    using _char_type = typename _base_impl::_char_type;
    
    using _base_impl::kBlockSize;
    using _base_impl::kInitialEmptyBlockHead;
    
protected:
    _BitOperationalDynamicDoubleArrayBCImpl() : _base_impl() {}
    
    virtual ~_BitOperationalDynamicDoubleArrayBCImpl() = default;
    
    _index_type _find_base(const std::vector<_char_type>& children) override {
        if (children.size() == 1 and _base_impl::personal_block_head_ != kInitialEmptyBlockHead) {
            _index_type pbh = _base_impl::personal_block_head_;
            return (kBlockSize * pbh + _base_impl::_block_at(pbh).empty_head()) xor children.front();
        }
        
        if (_base_impl::general_block_head_ != kInitialEmptyBlockHead) {
            size_t b = _base_impl::general_block_head_;
            while (true) {
                auto block = _base_impl::_block_at(b);
                if (block.num_empties() >= children.size()) {
                    assert(block.error_count() < _base_impl::kErrorThreshold);
                    auto ctz = da_util::xcheck_in_da_block(block.field_ptr(), children);
                    if (ctz < kBlockSize) {
                        return kBlockSize * b + ctz;
                    }
                }
                auto new_b = block.next();
                if (new_b == _base_impl::general_block_head_) {
                    break;
                }
                assert(b != block.next());
                assert(_base_impl::_block_at(new_b).error_count() < _base_impl::kErrorThreshold);
                _base_impl::_error_block(b);
                b = new_b;
            }
        }
        return _base_impl::_new_base(children.front());
    }
    
};


template <typename IndexType, bool LegacyBuild = false>
class DoubleArray :
    private std::conditional_t<LegacyBuild,
                               _DynamicDoubleArrayBCImpl<IndexType>,
                               _BitOperationalDynamicDoubleArrayBCImpl<IndexType>> {
    using _impl = std::conditional_t<LegacyBuild,
                                     _DynamicDoubleArrayBCImpl<IndexType>,
                                     _BitOperationalDynamicDoubleArrayBCImpl<IndexType>>;
    using _self = DoubleArray<IndexType, LegacyBuild>;
public:
    using input_trie = typename graph_util::Trie<char>;
    using index_type = typename _impl::_index_type;
    using char_type = typename _impl::_char_type;
    
    static constexpr index_type kRootIndex = _impl::kRootIndex;
    static constexpr char_type kLeafChar = graph_util::kLeafChar;
    
    static constexpr size_t kBlockSize = _impl::kBlockSize;
    
    DoubleArray() : _impl() {}
    
    template <typename StrIter,
              typename Traits = std::iterator_traits<StrIter>>
    DoubleArray(StrIter begin, StrIter end) {
        _arrange_keysets(begin, end, 0, kRootIndex);
    }
    
    DoubleArray(const std::vector<std::string>& key_set) : DoubleArray(key_set.begin(), key_set.end()) {}
    
    DoubleArray(const input_trie& trie) {
        _arrange_trie(trie, graph_util::kRootIndex, kRootIndex);
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
        return _impl::_size_in_bytes();
    }
    
    void insert(std::string_view key) {
        index_type node = kRootIndex;
        size_t pos = 0;
        for (; pos < key.size(); pos++) {
            if (_impl::_is_leaf(node)) {
                break;
            }
            if (not _transition(node, key[pos])) {
                _impl::_insert_in_bc(node, key.substr(pos));
                return;
            }
        }
        if (_impl::_is_leaf(node)) {
            auto tail_index = _impl::_tail_index(node);
            for (; pos < key.size(); pos++, tail_index++) {
                if (tail_index >= _impl::tail_.size() or
                    _impl::tail_[tail_index] != char_type(key[pos])) {
                    char_type e,cc;
                    if (tail_index < _impl::tail_.size()) {
                        e = _impl::tail_[tail_index];
                        cc = key[pos];
                    }
                    _impl::_insert_in_tail(node, tail_index, key.substr(pos));
                    return;
                }
            }
            if (_impl::tail_[tail_index] != kLeafChar) {
                _impl::_insert_in_tail(node, tail_index, "");
            }
        } else {
            if (not _transition(node, kLeafChar)) {
                _impl::_insert_in_bc(node, "");
            }
        }
    }
    
    bool erase(std::string_view key) {
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
                if (_impl::tail_[tail_index] != key[pos])
                    return false;
            }
            if (_impl::tail_[tail_index] == kLeafChar)
                _impl::_delete_leaf(node);
        } else {
            if (_transition(node, kLeafChar))
                _impl::_delete_leaf(node);
        }
        return true;
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
                if (_impl::tail_[tail_index] != key[pos])
                    return false;
            }
            return _impl::tail_[tail_index] == kLeafChar;
        } else {
            return _transition(node, kLeafChar);
        }
    }
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex] \texists, \tcheck, \tsibling, \tbase, \tchild"  << std::endl;
        for (size_t i = 0; i < _impl::storage_.size(); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            auto empty =_impl::_empty(i);
            if (empty) {
                std::cout << "\t\t"<<i<<"] \t"<<0<< std::endl;
            } else {
                std::cout << "\t\t"<<i<<"] \t"<<1<<", \t"<<_impl::_check(i)<<", \t"<<_impl::_sibling(i)<<", \t"<<_impl::_base(i)<<", \t"<<_impl::_child(i)<< std::endl;
            }
        }
    }
    
private:
    bool _transition(index_type& node, char_type c) const {
        if (_impl::_is_leaf(node))
            return false;
        assert(not _impl::_is_leaf(node));
        auto next = _impl::_base(node) xor c;
        if (_impl::_empty(next) or
            _impl::_check(next) != node)
            return false;
        node = next;
        return true;
    }
    
    void _arrange_da(const _self& da, index_type node, index_type co_node) {
        if (da._impl::_is_leaf(node)) {
            auto tail_index = _impl::_insert_suffix(da._impl::_suffix_in_tail(da._impl::_tail_index(node)));
            _impl::_set_tail_index(co_node, tail_index);
            return;
        }
        std::vector<char_type> children;
        da._impl::_for_each_children(node, [&](auto child, auto) {
            children.push_back(child);
        });
        
        auto new_base = _impl::_find_base(children);
        _impl::_insert_nodes(co_node, children, new_base);
        for (auto c : children) {
            auto target = node;
            da._transition(target, c);
            _arrange_da(da, target, new_base xor c);
        }
    }
    
    void _arrange_trie(const input_trie& trie, typename input_trie::size_type node, index_type co_node) {
        auto& n = trie.node(node);
        if (n.terminal()) {
            return;
        }
        std::vector<char_type> children;
        n.for_each_edge([&](auto c, auto) {
            children.push_back(c);
        });
        
        auto new_base = _impl::_find_base(children);
        _impl::_insert_nodes(co_node, children, new_base);
        for (size_t i = 0; i < children.size(); i++) {
            auto c = children[i];
            auto next = new_base xor c;
            _arrange_trie(trie, n.target(c), next);
        }
    }
    
    template <typename StrIter,
              typename Traits = std::iterator_traits<StrIter>>
    void _arrange_keysets(StrIter begin, StrIter end, size_t depth, index_type co_node) {
        if ((*begin).size() < depth)
            return;
        if (end-begin == 1) {
            auto tail_index = _impl::_insert_suffix((*begin).size() > depth ? std::string_view((const char*)&(*begin) + depth) : "");
            _impl::_set_tail_index(co_node, tail_index);
            return;
        }
        
        std::vector<char_type> children;
        std::vector<StrIter> iters = {begin};
        char_type prev_c = (*begin).size() <= depth ? kLeafChar : (*begin)[depth];
        char_type b = depth==0?0:(*begin)[depth-1];
        for (auto it = begin+1; it != end; ++it) {
            char_type bb = (*it)[depth-1];
            assert(depth == 0 or bb == b);
            char_type c = (*it).size() <= depth ? kLeafChar : (*it)[depth];
            if (c != prev_c) {
                children.push_back(prev_c);
                iters.push_back(it);
                prev_c = c;
            }
        }
        children.push_back((*(end-1))[depth]);
        iters.push_back(end);
        
        auto new_base = _impl::_find_base(children);
        _impl::_insert_nodes(co_node, children, new_base);
        for (size_t i = 0; i < children.size(); i++) {
            auto c = children[i];
            auto next = new_base xor c;
            _arrange_keysets(iters[i], iters[i+1], depth+1, next);
        }
    }
    
};

} // namespace sim_ds

#endif /* DoubleArray_hpp */
