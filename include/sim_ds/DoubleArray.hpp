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
//      Base: Pointer
//          if disabled:
//              1 0 0 *(pred)
//          else if basic base:
//              0 0 0 *(base)
//          else if label id:
//              if suffix:
//              0 1 1 *(label id)
//              else:
//              0 1 0 *(label id)
//      Check: Pointer
//          if disabled:
//              1 0 *(succ)
//          else:
//              0 ?(terminal flag) *(check)
//  Paremers for siblings link
//      Child: Byte
//          *(child char)
//      Sibling: Byte
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
    static constexpr _index_type kThirdBit = kUpperBit >> 2;
    static constexpr _index_type kEmptyFlag = kUpperBit;
    static constexpr _index_type kTerminalFlag = kSecondBit;
    static constexpr _index_type kLabelFlag = kSecondBit;
    static constexpr _index_type kSuffixFlag = kThirdBit;
    static constexpr _index_type kIndexMask = kThirdBit - 1;
    static constexpr _index_type kIndexMax = kIndexMask;
    static constexpr _char_type kEmptyChar = _Da::kEmptyChar;
    
    static constexpr size_t kCheckInsets = 0;
    static constexpr size_t kSiblingInsets = kIndexBytes;
    static constexpr size_t kBaseInsets = kIndexBytes + 1;
    static constexpr size_t kChildInsets = kIndexBytes * 2 + 1;
    static constexpr size_t kTerminalFlagInsets = kCheckInsets + kIndexBytes - 1;
    static constexpr size_t kEdgeFlagInsets = kBaseInsets + kIndexBytes - 1;
    static constexpr size_t kLabelFlagInsets = kEdgeFlagInsets;
    static constexpr size_t kSuffixFlagInsets = kLabelFlagInsets;
    static constexpr size_t kSuccInsets = kBaseInsets;
    static constexpr size_t kPredInsets = kCheckInsets;
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
    using _common::kUnitBytes;
    using _common::kIndexMask;
    using _common::kIndexMax;
    using _common::kEmptyChar;
    
    using _common::kEmptyFlag;
    using _common::kCheckInsets;
    using _common::kSiblingInsets;
    using _common::kBaseInsets;
    using _common::kEdgeFlagInsets;
    using _common::kTerminalFlag;
    using _common::kTerminalFlagInsets;
    using _common::kLabelFlag;
    using _common::kLabelFlagInsets;
    using _common::kSuffixFlag;
    using _common::kSuffixFlagInsets;
    using _common::kChildInsets;
    
    using _common::kSuccInsets;
    using _common::kPredInsets;
    
private:
    _unit_storage_pointer pointer_;
    
    friend typename _Da::_self;
    friend class _DoubleArrayUnitBcpConstReference<_Da>;
    
public:
    bool terminal() const {return *(pointer_ + kTerminalFlagInsets) bitand 0x40;}
    
    void enable_terminal() {
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
    
    bool label_is_suffix() const {return *(pointer_ + kSuffixFlagInsets) bitand 0x20;}
    
    _index_type pool_index() const {assert(has_label()); return base();}
    
    void set_pool_index(_index_type new_pool_index, bool label_is_suffix) {
        *reinterpret_cast<_index_type*>(pointer_ + kBaseInsets) = (new_pool_index bitand kIndexMask) bitor kLabelFlag bitor (label_is_suffix ? kSuffixFlag : 0);
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
    
    _index_type pred() const {
        assert(*reinterpret_cast<_index_type*>(pointer_ + kPredInsets) bitand kEmptyFlag);
        return *reinterpret_cast<_index_type*>(pointer_ + kPredInsets) bitand kIndexMask;
    }
    
    void set_pred(_index_type new_pred) {
        *reinterpret_cast<_index_type*>(pointer_ + kPredInsets) = (new_pred bitand kIndexMask) bitor kEmptyFlag;
    }
    
    _index_type succ() const {
        assert(*reinterpret_cast<_index_type*>(pointer_ + kSuccInsets) bitand kEmptyFlag);
        return *reinterpret_cast<_index_type*>(pointer_ + kSuccInsets) bitand kIndexMask;
    }
    
    void set_succ(_index_type new_succ) {
        *reinterpret_cast<_index_type*>(pointer_ + kSuccInsets) = (new_succ bitand kIndexMask) bitor kEmptyFlag;
    }
    
    void init(_index_type pred, _index_type succ) {
        set_pred(pred);
        set_sibling(kEmptyChar);
        set_succ(succ);
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
    using _common::kUnitBytes;
    using _common::kIndexMask;
    using _common::kIndexMax;
    using _common::kEmptyChar;
    
    using _common::kEmptyFlag;
    using _common::kCheckInsets;
    using _common::kSiblingInsets;
    using _common::kBaseInsets;
    using _common::kEdgeFlagInsets;
    using _common::kTerminalFlag;
    using _common::kTerminalFlagInsets;
    using _common::kLabelFlag;
    using _common::kLabelFlagInsets;
    using _common::kSuffixFlag;
    using _common::kSuffixFlagInsets;
    using _common::kChildInsets;
    
    using _common::kSuccInsets;
    using _common::kPredInsets;
    
private:
    _unit_storage_pointer pointer_;
    
    friend typename _Da::_self;
    
public:
    _DoubleArrayUnitBcpConstReference(const _DoubleArrayUnitBcpReference<_Da>& x) : pointer_(x.pointer_) {}
    
    bool terminal() const {return *(pointer_ + kTerminalFlagInsets) bitand 0x40;}
    
    _index_type check() const {return *reinterpret_cast<const _index_type*>(pointer_ + kCheckInsets) bitand kIndexMask;}
    
    _index_type base() const {return *reinterpret_cast<const _index_type*>(pointer_ + kBaseInsets) bitand kIndexMask;}
    
    bool has_label() const {return *(pointer_ + kLabelFlagInsets) bitand 0x40;}
    
    bool label_is_suffix() const {return *(pointer_ + kSuffixFlagInsets) bitand 0x20;}
    
    _index_type pool_index() const {assert(has_label()); return base();}
    
    _char_type child() const {return *reinterpret_cast<const _char_type*>(pointer_ + kChildInsets);}
    
    _char_type sibling() const {return *reinterpret_cast<const _char_type*>(pointer_ + kSiblingInsets);}
    
    bool base_empty() const {return *(pointer_ + kEdgeFlagInsets) bitand 0x80;}
    
    _index_type pred() const {
        assert(*reinterpret_cast<const _index_type*>(pointer_ + kPredInsets) bitand kEmptyFlag);
        return *reinterpret_cast<const _index_type*>(pointer_ + kPredInsets) bitand kIndexMask;
    }
    
    _index_type succ() const {
        assert(*reinterpret_cast<const _index_type*>(pointer_ + kSuccInsets) bitand kEmptyFlag);
        return *reinterpret_cast<const _index_type*>(pointer_ + kSuccInsets) bitand kIndexMask;
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
    
    static constexpr size_t kPredOffset = 0;
    static constexpr size_t kSuccOffset = kPredOffset + 1;
    static constexpr size_t kNumEmptiesOffset = kSuccOffset + 1;
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
    
    using _common::kPredOffset;
    using _common::kSuccOffset;
    using _common::kNumEmptiesOffset;
    using _common::kNumEmptiesMask;
    using _common::kErrorCountOffset;
    using _common::kErrorCountInset;
    using _common::kFieldSize;
    
    _block_pointer block_pointer_;
    
    friend typename _Dac::_self;
    
    friend class _DoubleArrayBlockConstReference<_Dac>;
    
public:
    _DoubleArrayBlockReference& init(_index_type pred, _index_type succ) {
        set_pred(pred);
        set_succ(succ);
        *(basic_ptr()+kNumEmptiesOffset) = kBlockSize;
        error_reset();
        bit_util::set256_epi1(1, field_ptr());
        return *this;
    }
    
    _block_pointer basic_ptr() {return block_pointer_ + kFieldSize;}
    
    _const_block_pointer basic_ptr() const {return block_pointer_ + kFieldSize;}
    
    _block_pointer field_ptr() {return block_pointer_;}
    
    _const_block_pointer field_ptr() const {return block_pointer_;}
    
    _index_type pred() const {return _index_type(*(basic_ptr()+kPredOffset));}
    
    void set_pred(_index_type pred) {
        *(basic_ptr()+kPredOffset) = pred;
    }
    
    _index_type succ() const {return _index_type(*(basic_ptr()+kSuccOffset));}
    
    void set_succ(_index_type succ) {
        *(basic_ptr()+kSuccOffset) = succ;
    }
    
    size_t num_empties() const {return *(basic_ptr()+kNumEmptiesOffset) bitand kNumEmptiesMask;}
    
    bool filled() const {return num_empties() == 0;}
    
    void consume(size_t num) {
        assert(num_empties() >= num);
        auto& t = *(basic_ptr()+kNumEmptiesOffset);
        t = (t bitand compl kNumEmptiesMask) | (num_empties() - num);
        assert(num_empties() == bit_util::popcnt256(field_ptr()));
    }
    
    void refill(size_t num) {
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
    
    using _common::kPredOffset;
    using _common::kSuccOffset;
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
    
    _index_type pred() const {return _index_type(*(basic_ptr()+kPredOffset));}
    
    _index_type succ() const {return _index_type(*(basic_ptr()+kSuccOffset));}
    
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
    _DoubleArrayBlockLegacyReference& init(_index_type pred, _index_type succ) {
        _base::init(pred, succ);
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
        _set_new_node(kRootIndex, kIndexMax, kEmptyChar, false);
        _consume_block(_block_index_of(kRootIndex), 1);
    }
    
    explicit _DoubleArrayBcMpTrieImpl(std::ifstream& ifs) {
        general_block_head_ = read_val<_index_type>(ifs);
        personal_block_head_ = read_val<_index_type>(ifs);
        read_vec(ifs, basic_block_);
        read_vec(ifs, container_);
        read_vec(ifs, tail_);
    }
    
    virtual ~_DoubleArrayBcMpTrieImpl() = default;
    
    size_t _size_in_bytes() const {
        return (sizeof(general_block_head_) +
                sizeof(personal_block_head_) +
                size_vec(basic_block_) +
                size_vec(container_) +
                size_vec(tail_));
    }
    
    void _serialize(std::ofstream& ofs) const {
        write_val(general_block_head_, ofs);
        write_val(personal_block_head_, ofs);
        write_vec(basic_block_, ofs);
        write_vec(container_, ofs);
        write_vec(tail_, ofs);
    }
    
    size_t _bc_size() const {return container_.size() / kUnitBytes;}
    
    size_t _block_size() const {return basic_block_.size() / kBlockQBytes;}
    
    _unit_reference _unit_at(_index_type index) {
        return _unit_reference(container_.data() + (kUnitBytes * index));
    }
    
    _unit_const_reference _unit_at(_index_type index) const {
        return _unit_const_reference(container_.data() + (kUnitBytes * index));
    }
    
    _index_type _block_index_of(_index_type index) const {return index/kBlockSize;}
    
    _block_reference _block_at(_index_type block) {
        assert(block < _block_size());
        return _block_reference(basic_block_.data() + kBlockQBytes * block);
    }
    
    _block_const_reference _block_at(_index_type block) const {
        assert(block < _block_size());
        return _block_const_reference(basic_block_.data() + kBlockQBytes * block);
    }
    
    void _expand() {
        if (_bc_size() > kIndexMax) {
            throw "Index out-of-range! You should set large byte-size of template parameter.";
        }
        container_.resize(container_.size() + kUnitBytes * kBlockSize);
        
        // Append blocks linking
        basic_block_.resize(basic_block_.size() + kBlockQBytes);
        auto back = _block_size() - 1;
        if (general_block_head_ == kInitialEmptyBlockHead) {
            _block_at(back).init(back, back);
            general_block_head_ = back;
        } else {
            auto pred = _block_at(general_block_head_).pred();
            _block_at(general_block_head_).set_pred(back);
            _block_at(pred).set_succ(back);
            _block_at(back).init(pred, general_block_head_);
        }
        
        // Link elements in appended block
        auto front = _bc_size() - kBlockSize;
        auto inset_back = kBlockSize - 1;
        _unit_at(front).init(inset_back, 1);
        for (size_t i = 1; i < inset_back; i++) {
            _unit_at(front+i).init(i-1, i+1);
        }
        _unit_at(front+inset_back).init(inset_back-1, 0);
    }
    
    void _shrink() {
        if (_block_size() == 1)
            return;
        assert(_block_at(_bc_size()-1).num_empties() == kBlockSize);
        container_.resize(container_.size() - kUnitBytes * kBlockSize);
        basic_block_.resize(basic_block_.size() - kBlockQBytes);
    }
    
    bool _empty(size_t index) const {
        return _block_at(_block_index_of(index)).empty_element_at(index%kBlockSize);
    }
    
    void _freeze(size_t index) {
        assert(_empty(index));
        // Remove empty-elements linking
        auto succ = _unit_at(index).succ();
        auto pred = _unit_at(index).pred();
        auto block_index = _block_index_of(index);
        auto offset = kBlockSize * (block_index);
        _unit_at(offset+succ).set_pred(pred);
        _unit_at(offset+pred).set_succ(succ);
        auto b = _block_at(block_index);
        auto inset = index % kBlockSize;
        if (succ == inset) {
            b.disable_link();
        } else if (inset == b.empty_head()) {
            b.set_empty_head(succ);
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
            auto tail = head_unit.pred();
            head_unit.set_pred(inset);
            _unit_at(offset+tail).set_succ(inset);
            _unit_at(index).init(tail, head);
        }
    }
    
    void _setup(size_t index) {
        if (index >= _bc_size())
            _expand();
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
    
    void _set_new_node(_index_type index, _index_type new_check, _char_type new_sibling, bool terminal) {
        _setup(index);
        auto unit = _unit_at(index);
        unit.set_check(new_check);
        unit.set_sibling(new_sibling);
        if (terminal) {
            unit.enable_terminal();
        }
    }
    
    void _set_new_node_by_label(_index_type index, _index_type new_check, _char_type new_sibling, bool terminal, _index_type new_pool_index, bool label_is_suffix) {
        _set_new_node(index, new_check, new_sibling, terminal);
        _unit_at(index).set_pool_index(new_pool_index, label_is_suffix);
    }
    
    std::string_view _suffix_in_tail(_index_type tail_index) const {
        return std::string_view((char*)tail_.data() + tail_index);
    }
    
    void _freeze_block_in_general(_index_type block) {
        auto target_block_ref = _block_at(block);
        assert(target_block_ref.error_count() < kErrorThreshold);
        auto succ = target_block_ref.succ();
        if (succ == block) {
            assert(general_block_head_ == block);
            general_block_head_ = kInitialEmptyBlockHead;
            return;
        }
        auto pred = target_block_ref.pred();
        assert(pred != block);
        _block_at(succ).set_pred(pred);
        _block_at(pred).set_succ(succ);
        if (block == general_block_head_) {
            general_block_head_ = succ;
        }
    }
    
    void _freeze_block_in_personal(_index_type block) {
        auto target_block_ref = _block_at(block);
        assert(target_block_ref.error_count() >= kErrorThreshold);
        auto succ = target_block_ref.succ();
        if (succ == block) {
            assert(personal_block_head_ == block);
            personal_block_head_ = kInitialEmptyBlockHead;
            return;
        }
        auto pred = target_block_ref.pred();
        assert(pred != block);
        _block_at(succ).set_pred(pred);
        _block_at(pred).set_succ(succ);
        if (block == personal_block_head_) {
            personal_block_head_ = succ;
        }
    }
    
    void _modify_block_to_general(_index_type block) {
        auto target_block_ref = _block_at(block);
        if (general_block_head_ == kInitialEmptyBlockHead) {
            target_block_ref.set_pred(block);
            target_block_ref.set_succ(block);
            general_block_head_ = block;
        } else {
            auto tail = _block_at(general_block_head_).pred();
            assert(tail != block);
            _block_at(general_block_head_).set_pred(block);
            target_block_ref.set_succ(general_block_head_);
            target_block_ref.set_pred(tail);
            _block_at(tail).set_succ(block);
        }
    }
    
    void _modify_block_to_personal(_index_type block) {
        auto target_block_ref = _block_at(block);
        if (personal_block_head_ == kInitialEmptyBlockHead) {
            target_block_ref.set_pred(block);
            target_block_ref.set_succ(block);
            personal_block_head_ = block;
        } else {
            auto tail = _block_at(personal_block_head_).pred();
            _block_at(personal_block_head_).set_pred(block);
            target_block_ref.set_succ(personal_block_head_);
            target_block_ref.set_pred(tail);
            _block_at(tail).set_succ(block);
        }
    }
    
    void _error_block(_index_type block) {
        auto b = _block_at(block);
        assert(_block_at(block).error_count() < kErrorThreshold);
        if (b.error_count() + 1 >= kErrorThreshold) {
            _freeze_block_in_general(block);
            _modify_block_to_personal(block);
        }
        b.errored();
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
        auto b = _block_at(block);
        if (b.filled()) {
            _modify_block_to_general(block);
        } else if (b.error_count() >= kErrorThreshold) {
            _freeze_block_in_personal(block);
            _modify_block_to_general(block);
        }
        b.error_reset();
        b.refill(num);
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
            action(child);
            if (sibling == first_child)
                break;
            child = sibling;
        }
    }
    
    size_t _num_of_children(_index_type node) const {
        if (_is_edge_at(node))
            return 0;
        size_t cnt = 0;
        _for_each_children(node, [&cnt](auto) {++cnt;});
        return cnt;
    }
    
    struct _internal_label_container {
        std::string_view label;
        bool terminal:1;
        bool suffix:1;
    };
    
    virtual void _insert_nodes(_index_type node, std::vector<_internal_label_container>& label_datas, _index_type base) {
        if (base >= _bc_size())
            _expand();
        _set_new_trans(node, base, label_datas.front().label.front());
        for (size_t i = 0; i < label_datas.size(); i++) {
            auto label = label_datas[i].label;
            assert(not label.empty());
            auto next = base xor (_char_type)label.front();
            if (label.size() == 1) {
                _set_new_node(next, node, label_datas[(i+1)%label_datas.size()].label.front(), label_datas[i].terminal);
            } else {
                _set_new_node_by_label(next, node,
                                       label_datas[(i+1)%label_datas.size()].label.front(),
                                       label_datas[i].terminal,
                                       _append_suffix_in_pool(label.substr(1)),
                                       true);
            }
        }
        _consume_block(_block_index_of(base), label_datas.size());
    }
    
    _index_type _append_suffix_in_pool(std::string_view suffix) {
        assert(not suffix.empty());
        _index_type index = tail_.size();
        tail_.resize(index + suffix.size() + 1);
        for (size_t i = 0; i < suffix.size(); i++)
            tail_[index+i] = suffix[i];
        tail_[index+suffix.size()] = kLeafChar;
        return index;
    }
    
    _index_type _new_base(_char_type c) const {
        if constexpr (BitOperationalFind) {
            return _bc_size();
        } else {
            return _bc_size() xor c;
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
                    auto succ = _unit_at(index).succ();
                    if (succ == block.empty_head())
                        break;
                    index = offset + succ;
                }
            }
            
            auto new_b = block.succ();
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
    
    _DynamicDoubleArrayMpTrieBehavior(std::ifstream& ifs) : _impl(ifs) {}
    
    virtual ~_DynamicDoubleArrayMpTrieBehavior() = default;
    
    virtual _index_type _grow(_index_type node, _index_type base, _char_type c) {
        _impl::_set_new_trans(node, base, c);
        auto next = base xor c;
        _impl::_set_new_node(next, node, c, false);
        _impl::_consume_block(_impl::_block_index_of(next), 1);
        return next;
    }
    
    virtual void _insert_in_bc(_index_type node, std::string_view suffix) {
        if (suffix.size() > 0) {
            node = _insert_trans(node, suffix.front());
            if (suffix.size() > 1) {
                auto pool_index = _impl::_append_suffix_in_pool(suffix.substr(1));
                _impl::_unit_at(node).set_pool_index(pool_index, true);
            }
        }
        _impl::_unit_at(node).enable_terminal();
    }
    
    void _insert_in_tail(_index_type node, _index_type tail_pos, std::string_view additional_suffix) {
        auto tail_index = _impl::_unit_at(node).pool_index();
        _impl::_unit_at(node).disable_terminal();
        while (tail_index < tail_pos) {
            _char_type c = _impl::tail_[tail_index++];
            node = _grow(node, _impl::_find_base({c}), c);
        }
        _char_type char_at_confliction = _impl::tail_[tail_pos];
        if (char_at_confliction != kLeafChar) {
            auto next = _grow(node, _impl::_find_base({char_at_confliction, (_char_type)additional_suffix.front()}), char_at_confliction);
            auto unit = _impl::_unit_at(next);
            if (_impl::tail_[tail_pos+1] != kLeafChar)
                unit.set_pool_index(tail_pos+1, true);
            unit.enable_terminal();
        } else {
            _impl::_unit_at(node).enable_terminal();
        }
        _insert_in_bc(node, additional_suffix);
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
            _index_type pred = _impl::_unit_at(node).check();
            counts_children = _impl::_num_of_children(pred);
            // Erase current char from siblings link.
            auto base = _impl::_base_at(pred);
            _char_type first_child = _impl::_unit_at(pred).child();
            if ((base xor first_child) == node) {
                auto unit = _impl::_unit_at(base xor first_child);
                _char_type child = unit.sibling();
                _index_type back_index = base xor first_child;
                while (child != first_child) {
                    back_index = base xor child;
                    child = _impl::_unit_at(back_index).sibling();
                }
                _char_type second_child = unit.sibling();
                _impl::_unit_at(back_index).set_sibling(second_child);
                _impl::_unit_at(pred).set_child(counts_children != 1 ? second_child : kEmptyChar);
            } else {
                auto pred_unit = _impl::_unit_at(base xor first_child);
                for (_char_type child = pred_unit.sibling(); ; ) {
                    auto next = base xor child;
                    auto sibling = _impl::_unit_at(next).sibling();
                    if (next == node) {
                        pred_unit.set_sibling(sibling);
                        break;
                    }
                    child = sibling;
                }
            }
            
            _impl::_erase(node);
            node = pred;
        } while (node != kRootIndex and counts_children == 1);
        _reduce();
    }
    
private:
    friend class _DynamicDoubleArrayPatriciaTrieBehavior<_impl>;
    
    struct _moving_luggage {
        bool terminal:1;
        bool is_edge:1;
        bool has_label:1;
        bool label_is_suffix:1;
        _index_type target;
        uint8_t child;
        _moving_luggage(bool terminal, bool is_edge, bool has_label, bool label_is_suffix, _index_type base, uint8_t child) : terminal(terminal), is_edge(is_edge), has_label(has_label), label_is_suffix(label_is_suffix), target(base), child(child) {}
    };
    
    struct _shelter {
        _index_type node;
        std::vector<_char_type> children;
        std::vector<_moving_luggage> luggages;
    };
    
    void _evacuate(_index_type node, _shelter& shelter) {
        shelter.node = node;
        auto base = _impl::_base_at(node);
        _impl::_for_each_children(node, [&](_char_type child) {
            shelter.children.push_back(child);
            auto index = base xor child;
            auto index_unit = _impl::_unit_at(index);
            shelter.luggages.emplace_back(index_unit.terminal(), _impl::_is_edge_at(index), index_unit.has_label(), index_unit.label_is_suffix(), index_unit.base(), index_unit.child());
            _impl::_erase(index);
        });
        _impl::_refill_block(_impl::_block_index_of(base), shelter.children.size());
    }
    
    void _update_node(_index_type index, _index_type check, _index_type sibling, const _moving_luggage& luggage) {
        if (not luggage.has_label) {
            _impl::_set_new_node(index, check, sibling, luggage.terminal);
            if (not luggage.is_edge)
                _impl::_set_new_trans(index, luggage.target, luggage.child);
        } else {
            _impl::_set_new_node_by_label(index, check, sibling, luggage.terminal, luggage.target, luggage.label_is_suffix);
            if (not luggage.is_edge)
                _impl::_unit_at(index).set_child(luggage.child);
        }
    }
    
    void _update_check(_index_type base, _char_type child, _index_type new_check) {
        assert(base < _impl::_bc_size());
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
        auto old_base = _impl::_base_at(shelter.node);
        _impl::_set_base_at(shelter.node, new_base);
        for (size_t i = 0; i < shelter.children.size(); i++) {
            auto child = shelter.children[i];
            auto sibling = shelter.children[(i+1)%shelter.children.size()];
            auto new_next = new_base xor child;
            auto& luggage = shelter.luggages[i];
            _update_node(new_next, shelter.node, sibling, luggage);
            if (not luggage.is_edge)
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
        _impl::_set_base_at(shelter.node, target_base);
        std::vector<_shelter> shelters;
        for (size_t i = 0; i < shelter.children.size(); i++) {
            auto child = shelter.children[i];
            auto sibling = shelter.children[(i+1)%shelter.children.size()];
            auto new_next = target_base xor child;
            if (not _impl::_empty(new_next)) {
                // Evacuate already placed siblings membering element at conflicting index to shelter.
                shelters.emplace_back();
                auto conflicting_node = _impl::_unit_at(new_next).check();
                _evacuate(conflicting_node, shelters.back());
            }
            auto& luggage = shelter.luggages[i];
            _update_node(new_next, shelter.node, sibling, luggage);
            if (not _impl::_is_edge_at(new_next))
                _update_check(_impl::_base_at(new_next), luggage.child, new_next);
        }
        _impl::_consume_block(_impl::_block_index_of(target_base), shelter.children.size());
        
        // Compress sheltered siblings recursively.
        for (auto& sht : shelters) {
            _compress_nodes(sht, _find_compression_target(sht.children));
        }
        
        return;
    }
    
    _index_type _find_compression_target(std::vector<_char_type>& siblings) const {
        if (siblings.size() == 1 and _impl::personal_block_head_ != kInitialEmptyBlockHead) {
            _index_type pbh = _impl::personal_block_head_;
            return (kBlockSize * pbh + _impl::_block_at(pbh).empty_head()) xor siblings.front();
        }
        if (_impl::general_block_head_ == kInitialEmptyBlockHead) {
            return _impl::_bc_size();
        }
        _index_type b = _impl::general_block_head_;
        while (true) {
            if (b == _impl::_block_index_of(_impl::_bc_size()-1)) {
                b = _impl::_block_at(_impl::_block_index_of(_impl::_bc_size()-1)).succ();
                if (b == _impl::general_block_head_)
                    return _impl::_bc_size();
                continue;
            }
            auto block = _impl::_block_at(b);
            assert(block.link_enabled());
            const auto offset = kBlockSize * b;
            for (auto index = offset + block.empty_head(); ; ) {
                _index_type n = index xor siblings.front();
                bool skip = false;
                for (_char_type c : siblings) {
                    auto target = n xor c;
                    if (not _impl::_empty(target) and
                        _impl::_num_of_children(_impl::_unit_at(target).check()) >= siblings.size()) {
                        skip = true;
                        break;
                    }
                }
                if (not skip) {
                    return n;
                }
                auto succ = _impl::_unit_at(index).succ();
                if (succ == block.empty_head())
                    break;
                index = offset + succ;
            }
            if (block.succ() == _impl::general_block_head_) {
                return _impl::_bc_size();
            }
            b = block.succ();
        }
    }
    
    void _reduce() {
        using bit_util::ctz256;
        for (_index_type b = _impl::_block_index_of(_impl::_bc_size()-1); b > 0; b--) {
            for (size_t ctz = ctz256(_impl::_block_at(b).field_ptr()); ctz < 256; ctz = ctz256(_impl::_block_at(b).field_ptr())) {
                auto parent = _impl::_unit_at(kBlockSize*b+ctz).check();
                auto base = _impl::_base_at(parent);
                _shelter shelter;
                shelter.node = parent;
                _impl::_for_each_children(parent, [&](_char_type child) {
                    shelter.children.push_back(child);
                });
                auto compression_target = _find_compression_target(shelter.children);
                if (_impl::_block_index_of(compression_target) >= b)
                    return;
                _impl::_for_each_children(parent, [&](_char_type child) {
                    auto next = base xor child;
                    auto unit = _impl::_unit_at(next);
                    shelter.luggages.emplace_back(unit.terminal(), _impl::_is_edge_at(next), unit.has_label(), unit.label_is_suffix(), unit.base(), unit.child());
                    _impl::_erase(next);
                });
                _impl::_refill_block(_impl::_block_index_of(base), shelter.children.size());
                _compress_nodes(shelter, compression_target);
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
            _impl::_consume_block(_impl::_block_index_of(conflicting_index), 1);
            auto new_base = _impl::_find_base(shelter.children);
            _impl::_thaw(conflicting_index);
            _impl::_refill_block(_impl::_block_index_of(conflicting_index), 1);
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
    using _base = _DoubleArrayBcMpTrieImpl<IndexType, BitOperationalFind>;
public:
    using typename _base::_index_type;
    using typename _base::_char_type;
    
    using _base::kLeafChar;
    using _base::kEmptyChar;
    using _base::kIndexBytes;
    using _base::kEmptyFlag;
    using _base::kIndexMask;
    
protected:
    _DoubleArrayBcPatriciaTrieImpl() : _base() {}
    
    _DoubleArrayBcPatriciaTrieImpl(std::ifstream& ifs) : _base(ifs) {}
    
    virtual ~_DoubleArrayBcPatriciaTrieImpl() = default;
    
    bool _is_edge_at(_index_type node) const override {
        auto unit = _base::_unit_at(node);
        return unit.base_empty() or (unit.has_label() and unit.label_is_suffix());
    }
    
    _index_type _base_in_pool(_index_type pool_index) const {
        const _index_type* base_ptr = reinterpret_cast<const _index_type*>(_base::tail_.data() + pool_index);
        return *base_ptr bitand kIndexMask;
    }
    
    void _set_base_in_pool(_index_type pool_index, _index_type new_base) {
        _index_type* base_ptr = reinterpret_cast<_index_type*>(_base::tail_.data() + pool_index);
        *base_ptr = new_base bitand kIndexMask;
    }
    
    _index_type _base_at(_index_type node) const override {
        assert(not _is_edge_at(node));
        auto unit = _base::_unit_at(node);
        return not unit.has_label() ? unit.base() : _base_in_pool(unit.pool_index());
    }
    
    void _set_base_at(_index_type node, _index_type base) override {
        auto unit = _base::_unit_at(node);
        if (not unit.has_label()) {
            unit.set_base(base);
        } else {
            assert(not unit.label_is_suffix());
            _set_base_in_pool(unit.pool_index(), base);
        }
    }
    
    _index_type _label_index_at(_index_type node) const {
        auto unit = _base::_unit_at(node);
        return unit.pool_index() + (unit.label_is_suffix() ? 0 : kIndexBytes);
    }
    
    std::string_view _label_in_pool(_index_type node) const {
        return std::string_view((const char*)_base::tail_.data() + _label_index_at(node));
    }
    
    _index_type _append_internal_label_in_pool(std::string_view key, _index_type new_base) {
        _index_type index = _base::tail_.size();
        _base::tail_.resize(_base::tail_.size() + kIndexBytes + key.size() + 1);
        _set_base_in_pool(index, new_base);
        for (size_t i = 0; i < key.size(); i++)
            _base::tail_[index+kIndexBytes+i] = key[i];
        _base::tail_[index+kIndexBytes+key.size()] = kLeafChar;
        return index;
    }
    
    void _insert_nodes(_index_type node, std::vector<typename _base::_internal_label_container>& label_datas, _index_type base) override {
        if (base >= _base::_bc_size())
            _base::_expand();
        _base::_set_new_trans(node, base, label_datas.front().label.front());
        for (size_t i = 0; i < label_datas.size(); i++) {
            auto label = label_datas[i].label;
            assert(not label.empty());
            auto next = base xor (_char_type)label.front();
            if (label.size() == 1) {
                _base::_set_new_node(next, node, label_datas[(i+1)%label_datas.size()].label.front(), label_datas[i].terminal);
            } else {
                auto pool_index = (label_datas[i].suffix ?
                                   _base::_append_suffix_in_pool(label.substr(1)) :
                                   _append_internal_label_in_pool(label.substr(1), kEmptyFlag));
                _base::_set_new_node_by_label(next, node,
                                       label_datas[(i+1)%label_datas.size()].label.front(),
                                       label_datas[i].terminal,
                                       pool_index,
                                       label_datas[i].suffix);
            }
        }
        _base::_consume_block(_base::_block_index_of(base), label_datas.size());
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
    
    _DynamicDoubleArrayPatriciaTrieBehavior(std::ifstream& ifs) : _base(ifs) {}
    
    virtual ~_DynamicDoubleArrayPatriciaTrieBehavior() = default;
    
    _index_type _grow(_index_type node, _index_type base, _char_type c) override {
        auto unit = _impl::_unit_at(node);
        assert(_impl::_is_edge_at(node));
        if (unit.has_label() and unit.label_is_suffix()) {
            auto new_pool_index = _impl::_append_internal_label_in_pool(_impl::_label_in_pool(node), base);
            unit.set_pool_index(new_pool_index, false);
            unit.set_child(c);
        } else {
            _impl::_set_new_trans(node, base, c);
        }
        auto next = base xor c;
        _impl::_set_new_node(next, node, c, false);
        _impl::_consume_block(_impl::_block_index_of(next), 1);
        return next;
    }
    
    void _insert_in_bc(_index_type node, std::string_view additional_suffix) override {
        if (not additional_suffix.empty()) {
            node = _base::_insert_trans(node, additional_suffix.front());
            if (additional_suffix.size() > 1) {
                auto pool_index = _impl::_append_suffix_in_pool(additional_suffix.substr(1));
                _impl::_unit_at(node).set_pool_index(pool_index, true);
            }
        }
        auto unit = _impl::_unit_at(node);
        unit.enable_terminal();
    }
    
    void _insert_in_pool(_index_type node, _index_type label_pos, std::string_view additional_suffix) {
        assert(_impl::_unit_at(node).has_label());
        auto forked_base = _impl::_find_base({_impl::tail_[label_pos], (_char_type)additional_suffix.front()});
        if (_impl::_unit_at(node).label_is_suffix()) {
            _fork_in_suffix(node, label_pos, forked_base);
        } else {
            _fork_in_internal_label(node, label_pos, forked_base);
        }
        _insert_in_bc(node, additional_suffix);
    }
    
private:
    void _fork_in_suffix(_index_type node, _index_type label_pos, _index_type forked_base) {
        auto node_unit = _impl::_unit_at(node);
        assert(node_unit.has_label() and
               node_unit.label_is_suffix());
        auto label_index = node_unit.pool_index();
        _char_type char_at_confliction = _impl::tail_[label_pos];
        assert(char_at_confliction != kLeafChar);
        auto left_label_length = label_pos - label_index;
        if (left_label_length == 0) {
            node_unit.set_base(forked_base);
        } else {
            std::string_view left_label((char*)_impl::tail_.data() + label_index, left_label_length);
            auto relay_pool_index = _impl::_append_internal_label_in_pool(left_label, forked_base);
            node_unit.set_pool_index(relay_pool_index, false);
        }
        node_unit.set_child(char_at_confliction);
        auto relay_next = forked_base xor char_at_confliction;
        if (_impl::tail_[label_pos + 1] == kLeafChar) { // Length of right-label is 1.
            _impl::_set_new_node(relay_next, node, char_at_confliction, true);
        } else {
            _impl::_set_new_node_by_label(relay_next, node, char_at_confliction, true, label_pos+1, true);
        }
        _impl::_consume_block(_impl::_block_index_of(relay_next), 1);
    }
    
    void _fork_in_internal_label(_index_type node, _index_type label_pos, _index_type forked_base) {
        auto node_unit = _impl::_unit_at(node);
        assert(node_unit.has_label() and
               not node_unit.label_is_suffix());
        const auto pool_index = node_unit.pool_index();
        const bool node_is_terminal = node_unit.terminal();
        node_unit.disable_terminal();
        
        const auto node_base = _impl::_base_in_pool(pool_index);
        const auto node_child = node_unit.child();
        
        auto label_index = pool_index + kIndexBytes;
        auto left_label_length = label_pos - label_index;
        auto right_label_length = 0;
        bool small_prefix = false;
        while (not small_prefix and _impl::tail_[label_pos + right_label_length] != kLeafChar) {
            small_prefix = left_label_length < ++right_label_length;
        }
        _char_type char_at_confliction = _impl::tail_[label_pos];
        assert(char_at_confliction != kLeafChar);
        auto relay_next = forked_base xor char_at_confliction;
        
        if (small_prefix) {
            // ||*|*|*||*|*|*|$||
            //  |→   ←| conflict between this area
            if (left_label_length == 0) {
                node_unit.set_base(forked_base);
            } else {
                std::string_view left_label((char*)_impl::tail_.data() + label_index, left_label_length);
                auto relay_pool_index = _impl::_append_internal_label_in_pool(left_label, forked_base);
                node_unit.set_pool_index(relay_pool_index, false);
            }
            node_unit.set_child(char_at_confliction);
            if (_impl::tail_[label_pos + 1] == kLeafChar) { // Length of right-label is 1.
                _impl::_set_new_node(relay_next, node, char_at_confliction, node_is_terminal);
                _impl::_set_new_trans(relay_next, node_base, node_child);
            } else {
                _impl::_set_new_node_by_label(relay_next, node, char_at_confliction, node_is_terminal, label_pos+1-kIndexBytes, false);
                _impl::_set_base_in_pool(label_pos+1-kIndexBytes, node_base);
                _impl::_unit_at(relay_next).set_child(node_child);
            }
        } else {
            // ||*|*|*||*|*|*|$||
            //         |→   ←| conflict between this area
            _impl::tail_[label_pos] = kLeafChar;
            _impl::_set_base_in_pool(pool_index, forked_base);
            node_unit.set_child(char_at_confliction);
            if (right_label_length == 1) {
                _impl::_set_new_node(relay_next, node, char_at_confliction, node_is_terminal);
                _impl::_set_new_trans(relay_next, node_base, node_child);
            } else {
                std::string right_label((char*)_impl::tail_.data() + label_pos + 1);
                auto relay_pool_index = _impl::_append_internal_label_in_pool(right_label, node_base);
                _impl::_set_new_node_by_label(relay_next, node, char_at_confliction, node_is_terminal, relay_pool_index, false);
                _impl::_unit_at(relay_next).set_child(node_child);
            }
        }
        _impl::_consume_block(_impl::_block_index_of(relay_next), 1);
        _base::_update_check(node_base, node_child, relay_next);
    }
    
};


// MARK: - Dynamic Double-array Interface

template <typename IndexType, bool LegacyBuild = true, bool Patricia = true>
class DoubleArray;


template <typename IndexType, bool LegacyBuild>
class DoubleArray<IndexType, LegacyBuild, false> :
private _DynamicDoubleArrayMpTrieBehavior<_DoubleArrayBcMpTrieImpl<IndexType, not LegacyBuild>> {
    using _impl = _DoubleArrayBcMpTrieImpl<IndexType, not LegacyBuild>;
    using _behavior = _DynamicDoubleArrayMpTrieBehavior<_impl>;
    using _self = DoubleArray<IndexType, LegacyBuild, false>;
public:
    using index_type = typename _impl::_index_type;
    using char_type = typename _impl::_char_type;
    
    static constexpr index_type kRootIndex = _impl::kRootIndex;
    static constexpr char_type kLeafChar = graph_util::kLeafChar;
    
    static constexpr size_t kBlockSize = _impl::kBlockSize;
    
    DoubleArray() : _behavior() {}
    
    DoubleArray(std::ifstream& ifs) : _behavior(ifs) {}
    
    template <typename StrIter,
    typename Traits = std::iterator_traits<StrIter>>
    DoubleArray(StrIter begin, StrIter end) {
        _arrange_keysets(begin, end, 0, kRootIndex);
    }
    
    DoubleArray(const std::vector<std::string>& key_set) : DoubleArray(key_set.begin(), key_set.end()) {}
    
    ~DoubleArray() = default;
    
    DoubleArray& rebuild() {
        DoubleArray new_da;
        new_da._arrange_da(*this, kRootIndex, kRootIndex);
        *this = new_da;
        return *this;
    }
    
    size_t size_in_bytes() const {return _impl::_size_in_bytes();}
    
    size_t bc_size_in_bytes() const {return size_vec(_impl::container_);}
    
    size_t pool_size_in_bytes() const {return size_vec(_impl::tail_);}
    
    size_t succinct_size_in_bytes() const {return size_in_bytes() - bc_size_in_bytes() - pool_size_in_bytes();}
    
    size_t bc_blank_size_in_bytes() const {return _impl::kUnitBytes * (_impl::_bc_size() - num_nodes());}
    
    size_t pool_blank_size_in_bytes() const {
        size_t blank_size = _impl::tail_.size();
        for (size_t i = 0; i < _impl::_bc_size(); i++) {
            auto unit = _impl::_unit_at(i);
            if (_impl::_empty(i) or not unit.has_label())
                continue;
            blank_size -= _impl::_suffix_in_tail(unit.pool_index()).size()+1;
        }
        return blank_size;
    }
    
    size_t blank_size_in_bytes() const {return bc_blank_size_in_bytes() + pool_blank_size_in_bytes();}
    
    size_t num_nodes() const {
        size_t cnt = 0;
        for (size_t i = 0; i < _impl::_block_size(); i++) {
            cnt += 256 - _impl::_block_at(i).num_empties();
        }
        return cnt;
    }
    
    bool accept(std::string_view key) const {
        return _traverse(key, [](auto){}, [](auto, auto){}, [](auto, auto, auto){});
    }
    
    bool insert(std::string_view key) {
        return not _traverse(key, [](auto){},
                             [&](index_type node, size_t key_pos) {
                                 _behavior::_insert_in_bc(node, key_pos < key.size() ? key.substr(key_pos) : "");
                             }, [&](index_type node, size_t tail_pos, size_t key_pos) {
                                 _behavior::_insert_in_tail(node, tail_pos, key_pos < key.size() ? key.substr(key_pos) : "");
                             });
    }
    
    bool erase(std::string_view key) {
        return _traverse(key,
                         [&](index_type node) {
                             _behavior::_delete_leaf(node);
                         },
                         [](auto, auto){}, [](auto, auto, auto){});
    }
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex] \tterminal, \tcheck, \tsibling, \thas label, \tbase, \tchild"  << std::endl;
        for (size_t i = 0; i < std::min(_impl::_bc_size(), (size_t)0x10000); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            std::cout << "\t\t"<<i<<"] \t";
            auto empty =_impl::_empty(i);
            if (not empty) {
                auto unit = _impl::_unit_at(i);
                std::cout<<unit.terminal()<<", \t"<<unit.check()<<", \t"<<unit.sibling()<<", \t"<<unit.has_label();
                if (unit.has_label())
                    std::cout<<", \t"<<unit.pool_index();
                if (not _impl::_is_edge_at(i))
                    std::cout<<", \t"<<_impl::_base_at(i)<<", \t"<<unit.child();
            }
            std::cout << std::endl;
        }
    }
    
private:
    template <class SuccessAction, class FailedInBcAction, class FailedInTailAction>
    bool _traverse(std::string_view key, SuccessAction success, FailedInBcAction failed_in_bc, FailedInTailAction failed_in_tail) const {
        index_type node = kRootIndex;
        size_t key_pos = 0;
        for (; key_pos < key.size(); key_pos++) {
            if (not _transition(node, key[key_pos])) {
                failed_in_bc(node, key_pos);
                return false;
            }
            if (_impl::_unit_at(node).has_label() and
                not _transition_in_tail(node, key, ++key_pos, failed_in_tail)) {
                return false;
            }
        }
        auto leached_unit = _impl::_unit_at(node);
        if (not leached_unit.terminal()) {
            failed_in_bc(node, key_pos);
            return false;
        }
        success(node);
        return true;
    }
    
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
    
    template <class FailedInTailAction>
    bool _transition_in_tail(index_type node, std::string_view key, size_t& key_pos, FailedInTailAction failed_in_tail) const {
        assert(_impl::_unit_at(node).has_label());
        auto tail_index = _impl::_unit_at(node).pool_index();
        size_t i = 0;
        assert(_impl::tail_[tail_index] != kLeafChar);
        while (key_pos < key.size()) {
            char_type char_in_tail = _impl::tail_[tail_index+i];
            if (char_in_tail == kLeafChar or
                char_in_tail != (char_type)key[key_pos]) {
                failed_in_tail(node, tail_index+i, key_pos);
                return false;
            }
            key_pos++;
            i++;
        }
        if (_impl::tail_[tail_index+i] != kLeafChar) {
            failed_in_tail(node, tail_index+i, key_pos);
            return false;
        }
        key_pos--;
        return true;
    }
    
    void _arrange_da(const _self& da, index_type node, index_type co_node) {
        std::vector<typename _impl::_internal_label_container> label_datas;
        std::vector<char_type> children;
        auto base = _impl::_base_at(node);
        da._behavior::_for_each_children(node, [&](char_type child) {
            children.push_back(child);
            auto index = base xor child;
            auto unit = _impl::_unit_at(index);
            auto num_children = _impl::_num_of_children(index);
            std::string label = std::string(child);
            if (unit.has_label())
                _impl::_label_in_pool(index);
            while (not unit.terminal() and num_children == 1) {
                label += unit.child;
                _transition(index, unit.child);
                unit = _impl::_unit_at(index);
                if (unit.has_label())
                    label += _impl::_suffix_in_tail(unit.pool_index());
                num_children = _impl::_num_of_children(index);
            }
            label_datas.push_back({label,
                unit.terminal(),
                unit.label_is_suffix()});
        });
        
        auto new_base = _impl::_find_base(children);
        _impl::_insert_nodes(co_node, label_datas, new_base);
        for (auto label_data : label_datas) {
            auto target = node;
            auto c = label_data.label.front();
            da._transition(target, c);
            if (not label_data.suffix)
                _arrange_da(da, target, new_base xor c);
        }
    }
    
    template <typename StrIter,
    typename Traits = std::iterator_traits<StrIter>>
    void _arrange_keysets(StrIter begin, StrIter end, size_t depth, index_type co_node) {
        while (begin < end and ((*begin).size() == depth))
            ++begin;
        if (begin >= end)
            return;
        
        std::vector<typename _impl::_internal_label_container> label_datas;
        std::vector<char_type> children;
        std::vector<StrIter> iters = {begin};
        std::string_view front_label((*begin).data() + depth);
        char_type prev_key = (*begin)[depth];
        auto append = [&](auto it) {
            assert(not front_label.empty());
            if (it == iters.back()) {
                label_datas.push_back({front_label, true, true});
            } else {
                label_datas.push_back({front_label.substr(0,1), front_label.size() == 1, false});
            }
            children.push_back(prev_key);
            iters.push_back(it+1);
        };
        for (auto it = begin+1; it != end; ++it) {
            char_type c = (*it)[depth];
            if (c != prev_key) {
                append(it-1);
                front_label = std::string_view((*it).data() + depth);
                prev_key = c;
            }
        }
        append(end-1);
        
        auto new_base = _impl::_find_base(children);
        _impl::_insert_nodes(co_node, label_datas, new_base);
        for (size_t i = 0; i < label_datas.size(); i++) {
            auto label = label_datas[i].label;
            if (not label_datas[i].suffix)
                _arrange_keysets(iters[i], iters[i+1], depth+label.size(), new_base xor (char_type)label.front());
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
    using index_type = typename _behavior::_index_type;
    using char_type = typename _behavior::_char_type;
    
    static constexpr index_type kRootIndex = _impl::kRootIndex;
    static constexpr char_type kLeafChar = '\0';
    
    static constexpr size_t kBlockSize = _behavior::kBlockSize;
    
    DoubleArray() : _behavior() {}
    
    DoubleArray(std::ifstream& ifs) : _behavior(ifs) {}
    
    template <typename StrIter,
    typename Traits = std::iterator_traits<StrIter>>
    DoubleArray(StrIter begin, StrIter end) {
        _arrange_keysets(begin, end, 0, kRootIndex);
    }
    
    DoubleArray(const std::vector<std::string>& key_set) : DoubleArray(key_set.begin(), key_set.end()) {}
    
    ~DoubleArray() = default;
    
    DoubleArray& rebuild() {
        DoubleArray new_da;
        new_da._arrange_da(*this, kRootIndex, kRootIndex);
        *this = new_da;
        return *this;
    }
    
    size_t size_in_bytes() const {return _impl::_size_in_bytes();}
    
    size_t bc_size_in_bytes() const {return size_vec(_impl::container_);}
    
    size_t pool_size_in_bytes() const {return size_vec(_impl::tail_);}
    
    size_t succinct_size_in_bytes() const {return size_in_bytes() - bc_size_in_bytes() - pool_size_in_bytes();}
    
    size_t bc_blank_size_in_bytes() const {return _impl::kUnitBytes * (_impl::_bc_size() - num_nodes());}
    
    size_t pool_blank_size_in_bytes() const {
        size_t blank_size = _impl::tail_.size();
        for (size_t i = 0; i < _impl::_bc_size(); i++) {
            auto unit = _impl::_unit_at(i);
            if (_impl::_empty(i) or not unit.has_label())
                continue;
            blank_size -= ((unit.label_is_suffix() ? 0 : _impl::kIndexBytes) +
                           _impl::_label_in_pool(i).size()+1);
        }
        return blank_size;
    }
    
    size_t blank_size_in_bytes() const {return bc_blank_size_in_bytes() + pool_blank_size_in_bytes();}
    
    size_t num_nodes() const {
        size_t cnt = 0;
        for (size_t i = 0; i < _impl::_block_size(); i++) {
            cnt += 256 - _impl::_block_at(i).num_empties();
        }
        return cnt;
    }
    
    void serialize(std::ifstream& ifs) const {
        _impl::_serialize(ifs);
    }
    
    bool accept(std::string_view key) const {
        return _traverse(key, [](auto){}, [](auto, auto){}, [](auto, auto, auto){});
    }
    
    bool insert(std::string_view key) {
        return not _traverse(key, [](auto){},
                             [&](index_type node, size_t key_pos) {
                                 _behavior::_insert_in_bc(node, key_pos < key.size() ? key.substr(key_pos) : "");
                             },
                             [&](index_type node, size_t label_pos, size_t key_pos) {
                                 _behavior::_insert_in_pool(node, label_pos, key_pos < key.size() ? key.substr(key_pos) : "");
                             });
    }
    
    bool erase(std::string_view key) {
        return _traverse(key,
                         [&](index_type node) {
                             _behavior::_delete_leaf(node);
                         }, [](auto, auto){}, [](auto, auto, auto){});
    }
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex] \texists, \tcheck, \tsibling, \tbase, \tchild"  << std::endl;
        for (size_t i = 0; i < std::min(_impl::_bc_size(), (size_t)0x10000); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            std::cout << "\t\t"<<i<<"] \t";
            auto empty =_impl::_empty(i);
            if (not empty) {
                auto unit = _impl::_unit_at(i);
                std::cout<<unit.terminal()<<", \t"<<unit.check()<<", \t"<<unit.sibling()<<", \t"<<unit.has_label();
                if (unit.has_label())
                    std::cout<<", \t"<<unit.pool_index();
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
                failed_in_bc(node, key_pos);
                return false;
            }
            if (_impl::_unit_at(node).has_label() and
                not _transition_with_label(node, key, ++key_pos, failed_in_pool))
                return false;
        }
        if (not _impl::_unit_at(node).terminal()) {
            failed_in_bc(node, key.size());
            return false;
        }
        success(node);
        return true;
    }
    
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
    
    template <class FailedAction>
    bool _transition_with_label(index_type node, std::string_view key, size_t& key_pos, FailedAction failed) const {
        assert(_impl::_unit_at(node).has_label());
        auto label_index = _impl::_label_index_at(node);
        assert(_impl::tail_[label_index] != kLeafChar);
        size_t i = 0;
        while (key_pos < key.size()) {
            char_type char_in_label = _impl::tail_[label_index+i];
            if (char_in_label == kLeafChar) {
                --key_pos;
                return true;
            }
            if (char_in_label != (char_type)key[key_pos]) {
                failed(node, label_index+i, key_pos);
                return false;
            }
            i++;
            key_pos++;
        }
        if (_impl::tail_[label_index+i] != kLeafChar) {
            failed(node, label_index+i, key_pos);
            return false;
        }
        --key_pos;
        return true;
    }
    
    void _arrange_da(const _self& da, index_type node, index_type co_node) {
        std::vector<typename _impl::_internal_label_container> label_datas;
        std::vector<char_type> children;
        auto base = _impl::_base_at(node);
        da._behavior::_for_each_children(node, [&](char_type child) {
            children.push_back(child);
            auto index = base xor child;
            auto unit = _impl::_unit_at(index);
            auto num_children = _impl::_num_of_children(index);
            std::string label = std::string(child);
            if (unit.has_label())
                _impl::_label_in_pool(index);
            while (not unit.terminal() and num_children == 1) {
                label += unit.child;
                _transition(index, unit.child);
                unit = _impl::_unit_at(index);
                if (unit.has_label())
                    label += _impl::_label_in_pool(index);
                num_children = _impl::_num_of_children(index);
            }
            label_datas.push_back({label,
                                   unit.terminal(),
                                   unit.label_is_suffix()});
        });
        
        auto new_base = _impl::_find_base(children);
        _impl::_insert_nodes(co_node, label_datas, new_base);
        for (auto label_data : label_datas) {
            auto target = node;
            auto c = label_data.label.front();
            da._transition(target, c);
            if (not label_data.suffix)
                _arrange_da(da, target, new_base xor c);
        }
    }
    
    template <typename StrIter,
    typename Traits = std::iterator_traits<StrIter>>
    void _arrange_keysets(StrIter begin, StrIter end, size_t depth, index_type co_node) {
        while (begin < end and ((*begin).size() == depth))
            ++begin;
        if (begin >= end)
            return;
        
        std::vector<typename _impl::_internal_label_container> label_datas;
        std::vector<char_type> children;
        std::vector<StrIter> iters = {begin};
        std::string_view front_label((*begin).data() + depth);
        char_type prev_key = (*begin).size() <= depth ? kLeafChar : (*begin)[depth];
        auto append = [&](auto it) {
            size_t label_length = 1;
            std::string_view back_label((*it).data() + depth);
            while (label_length < front_label.size() and label_length < back_label.size() and
                   back_label[label_length] == front_label[label_length])
                label_length++;
            label_datas.push_back({front_label.substr(0, label_length),
                                   label_length == front_label.size(),
                                   label_length == back_label.size()});
            children.push_back(prev_key);
            iters.push_back(it+1);
        };
        for (auto it = begin+1; it != end; ++it) {
            char_type c = (*it)[depth];
            if (c != prev_key) {
                append(it-1);
                front_label = std::string_view((*it).data() + depth);
                prev_key = c;
            }
        }
        append(end-1);
        
        auto new_base = _impl::_find_base(children);
        _impl::_insert_nodes(co_node, label_datas, new_base);
        for (size_t i = 0; i < label_datas.size(); i++) {
            auto label = label_datas[i].label;
            if (not label_datas[i].suffix)
                _arrange_keysets(iters[i], iters[i+1], depth+label.size(), new_base xor (char_type)label.front());
        }
    }
    
    
    using u32DoubleArray = DoubleArray<uint32_t>;
    using u64DoubleArray = DoubleArray<uint64_t>;
    
};

} // namespace sim_ds

#endif /* DoubleArray_hpp */

