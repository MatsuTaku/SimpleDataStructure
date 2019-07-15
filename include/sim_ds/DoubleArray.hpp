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


// MARK: - Unit reference

//  Double-array unit (Base/Check/LabelId) Implementation
//      Check: Pointer
//          if disabled:
//              1 0 *(next)
//          else:
//              0 ?(terminal flag) *(check)
//      Sibling: Byte
//          *(child char)
//      Base: Pointer
//          if disabled:
//              1 0 *(prev)
//          else if basic base:
//              0 0 *(base)
//          else if label id:
//              0 1 *(label id)
//      Child: Byte
//          *(child char)
//
template <class _Da>
class _DoubleArrayUnitBcpReferenceCommon {
public:
    using _index_type = typename _Da::_index_type;
    using _char_type = typename _Da::_char_type;
    
    static constexpr size_t kIndexBytes = sizeof(_index_type);
    static constexpr _index_type kUpperBit = 1ull << (kIndexBytes*8-1);
    static constexpr _index_type kSecondBit = kUpperBit >> 1;
    static constexpr _index_type kEmptyFlag = kUpperBit;
    static constexpr _index_type kLabelFlag = kSecondBit;
    static constexpr _index_type kTerminalFlag = kSecondBit;
    static constexpr _index_type kIndexMask = kSecondBit - 1;
    static constexpr _index_type kIndexMax = kIndexMask;
    static constexpr _char_type kEmptyChar = _Da::kEmptyChar;
    
    static constexpr size_t kCheckInsets = 0;
    static constexpr size_t kSiblingInsets = kIndexBytes;
    static constexpr size_t kBaseInsets = kIndexBytes + 1;
    static constexpr size_t kChildInsets = kIndexBytes * 2 + 1;
    static constexpr size_t kEdgeFlagInsets = kBaseInsets + kIndexBytes - 1;
    static constexpr size_t kLabelFlagInsets = kBaseInsets + kIndexBytes - 1;
    static constexpr size_t kTerminalFlagInsets = kCheckInsets + kIndexBytes - 1;
    static constexpr size_t kNextInsets = kBaseInsets;
    static constexpr size_t kPrevInsets = kCheckInsets;
    static constexpr size_t kUnitBytes = kIndexBytes * 2 + 2;
};


template <class _Da>
class _DoubleArrayUnitBcpConstReference;

template <class _Da>
class _DoubleArrayUnitBcpReference : private _DoubleArrayUnitBcpReferenceCommon<_Da> {
    using _common = _DoubleArrayUnitBcpReferenceCommon<_Da>;
    
    using _unit_storage_type = typename _Da::_unit_storage_type;
    static_assert(sizeof(_unit_storage_type) == 1, "Invalid unit storage type!");
    using _unit_storage_pointer = typename _Da::_unit_storage_pointer;
    
    using typename _common::_index_type;
    using typename _common::_char_type;
    
    using _common::kIndexBytes;
    using _common::kEmptyFlag;
    using _common::kLabelFlag;
    using _common::kTerminalFlag;
    using _common::kIndexMask;
    using _common::kIndexMax;
    using _common::kEmptyChar;
    
    using _common::kCheckInsets;
    using _common::kSiblingInsets;
    using _common::kBaseInsets;
    using _common::kChildInsets;
    using _common::kEdgeFlagInsets;
    using _common::kLabelFlagInsets;
    using _common::kTerminalFlagInsets;
    using _common::kNextInsets;
    using _common::kPrevInsets;
    using _common::kUnitBytes;
    
private:
    _unit_storage_pointer pointer_;
    
    friend typename _Da::_self;
    friend class _DoubleArrayUnitBcpConstReference<_Da>;
    
public:
    bool terminal() const {return *(pointer_ + kTerminalFlagInsets) bitand 0x40;}
    
    void set_terminal() {
        *(pointer_ + kTerminalFlagInsets) |= 0x40;
    }
    
    void disable_terminal() {
        *(pointer_ + kTerminalFlagInsets) &= compl 0x40;
    }
    
    _index_type check() const {return *reinterpret_cast<_index_type*>(pointer_ + kCheckInsets) bitand kIndexMask;}
    
    void set_check(_index_type new_check) {
        _index_type& target = *reinterpret_cast<_index_type*>(pointer_ + kCheckInsets);
        target = new_check bitor (target bitand kTerminalFlag);
    }
    
    _index_type base() const {return *reinterpret_cast<_index_type*>(pointer_ + kBaseInsets) bitand kIndexMask;}
    
    void set_base(_index_type new_base) {
        *reinterpret_cast<_index_type*>(pointer_ + kBaseInsets) = new_base bitand kIndexMask;
    }
    
    bool has_label() const {return *(pointer_ + kLabelFlagInsets) bitand 0x40;}
    
    _index_type pool_index() const {assert(has_label()); return base();}
    
    void set_pool_index(_index_type new_pool_index) {
        *reinterpret_cast<_index_type*>(pointer_ + kBaseInsets) = (new_pool_index bitand kIndexMask) bitor kLabelFlag;
    }
    
    _char_type child() const {return *reinterpret_cast<_char_type*>(pointer_ + kChildInsets);}
    
    void set_child(_char_type new_child) {
        *reinterpret_cast<_char_type*>(pointer_ + kChildInsets) = new_child;
    }
    
    _char_type sibling() const {return *reinterpret_cast<_char_type*>(pointer_ + kSiblingInsets);}
    
    void set_sibling(_char_type new_sibling) {
        *reinterpret_cast<_char_type*>(pointer_ + kSiblingInsets) = new_sibling;
    }
    
    bool base_empty() const {return *(pointer_ + kEdgeFlagInsets) bitand 0x80;}
    
    _index_type prev() const {
        assert(*reinterpret_cast<_index_type*>(pointer_ + kPrevInsets) bitand kEmptyFlag);
        return *reinterpret_cast<_index_type*>(pointer_ + kPrevInsets) bitand kIndexMask;
    }
    
    void set_prev(_index_type new_prev) {
        *reinterpret_cast<_index_type*>(pointer_ + kPrevInsets) = (new_prev bitand kIndexMask) bitor kEmptyFlag;
    }
    
    _index_type next() const {
        assert(*reinterpret_cast<_index_type*>(pointer_ + kNextInsets) bitand kEmptyFlag);
        return *reinterpret_cast<_index_type*>(pointer_ + kNextInsets) bitand kIndexMask;
    }
    
    void set_next(_index_type new_next) {
        *reinterpret_cast<_index_type*>(pointer_ + kNextInsets) = (new_next bitand kIndexMask) bitor kEmptyFlag;
    }
    
    void init(_index_type prev, _index_type next) {
        set_prev(prev);
        set_sibling(kEmptyChar);
        set_next(next);
        set_child(kEmptyChar);
    }
    
    void clean() {
        *reinterpret_cast<_index_type*>(pointer_ + kCheckInsets) = kEmptyFlag;
        set_sibling(kEmptyChar);
        *reinterpret_cast<_index_type*>(pointer_ + kBaseInsets) = kEmptyFlag;
        set_child(kEmptyChar);
    }
    
private:
    _DoubleArrayUnitBcpReference(_unit_storage_pointer pointer) : pointer_(pointer) {}
    
};


template <class _Da>
class _DoubleArrayUnitBcpConstReference : private _DoubleArrayUnitBcpReferenceCommon<_Da> {
    using _common = _DoubleArrayUnitBcpReferenceCommon<_Da>;
    
    using _unit_storage_type = typename _Da::_unit_storage_type;
    static_assert(sizeof(_unit_storage_type) == 1, "Invalid unit storage type!");
    using _unit_storage_pointer = typename _Da::_const_unit_storage_pointer;
    
    using typename _common::_index_type;
    using typename _common::_char_type;
    
    using _common::kIndexBytes;
    using _common::kEmptyFlag;
    using _common::kLabelFlag;
    using _common::kTerminalFlag;
    using _common::kIndexMask;
    using _common::kIndexMax;
    using _common::kEmptyChar;
    
    using _common::kCheckInsets;
    using _common::kSiblingInsets;
    using _common::kBaseInsets;
    using _common::kChildInsets;
    using _common::kEdgeFlagInsets;
    using _common::kLabelFlagInsets;
    using _common::kTerminalFlagInsets;
    using _common::kNextInsets;
    using _common::kPrevInsets;
    using _common::kUnitBytes;
    
private:
    _unit_storage_pointer pointer_;
    
    friend typename _Da::_self;
    
public:
    _DoubleArrayUnitBcpConstReference(const _DoubleArrayUnitBcpReference<_Da>& x) : pointer_(x.pointer_) {}
    
    bool terminal() const {return *(pointer_ + kTerminalFlagInsets) bitand 0x40;}
    
    _index_type check() const {return *reinterpret_cast<const _index_type*>(pointer_ + kCheckInsets) bitand kIndexMask;}
    
    _index_type base() const {return *reinterpret_cast<const _index_type*>(pointer_ + kBaseInsets) bitand kIndexMask;}
    
    bool has_label() const {return *(pointer_ + kLabelFlagInsets) bitand 0x40;}
    
    _index_type pool_index() const {assert(has_label()); return base();}
    
    _char_type child() const {return *reinterpret_cast<const _char_type*>(pointer_ + kChildInsets);}
    
    _char_type sibling() const {return *reinterpret_cast<const _char_type*>(pointer_ + kSiblingInsets);}
    
    bool base_empty() const {return *(pointer_ + kEdgeFlagInsets) bitand 0x80;}
    
    _index_type prev() const {
        assert(*reinterpret_cast<_index_type*>(pointer_ + kPrevInsets) bitand kEmptyFlag);
        return *reinterpret_cast<_index_type*>(pointer_ + kPrevInsets) bitand kIndexMask;
    }
    
    _index_type next() const {
        assert(*reinterpret_cast<_index_type*>(pointer_ + kNextInsets) bitand kEmptyFlag);
        return *reinterpret_cast<_index_type*>(pointer_ + kNextInsets) bitand kIndexMask;
    }
    
private:
    _DoubleArrayUnitBcpConstReference(_unit_storage_pointer pointer) : pointer_(pointer) {}
    
};


// MARK: - Block reference

template <class _Da>
class _DoubleArrayBlockReferenceCommon {
public:
    using _index_type = typename _Da::_index_type;
    using _block_word_type = typename _Da::_block_word_type;
    
    static constexpr size_t kWordBits = sizeof(_block_word_type)*8;
    static constexpr size_t kBlockSize = _Da::kBlockSize;
    
    static constexpr size_t kPrevOffset = 0;
    static constexpr size_t kNextOffset = kPrevOffset + 1;
    static constexpr size_t kNumEmptiesOffset = kNextOffset + 1;
    static constexpr size_t kErrorCountOffset = kNumEmptiesOffset;
    static constexpr size_t kErrorCountInset = 32;
    static constexpr size_t kFieldSize = kBlockSize / kWordBits; // 4
    
    static constexpr _block_word_type kNumEmptiesMask = 0x1FF;
};


template <class _Dac>
class _DoubleArrayBlockConstReference;

template <class _Dac>
class _DoubleArrayBlockReference : _DoubleArrayBlockReferenceCommon<_Dac> {
    using _common = _DoubleArrayBlockReferenceCommon<_Dac>;
protected:
    using typename _common::_index_type;
    using typename _common::_block_word_type;
    static_assert(sizeof(_block_word_type) == 8, "Invalid block word type!");
    using _block_pointer = typename _Dac::_block_pointer;
    using _const_block_pointer = typename _Dac::_block_pointer;
    
    using _common::kWordBits;
    using _common::kBlockSize;
    
    using _common::kPrevOffset;
    using _common::kNextOffset;
    using _common::kNumEmptiesOffset;
    using _common::kNumEmptiesMask;
    using _common::kErrorCountOffset;
    using _common::kErrorCountInset;
    using _common::kFieldSize;
    
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
class _DoubleArrayBlockConstReference : _DoubleArrayBlockReferenceCommon<_Dac> {
    using _common = _DoubleArrayBlockReferenceCommon<_Dac>;
protected:
    using typename _common::_index_type;
    using typename _common::_block_word_type;
    static_assert(sizeof(_block_word_type) == 8, "Invalid block word type!");
    using _block_pointer = typename _Dac::_const_block_pointer;
    
    using _common::kWordBits;
    using _common::kBlockSize;
    
    using _common::kPrevOffset;
    using _common::kNextOffset;
    using _common::kNumEmptiesOffset;
    using _common::kNumEmptiesMask;
    using _common::kErrorCountOffset;
    using _common::kErrorCountInset;
    using _common::kFieldSize;
    
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


// MARK: - MPTrie

template <typename IndexType, bool BitOperationalFind>
class _DoubleArrayBcMpTrieImpl {
public:
    using _self = _DoubleArrayBcMpTrieImpl<IndexType, BitOperationalFind>;
    using _index_type = IndexType;
    using _char_type = uint8_t;
    using _inset_type = uint8_t;
    
    using _unit_storage_type = uint8_t;
    using _unit_storage_pointer = _unit_storage_type*;
    using _const_unit_storage_pointer = const _unit_storage_type*;
    using _unit_reference = _DoubleArrayUnitBcpReference<_self>;
    using _unit_const_reference = _DoubleArrayUnitBcpConstReference<_self>;
    
    using _block_word_type = uint64_t;
    using _block_pointer = _block_word_type*;
    using _const_block_pointer = const _block_word_type*;
    using _block_reference = _DoubleArrayBlockLegacyReference<_self>;
    using _block_const_reference = _DoubleArrayBlockLegacyConstReference<_self>;
    
    static constexpr _index_type kRootIndex = 0;
    static constexpr _char_type kLeafChar = graph_util::kLeafChar;
    static constexpr _char_type kEmptyChar = 0xFF;
    
    static constexpr size_t kUnitBytes = _unit_reference::kUnitBytes;
    static constexpr _index_type kIndexMax = _unit_reference::kIndexMax;
    static constexpr _index_type kIndexMask = _unit_reference::kIndexMask;
    static constexpr size_t kIndexBytes = _unit_reference::kIndexBytes;
    static constexpr _index_type kEmptyFlag = _unit_reference::kEmptyFlag;
    
    static constexpr size_t kBlockQBytes = 8;
    static constexpr unsigned kBlockSize = 0x100;
    static constexpr _index_type kInitialEmptyBlockHead = std::numeric_limits<_index_type>::max();
    
    using _input_trie = graph_util::Trie<char>;
    
    static constexpr size_t kErrorThreshold = 32;
    
protected:
    _index_type general_block_head_;
    _index_type personal_block_head_;
    aligned_vector<_block_word_type, 32> basic_block_;
    std::vector<_unit_storage_type> container_;
    std::vector<_char_type> tail_;
    
    _DoubleArrayBcMpTrieImpl() : general_block_head_(kInitialEmptyBlockHead), personal_block_head_(kInitialEmptyBlockHead) {
        _expand();
        _setup(kRootIndex);
        auto root_unit = _unit_at(kRootIndex);
        root_unit.set_check(0);
        root_unit.set_sibling(0);
        _consume_block(_block_index_of(kRootIndex), 1);
    }
    
    virtual ~_DoubleArrayBcMpTrieImpl() = default;
    
    size_t _size_in_bytes() const {
        return sizeof(general_block_head_) + size_vec(basic_block_) + size_vec(container_) + size_vec(tail_);
    }
    
    size_t _num_elements() const {return container_.size() / kUnitBytes;}
    
    size_t _num_blocks() const {return basic_block_.size() / kBlockQBytes;}
    
    _unit_reference _unit_at(_index_type index) {
        return _unit_reference(container_.data() + (kUnitBytes * index));
    }
    
    _unit_const_reference _unit_at(_index_type index) const {
        return _unit_const_reference(container_.data() + (kUnitBytes * index));
    }
    
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
        if (_num_elements() > kIndexMax) {
            throw "Index out-of-range! You should set large byte-size of template parameter.";
        }
        container_.resize(container_.size() + kUnitBytes * kBlockSize);
        
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
        auto front = _num_elements() - kBlockSize;
        auto inset_back = kBlockSize - 1;
        _unit_at(front).init(inset_back, 1);
        for (size_t i = 1; i < inset_back; i++) {
            _unit_at(front+i).init(i-1, i+1);
        }
        _unit_at(front+inset_back).init(inset_back-1, 0);
    }
    
    void _shrink() {
        if (_num_blocks() == 1)
            return;
        assert(_block_at(_num_elements()-1).num_empties() == kBlockSize);
        container_.resize(container_.size() - kUnitBytes * kBlockSize);
        basic_block_.resize(basic_block_.size() - kBlockQBytes);
    }
    
    bool _empty(size_t index) const {
        return _block_at(_block_index_of(index)).empty_element_at(index%kBlockSize);
    }
    
    void _freeze(size_t index) {
        assert(_empty(index));
        // Remove empty-elements linking
        auto next = _unit_at(index).next();
        auto prev = _unit_at(index).prev();
        auto block_index = _block_index_of(index);
        auto offset = kBlockSize * (block_index);
        _unit_at(offset+next).set_prev(prev);
        _unit_at(offset+prev).set_next(next);
        auto b = _block_at(block_index);
        auto inset = index % kBlockSize;
        if (next == inset) {
            b.disable_link();
        } else if (inset == b.empty_head()) {
            b.set_empty_head(next);
        }
        
        b.freeze_element_at(index%kBlockSize);
    }
    
    void _thaw(size_t index) {
        assert(not _empty(index));
        auto block_index = _block_index_of(index);
        auto b = _block_at(block_index);
        b.thaw_element_at(index%kBlockSize);
        
        // Append empty-elements linking
        auto inset = index % kBlockSize;
        if (not b.link_enabled()) {
            _unit_at(index).init(inset, inset);
            b.set_empty_head(inset);
        } else {
            auto offset = kBlockSize * (block_index);
            auto head = b.empty_head();
            auto head_unit = _unit_at(offset+head);
            auto tail = head_unit.prev();
            head_unit.set_prev(inset);
            _unit_at(offset+tail).set_next(inset);
            _unit_at(index).init(tail, head);
        }
    }
    
    void _setup(size_t index) {
        _freeze(index);
        _unit_at(index).clean();
    }
    
    void _erase(size_t index) {
        _unit_at(index).clean();
        _thaw(index);
    }
    
    virtual _index_type _base_at(_index_type node) const {return _unit_at(node).base();}
    
    virtual void _set_base_at(_index_type node, _index_type new_base) {
        _unit_at(node).set_base(new_base);
    }
    
    virtual bool _is_edge_at(_index_type node) const {
        auto unit = _unit_at(node);
        return unit.base_empty() or unit.has_label();
    }
    
    void _set_new_trans(_index_type node, _index_type new_base, _char_type new_child) {
        _set_base_at(node, new_base);
        _unit_at(node).set_child(new_child);
    }
    
    void _set_new_node(_index_type index, _index_type new_check, _char_type new_sibling, bool terminal, _index_type new_pool_index = kEmptyFlag) {
        _setup(index);
        auto unit = _unit_at(index);
        unit.set_check(new_check);
        unit.set_sibling(new_sibling);
        if (terminal)
            unit.set_terminal();
        if (new_pool_index != kEmptyFlag)
            unit.set_pool_index(new_pool_index);
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
        assert(not _is_edge_at(node));
        auto base = _base_at(node);
        auto first_child = _unit_at(node).child();
        for (auto child = first_child; ; ) {
            auto next = base xor child;
            assert(_unit_at(next).check() == node);
            auto sibling = _unit_at(next).sibling();
            action(child, sibling);
            if (sibling == first_child)
                break;
            child = sibling;
        }
    }
    
    template <class Action>
    void _for_each_children(_index_type node, Action action) const {
        assert(not _is_edge_at(node));
        auto base = _base_at(node);
        auto first_child = _unit_at(node).child();
        for (auto child = first_child; ; ) {
            auto next = base xor child;
            assert(not _empty(next));
            assert(_unit_at(next).check() == node);
            auto sibling = _unit_at(next).sibling();
            action(child, sibling);
            if (sibling == first_child)
                break;
            child = sibling;
        }
    }
    
    size_t _num_of_children(_index_type node) const {
        if (_is_edge_at(node))
            return 0;
        size_t cnt = 0;
        _for_each_children(node, [&cnt](auto, auto) {++cnt;});
        return cnt;
    }
    
    void _insert_nodes(_index_type node, std::vector<_char_type>& children, _index_type base) {
        if (base >= _num_elements())
            _expand();
        _set_new_trans(node, base, children.front());
        for (size_t i = 0; i < children.size(); i++) {
            auto c = children[i];
            auto next = base xor c;
            _setup(next);
            _set_new_node(next, node, children[(i+1)%children.size()], false);
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
    
    _index_type _new_base(_char_type c) const {
        if constexpr (BitOperationalFind) {
            return _num_elements();
        } else {
            return _num_elements() xor c;
        }
    }
    
    _index_type _find_base(const std::vector<_char_type>& children) {
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
            const auto offset = kBlockSize * b;
            
            if constexpr (BitOperationalFind) {
                assert(block.error_count() < kErrorThreshold);
                auto ctz = da_util::xcheck_in_da_block(block.field_ptr(), children);
                if (ctz < kBlockSize) {
                    return offset + ctz;
                }
            } else {
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
                    auto next = _unit_at(index).next();
                    if (next == block.empty_head())
                        break;
                    index = offset + next;
                }
            }
            
            auto new_b = block.next();
            if (new_b == general_block_head_) {
                break;
            }
            _error_block(b);
            b = new_b;
        }
        return _new_base(children.front());
    }
    
};


template <class _Impl>
class _DynamicDoubleArrayPatriciaTrieBehavior;

template <class _Impl>
class _DynamicDoubleArrayMpTrieBehavior : protected _Impl {
public:
    using _impl = _Impl;
    using typename _impl::_index_type;
    using typename _impl::_char_type;
    
    using _impl::kBlockSize;
    using _impl::kRootIndex;
    using _impl::kInitialEmptyBlockHead;
    using _impl::kLeafChar;
    using _impl::kEmptyChar;
    
protected:
    _DynamicDoubleArrayMpTrieBehavior() : _impl() {}
    
    virtual ~_DynamicDoubleArrayMpTrieBehavior() = default;
    
    _index_type _grow(_index_type node, _index_type base, _char_type c) {
        if (base >= _impl::_num_elements())
            _impl::_expand();
        _impl::_set_new_trans(node, base, c);
        auto next = base xor c;
        _impl::_set_new_node(next, node, c, false);
        _impl::_consume_block(_impl::_block_index_of(base), 1);
        return next;
    }
    
    void _insert_in_bc(_index_type node, std::string_view suffix) {
        if (suffix.size() > 0) {
            node = _insert_trans(node, suffix.front());
            auto tail_index = _impl::_insert_suffix(suffix.size() > 1 ? suffix.substr(1) : "");
            _impl::_unit_at(node).set_pool_index(tail_index);
        }
        _impl::_unit_at(node).set_terminal();
    }
    
    virtual void _insert_in_tail(_index_type node, _index_type tail_pos, std::string_view suffix) {
        auto tail_index = _impl::_unit_at(node).pool_index();
        _impl::_unit_at(node).disable_terminal();
        while (tail_index < tail_pos) {
            auto c = _impl::tail_[tail_index++];
            node = _grow(node, this->_find_base({c}), c);
        }
        auto c = _impl::tail_[tail_index];
        auto next = _grow(node, this->_find_base({c}), c);
        auto unit = _impl::_unit_at(next);
        unit.set_pool_index(tail_index+1);
        unit.set_terminal();
        _insert_in_bc(node, suffix);
    }
    
    void _delete_leaf(_index_type node) {
        if (not _impl::_unit_at(node).terminal())
            return;
        if (not _impl::_is_edge_at(node)) {
            _impl::_unit_at(node).disable_terminal();
            return;
        }
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
    friend class _DynamicDoubleArrayPatriciaTrieBehavior<_impl>;
    
    struct _moving_luggage {
        _index_type base;
        uint8_t child;
        bool has_leaf;
        bool terminal;
        _moving_luggage(_index_type base, uint8_t child, bool has_leaf, bool terminal) : base(base), child(child), has_leaf(has_leaf), terminal(terminal) {}
    };
    
    struct _shelter {
        _index_type node;
        std::vector<_char_type> children;
        std::vector<_moving_luggage> luggages;
    };
    
    void _evacuate(_index_type node, _shelter& shelter) {
        shelter.node = node;
        auto base = _impl::_base_at(node);
        _impl::_for_each_children(node, [&](auto child, auto) {
            shelter.children.push_back(child);
            auto index = base xor child;
            auto index_unit = _impl::_unit_at(index);
            shelter.luggages.emplace_back(index_unit.base(), index_unit.child(), index_unit.has_label(), index_unit.terminal());
            _impl::_erase(index);
        });
        _impl::_refill_block(_impl::_block_index_of(base), shelter.children.size());
    }
    
    void _update_node(_index_type index, _index_type check, _index_type sibling, _index_type base, _index_type child, bool has_label, bool terminal) {
        assert(_impl::_empty(index));
        if (not has_label) {
            _impl::_set_new_node(index, check, sibling, terminal);
            _impl::_set_new_trans(index, base, child);
        } else {
            _impl::_set_new_node(index, check, sibling, terminal, base);
            _impl::_unit_at(index).set_child(child);
        }
    }
    
    void _update_check(_index_type base, _char_type child, _index_type new_check) {
        assert(base < _impl::_num_elements());
        for (_char_type c = child; ; ) {
            auto unit = _impl::_unit_at(base xor c);
            unit.set_check(new_check);
            auto sibling = unit.sibling();
            if (sibling == child)
                break;
            c = sibling;
        }
    }
    
    _index_type _move_nodes(_shelter& shelter, _index_type new_base,
                            _index_type monitoring_node = kRootIndex) {
        if (new_base >= _impl::_num_elements())
            _impl::_expand();
        auto old_base = _impl::_base_at(shelter.node);
        _impl::_set_base_at(shelter.node, new_base);
        for (size_t i = 0; i < shelter.children.size(); i++) {
            auto child = shelter.children[i];
            auto sibling = shelter.children[(i+1)%shelter.children.size()];
            auto new_next = new_base xor child;
            auto& luggage = shelter.luggages[i];
            _update_node(new_next, shelter.node, sibling, luggage.base, luggage.child, luggage.has_leaf, luggage.terminal);
            if (not _impl::_is_edge_at(new_next))
                _update_check(_impl::_base_at(new_next), luggage.child, new_next);
            assert(not _impl::_empty(new_next));
            if ((old_base xor child) == monitoring_node) {
                monitoring_node = new_next;
            }
        }
        _impl::_consume_block(_impl::_block_index_of(new_base), shelter.children.size());
        
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
            auto& luggage = shelter.luggages[i];
            _update_node(new_next, shelter.node, sibling, luggage.base, luggage.child, luggage.has_leaf, luggage.terminal);
            if (not _impl::_is_edge_at(new_next))
                _update_check(_base_at(new_next), luggage.child, new_next);
        }
        _consume_block(_block_index_of(target_base), shelter.children.size());
        
        // Compress sheltered siblings recursively.
        for (auto& sht : shelters) {
            _compress_nodes(sht.check, sht.children, sht.luggages, _find_compression_target(sht.children));
        }
        
        return;
    }
    
    _index_type _find_compression_target(std::vector<_char_type>& siblings) const {
        if (siblings.size() == 1 and _impl::personal_block_head_ != kInitialEmptyBlockHead) {
            _index_type pbh = _impl::personal_block_head_;
            return (kBlockSize * pbh + _block_at(pbh).empty_head()) xor siblings.front();
        }
        if (_impl::general_block_head_ == kInitialEmptyBlockHead) {
            return _impl::_num_elements();
        }
        _index_type b = _impl::general_block_head_;
        while (true) {
            if (b == _block_index_of(_impl::_num_elements()-1)) {
                b = _block_at(_block_index_of(_impl::_num_elements()-1)).next();
                if (b == _impl::general_block_head_)
                    return _impl::_num_elements();
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
            if (block.next() == _impl::general_block_head_) {
                return _impl::_num_elements();
            }
            b = block.next();
        }
    }
    
    void _reduce() {
        using bit_util::ctz256;
        for (_index_type b = _block_index_of(_impl::_num_elements()-1); b > 0; b--) {
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
                    auto unit = _unit_at(next);
                    luggages.emplace_back(unit.base(), unit.child(), unit.has_label(), unit.terminal());
                    _erase(next);
                });
                _refill_block(_block_index_of(base), children.size());
                _compress_nodes(parent, children, luggages, compression_target);
            }
            _impl::_shrink();
        }
    }
    
    _index_type _solve_collision(_index_type node, _char_type c) {
        auto base = _impl::_base_at(node);
        auto conflicting_index = base xor c;
        auto competitor = _impl::_unit_at(conflicting_index).check();
        if (conflicting_index != kRootIndex and
            _impl::_num_of_children(competitor) <= _impl::_num_of_children(node)) {
            _shelter shelter;
            _evacuate(competitor, shelter);
            _impl::_freeze(conflicting_index);
            auto new_base = _impl::_find_base(shelter.children);
            _impl::_thaw(conflicting_index);
            node = _move_nodes(shelter, new_base, node);
        } else {
            _shelter shelter;
            _evacuate(node, shelter);
            shelter.children.push_back(c);
            auto new_base = _impl::_find_base(shelter.children);
            shelter.children.pop_back();
            _move_nodes(shelter, new_base);
        }
        return node;
    }
    
    _index_type _insert_trans(_index_type node, _char_type c) {
        if (_impl::_is_edge_at(node)) {
            return _grow(node, _impl::_find_base({c}), c);
        }
        auto base = _impl::_base_at(node);
        auto next = base xor c;
        if (not _impl::_empty(next)) {
            node = _solve_collision(node, c);
            base = _impl::_base_at(node);
            next = base xor c;
        }
        assert(_impl::_empty(next));
        _impl::_setup(next);
        auto next_unit = _impl::_unit_at(next);
        next_unit.set_check(node);
        // Insert c to siblings link
        auto node_unit = _impl::_unit_at(node);
        auto first_child = node_unit.child();
        assert(c != first_child);
        if (c < first_child) {
            node_unit.set_child(c);
            next_unit.set_sibling(first_child);
            for (auto child = first_child; ; ) {
                auto child_unit = _impl::_unit_at(base xor child);
                auto sibling = child_unit.sibling();
                if (sibling == first_child) {
                    child_unit.set_sibling(c);
                    break;
                }
                child = sibling;
            }
        } else {
            for (auto child = first_child; ; ) {
                auto child_unit = _impl::_unit_at(base xor child);
                auto sibling = child_unit.sibling();
                if (sibling > c or sibling == first_child) {
                    child_unit.set_sibling(c);
                    next_unit.set_sibling(sibling);
                    break;
                }
                child = sibling;
            }
        }
        
        _impl::_consume_block(_impl::_block_index_of(base), 1);
        
        return next;
    }
    
};


// MARK: - Patricia Trie

template <typename IndexType, bool BitOperationalFind>
class _DoubleArrayBcPatriciaTrieImpl : protected _DoubleArrayBcMpTrieImpl<IndexType, BitOperationalFind> {
    using _impl = _DoubleArrayBcMpTrieImpl<IndexType, BitOperationalFind>;
public:
    using typename _impl::_index_type;
    using typename _impl::_char_type;
    
    using _impl::kLeafChar;
    using _impl::kEmptyChar;
    using _impl::kIndexBytes;
    using _impl::kEmptyFlag;
    using _impl::kIndexMask;
    
protected:
    _DoubleArrayBcPatriciaTrieImpl() : _impl() {}
    
    virtual ~_DoubleArrayBcPatriciaTrieImpl() = default;
    
    _index_type _base_in_pool(_index_type pool_index) const {
        uint8_t* pool_ptr = (uint8_t*)_impl::tail_.data() + pool_index;
        _index_type base_in_pool = 0;
        for (size_t i = 0; i < kIndexBytes; i++)
            base_in_pool |= (*(pool_ptr++)) << (8 * i);
        return base_in_pool bitand kIndexMask;
    }
    
    bool _base_in_pool_empty(_index_type pool_index) const {return _impl::tail_[pool_index+kIndexBytes-1] bitand 0x80;}
    
    bool _is_edge_at(_index_type node) const override {
        auto unit = _impl::_unit_at(node);
        return unit.base_empty() or (unit.has_label() and _base_in_pool_empty(unit.pool_index()));
    }
    
    void _init_base_in_pool(_index_type pool_index) {
        uint8_t* pool_ptr = _impl::tail_.data() + pool_index;
        *reinterpret_cast<_index_type*>(pool_ptr) = kEmptyFlag;
    }
    
    void _set_base_in_pool(_index_type pool_index, _index_type new_base) {
        uint8_t* pool_ptr = _impl::tail_.data() + pool_index;
        *reinterpret_cast<_index_type*>(pool_ptr) = new_base bitand kIndexMask;
    }
    
    _index_type _base_at(_index_type node) const override {
        auto unit = _impl::_unit_at(node);
        return not unit.has_label() ? unit.base() : _base_in_pool(unit.pool_index());
    }
    
    void _set_base_at(_index_type node, _index_type base) override {
        auto unit = _impl::_unit_at(node);
        if (not unit.has_label()) {
            unit.set_base(base);
        } else {
            _set_base_in_pool(unit.pool_index(), base);
        }
    }
    
    std::string_view _label_in_pool(_index_type node) const {
        return std::string_view((const char*)_impl::tail_.data() + _unit_at(node).pool_index() + kIndexBytes);
    }
    
    _index_type _insert_label_in_pool(std::string_view key) {
        _index_type index = _impl::tail_.size();
        _impl::tail_.resize(_impl::tail_.size() + kIndexBytes + key.size() + 1);
        _init_base_in_pool(index);
        for (size_t i = 0; i < key.size(); i++)
            _impl::tail_[index+kIndexBytes+i] = key[i];
        _impl::tail_[index+kIndexBytes+key.size()] = kLeafChar;
        return index;
    }
    
};


template <class _Impl>
class _DynamicDoubleArrayPatriciaTrieBehavior : protected _DynamicDoubleArrayMpTrieBehavior<_Impl> {
public:
    using _impl = _Impl;
    using _base = _DynamicDoubleArrayMpTrieBehavior<_Impl>;
    using typename _impl::_index_type;
    using typename _impl::_char_type;
    
    using _impl::kIndexBytes;
    using _impl::kLeafChar;
    using _impl::kEmptyFlag;
    
protected:
    _DynamicDoubleArrayPatriciaTrieBehavior() : _base() {}
    
    virtual ~_DynamicDoubleArrayPatriciaTrieBehavior() = default;
    
    _index_type _grow_with_label(_index_type node, _index_type base, std::string_view label) {
        assert(not label.empty());
        if (label.size() == 1)
            return _base::_grow(node, base, label.front());
        
        if (base >= _impl::_num_elements())
            _impl::_expand();
        _char_type c = label.front();
        _impl::_set_new_trans(node, base, c);
        auto next = base xor c;
        _impl::_set_new_node(next, node, c, false, _impl::_insert_label_in_pool(label.substr(1)));
        _impl::_consume_block(_impl::_block_index_of(base), 1);
        return next;
    }
    
    void _insert_in_bc(_index_type node, std::string_view appendant_suffix) {
        if (not appendant_suffix.empty()) {
            node = _base::_insert_trans(node, appendant_suffix.front());
            if (appendant_suffix.size() > 1) {
                auto pool_index = _impl::_insert_label_in_pool(appendant_suffix.substr(1));
                _impl::_unit_at(node).set_pool_index(pool_index);
            }
        }
        _impl::_unit_at(node).set_terminal();
    }
    
    void _insert_in_pool(_index_type node, _index_type label_pos, std::string_view appendant_suffix) {
        _char_type char_at_confliction = _impl::tail_[label_pos];
        assert(char_at_confliction != kLeafChar);
        assert(char_at_confliction != (_char_type)appendant_suffix.front());
        auto node_unit = _impl::_unit_at(node);
        const auto pool_index = node_unit.pool_index();
        const bool node_is_terminal = node_unit.terminal();
        node_unit.disable_terminal();
        
        const auto node_is_edge = _impl::_is_edge_at(node);
        const auto node_base = _impl::_base_in_pool(pool_index);
        const auto node_child = node_unit.child();
        auto link = [&](_index_type inserted_node) {
            if (node_is_edge) { // There are no children.
                return;
            }
            _impl::_set_new_trans(inserted_node, node_base, node_child);
            _base::_update_check(node_base, node_child, inserted_node);
        };
        
        auto label_index = pool_index + kIndexBytes;
        auto label_prefix_length = label_pos - label_index;
        auto relay_base = _impl::_find_base({char_at_confliction, (_char_type)appendant_suffix.front()});
        if (label_prefix_length == 0) {
            //            ||*|*|*||*|*|*|$||
            //             |↑| conflict at front
            node_unit.set_base(relay_base);
            node_unit.set_child(char_at_confliction);
            auto relay_next = relay_base xor char_at_confliction;
            if (_impl::tail_[label_index + 1] == kLeafChar) { // Length of suffix is 1.
                _impl::_set_new_node(relay_next, node, char_at_confliction, node_is_terminal);
            } else {
                _impl::_set_new_node(relay_next, node, char_at_confliction, node_is_terminal, pool_index + 1);
                _impl::_init_base_in_pool(pool_index+1);
            }
            _impl::_consume_block(_impl::_block_index_of(relay_next), 1);
            link(relay_next);
        } else {
            auto label_suffix_length = 0;
            while (_impl::tail_[label_pos + label_suffix_length] != kLeafChar)
                label_suffix_length++;
            assert(label_suffix_length > 0);
            if (label_prefix_length < label_suffix_length) {
                //              ||*|*|*||*|*|*|$||
                //                 |→ ←| conflict at
                std::string label_prefix((char*)_impl::tail_.data() + label_index, label_prefix_length);
                node_unit.set_pool_index(_impl::_insert_label_in_pool(label_prefix));
                auto relay_next = _base::_grow(node, relay_base, char_at_confliction);
                auto relay_label_index = label_pos+1 - kIndexBytes;
                assert(_impl::tail_[label_pos+1] != kLeafChar);
                auto relay_next_unit = _impl::_unit_at(relay_next);
                relay_next_unit.set_pool_index(relay_label_index);
                _impl::_init_base_in_pool(relay_label_index);
                link(relay_next);
            } else {
                //              ||*|*|*||*|*|*|$||
                //                      |→   ←| conflict at
                std::string label_suffix((char*)_impl::tail_.data() + label_pos, label_suffix_length);
                auto relay_next = _grow_with_label(node, relay_base, label_suffix);
                link(relay_next);
                _impl::tail_[label_pos] = kLeafChar;
            }
        }
        assert(_impl::_base_at(node) == relay_base);
        _insert_in_bc(node, appendant_suffix);
    }
    
private:
    
    
};


// MARK: - Dynamic Double-array Interface

template <typename IndexType, bool LegacyBuild = true, bool Patricia = false>
class DoubleArray;


template <typename IndexType, bool LegacyBuild>
class DoubleArray<IndexType, LegacyBuild, false> :
    private _DynamicDoubleArrayMpTrieBehavior<_DoubleArrayBcMpTrieImpl<IndexType, not LegacyBuild>> {
    using _impl = _DoubleArrayBcMpTrieImpl<IndexType, not LegacyBuild>;
    using _behavior = _DynamicDoubleArrayMpTrieBehavior<_impl>;
    using _self = DoubleArray<IndexType, LegacyBuild, false>;
public:
    using input_trie = typename graph_util::Trie<char>;
    using index_type = typename _impl::_index_type;
    using char_type = typename _impl::_char_type;
    
    static constexpr index_type kRootIndex = _impl::kRootIndex;
    static constexpr char_type kLeafChar = graph_util::kLeafChar;
    
    static constexpr size_t kBlockSize = _impl::kBlockSize;
    
    DoubleArray() : _behavior() {}
    
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
    
    size_t num_nodes() const {
        size_t cnt = 0;
        for (size_t i = 0; i < _impl::_num_blocks(); i++) {
            cnt += 256 - _impl::_block_at(i).num_empties();
        }
        return cnt;
    }
    
    void insert(std::string_view key) {
        index_type node = kRootIndex;
        size_t pos = 0;
        for (; pos < key.size(); pos++) {
            if (_impl::_unit_at(node).has_label())
                break;
            if (not _transition(node, key[pos])) {
                _behavior::_insert_in_bc(node, key.substr(pos));
                return;
            }
        }
        auto leached_unit = _impl::_unit_at(node);
        if (leached_unit.has_label()) {
            auto tail_index = leached_unit.pool_index();
            for (; pos < key.size(); pos++, tail_index++) {
                if (_impl::tail_[tail_index] != char_type(key[pos])) {
                    _behavior::_insert_in_tail(node, tail_index, key.substr(pos));
                    return;
                }
            }
            if (_impl::tail_[tail_index] != kLeafChar) {
                _behavior::_insert_in_tail(node, tail_index, "");
            }
        } else {
            if (not leached_unit.terminal()) {
                leached_unit.set_terminal();
            }
        }
    }
    
    bool erase(std::string_view key) {
        index_type node = kRootIndex;
        size_t pos = 0;
        for (; pos < key.size(); pos++) {
            if (_impl::_unit_at(node).has_label()) {
                break;
            }
            if (not _transition(node, key[pos])) {
                return false;
            }
        }
        auto leached_unit = _impl::_unit_at(node);
        if (leached_unit.has_label()) {
            auto tail_index = leached_unit.pool_index();
            for (; pos < key.size(); pos++, tail_index++) {
                if (_impl::tail_[tail_index] != key[pos])
                    return false;
            }
            if (_impl::tail_[tail_index] == kLeafChar)
                _impl::_delete_leaf(node);
        } else {
            if (leached_unit.terminal())
                _impl::_delete_leaf(node);
        }
        return true;
    }
    
    bool accept(std::string_view key) const {
        index_type node = kRootIndex;
        size_t pos = 0;
        for (; pos < key.size(); pos++) {
            if (_impl::_unit_at(node).has_label()) {
                break;
            }
            if (not _transition(node, key[pos])) {
                return false;
            }
        }
        auto leached_unit = _impl::_unit_at(node);
        if (leached_unit.has_label()) {
            auto tail_index = leached_unit.pool_index();
            for (; pos < key.size(); pos++, tail_index++) {
                if (_impl::tail_[tail_index] != key[pos])
                    return false;
            }
            return _impl::tail_[tail_index] == kLeafChar;
        } else {
            return leached_unit.terminal();
        }
    }
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex] \tterminal, \tcheck, \tsibling, \tbase, \tchild"  << std::endl;
        for (size_t i = 0; i < _impl::_num_elements(); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            std::cout << "\t\t"<<i<<"] \t";
            auto empty =_impl::_empty(i);
            if (not empty) {
                auto unit = _impl::_unit_at(i);
                std::cout<<unit.terminal()<<", \t"<<unit.check()<<", \t"<<unit.sibling()<<", \t"<<unit.has_label();
                if (not _impl::_is_edge_at(i))
                    std::cout<<", \t"<<_impl::_base_at(i)<<", \t"<<unit.child();
            }
            std::cout << std::endl;
        }
    }
    
private:
    bool _transition(index_type& node, char_type c) const {
        if (_impl::_is_edge_at(node))
            return false;
        auto next = _impl::_base_at(node) xor c;
        if (_impl::_empty(next) or
            _impl::_unit_at(next).check() != node)
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
            _impl::_unit_at(co_node).set_pool_index(tail_index);
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


template <typename IndexType, bool LegacyBuild>
class DoubleArray<IndexType, LegacyBuild, true> :
    private _DynamicDoubleArrayPatriciaTrieBehavior<_DoubleArrayBcPatriciaTrieImpl<IndexType, not LegacyBuild>> {
    using _impl = _DoubleArrayBcPatriciaTrieImpl<IndexType, not LegacyBuild>;
    using _behavior = _DynamicDoubleArrayPatriciaTrieBehavior<_impl>;
    using _self = DoubleArray<IndexType, LegacyBuild, false>;
public:
    using input_trie = typename graph_util::Trie<char>;
    using index_type = typename _behavior::_index_type;
    using char_type = typename _behavior::_char_type;
    
    static constexpr index_type kRootIndex = _impl::kRootIndex;
    static constexpr char_type kLeafChar = '\0';
    
    static constexpr size_t kBlockSize = _behavior::kBlockSize;
    
    DoubleArray() : _behavior() {}
    
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
    
    size_t num_nodes() const {
        size_t cnt = 0;
        for (size_t i = 0; i < _impl::_num_blocks(); i++) {
            cnt += 256 - _impl::_block_at(i).num_empties();
        }
        return cnt;
    }
    
    bool accept(std::string_view key) const {
        return _traverse(key, []{}, []{}, []{});
    }
    
    bool insert(std::string_view key) {
        return not _traverse(key, [](auto, auto){},
                             [&](index_type node, std::string_view key, size_t key_pos) {
                                 _behavior::_insert_in_bc(node, key_pos < key.size() ? key.substr(key_pos) : "");
                             },
                             [&](index_type node, size_t label_pos, std::string_view key, size_t key_pos) {
                                 _behavior::_insert_in_pool(node, label_pos, key_pos < key.size() ? key.substr(key_pos) : "");
                             });
    }
    
    bool erase(std::string_view key) {
        return _traverse(key,
                         [&](index_type node, std::string_view key) {
                             _behavior::_delete_leaf(node);
                         }, []{}, []{});
    }
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex] \texists, \tcheck, \tsibling, \tbase, \tchild"  << std::endl;
        for (size_t i = 0; i < _impl::_num_elements(); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            std::cout << "\t\t"<<i<<"] \t";
            auto empty =_impl::_empty(i);
            if (not empty) {
                auto unit = _impl::_unit_at(i);
                std::cout<<unit.terminal()<<", \t"<<unit.check()<<", \t"<<unit.sibling()<<", \t"<<unit.has_label();
                if (not _impl::_is_edge_at(i))
                    std::cout<<", \t"<<_impl::_base_at(i)<<", \t"<<unit.child();
            }
            std::cout << std::endl;
        }
    }
    
private:
    template <class SuccessAction, class FailedInBcAction, class FailedInPoolAction>
    bool _traverse(std::string_view key, SuccessAction success, FailedInBcAction failed_in_bc, FailedInPoolAction failed_in_pool) const {
        index_type node = kRootIndex;
        for (size_t key_pos = 0; key_pos < key.size(); key_pos++) {
            if (not _transition(node, key[key_pos])) {
                failed_in_bc(node, key, key_pos);
                return false;
            }
            if (_impl::_unit_at(node).has_label() and not _transition_with_label(node, key, ++key_pos, failed_in_pool))
                return false;
        }
        if (not _impl::_unit_at(node).terminal()) {
            failed_in_bc(node, key, key.size());
            return false;
        }
        success(node, key);
        return true;
    }
    
    bool _transition(index_type& node, char_type c) const {
        if (_impl::_is_edge_at(node))
            return false;
        auto next = _behavior::_base_at(node) xor c;
        if (_behavior::_empty(next) or
            _behavior::_unit_at(next).check() != node)
            return false;
        node = next;
        return true;
    }
    
    template <class FailedAction>
    bool _transition_with_label(index_type node, std::string_view key, size_t& key_pos, FailedAction failed) const {
        assert(_impl::_unit_at(node).has_label());
        auto label_index = _impl::_unit_at(node).pool_index() + _impl::kIndexBytes;
        assert(_impl::tail_[label_index] != kLeafChar);
        size_t i = 0;
        while (key_pos < key.size()) {
            char_type char_in_label = _impl::tail_[label_index+i];
            if (char_in_label == kLeafChar) {
                --key_pos;
                return true;
            }
            if (char_in_label != (char_type)key[key_pos]) {
                failed(node, label_index+i, key, key_pos);
                return false;
            }
            i++;
            key_pos++;
        }
        if (not (_impl::tail_[label_index+i] == kLeafChar)) {
            failed(node, label_index+i, key, key_pos);
            return false;
        }
        return true;
    }
    
    void _arrange_da(const _self& da, index_type node, index_type co_node) {
        if (da._behavior::_is_leaf(node)) {
            auto tail_index = _behavior::_insert_suffix(da._behavior::_suffix_in_tail(da._behavior::_tail_index(node)));
            _behavior::_set_tail_index(co_node, tail_index);
            return;
        }
        std::vector<char_type> children;
        da._behavior::_for_each_children(node, [&](auto child, auto) {
            children.push_back(child);
        });
        
        auto new_base = _behavior::_find_base(children);
        _behavior::_insert_nodes(co_node, children, new_base);
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
        
        auto new_base = _behavior::_find_base(children);
        _behavior::_insert_nodes(co_node, children, new_base);
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
            auto tail_index = _behavior::_insert_suffix((*begin).size() > depth ? std::string_view((const char*)&(*begin) + depth) : "");
            _behavior::_unit_at(co_node).set_pool_index(tail_index);
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
        
        auto new_base = _behavior::_find_base(children);
        _behavior::_insert_nodes(co_node, children, new_base);
        for (size_t i = 0; i < children.size(); i++) {
            auto c = children[i];
            auto next = new_base xor c;
            _arrange_keysets(iters[i], iters[i+1], depth+1, next);
        }
    }
    
};

} // namespace sim_ds

#endif /* DoubleArray_hpp */
