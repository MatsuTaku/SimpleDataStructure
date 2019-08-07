//
//  DoubleArrayBlock.hpp
//
//  Created by 松本拓真 on 2019/08/06.
//

#ifndef DoubleArrayBlock_hpp
#define DoubleArrayBlock_hpp

namespace sim_ds {


// MARK: - Block reference

template <class _Da>
class _CompactDoubleArrayBlockContainer;

//  Double-array block implementation
//             ---------------------------------------------------------------------------------------------------------------------------------------------------------
//    Enabled  | Base bit field [256bit] | Unit bit field [256bit] | Pred[8Byte] | Succ[8Byte] | Number of empties [4Byte] | Error counts [4Byte] | Empty head [1Byte] |
//             ---------------------------------------------------------------------------------------------------------------------------------------------------------
//             |
//          pointer_
//
template <class _Ctnr>
class _CompactDoubleArrayBlockReferenceCommon {
public:
    using _container = _Ctnr;
    using _block_word_type = typename _container::_block_word_type;
    static_assert(sizeof(_block_word_type) == 8, "Invalid block word type!");
    using _index_type = typename _Ctnr::_index_type;
    using _inset_type = typename _Ctnr::_inset_type;
    
    static constexpr size_t kWordBits = sizeof(_block_word_type)*8;
    static constexpr size_t kBlockSize = _Ctnr::kBlockSize;
    
    static constexpr size_t kPredOffset = 0;
    static constexpr size_t kSuccOffset = kPredOffset + 1;
    static constexpr size_t kNumEmptiesOffset = kSuccOffset + 1;
    static constexpr size_t kErrorCountOffset = kNumEmptiesOffset;
    static constexpr size_t kErrorCountInset = 32;
    static constexpr size_t kEmptyHeadOffset = kNumEmptiesOffset + 1;
    static constexpr size_t kFieldQBytes = kBlockSize / kWordBits; // 4
    static constexpr size_t kBlockQBytes = kFieldQBytes*3;
    
    static constexpr _block_word_type kNumEmptiesMask = 0x1FF;
    
    static constexpr _block_word_type kDisabledFlag = 1ull << sizeof(_inset_type)*8;
    static constexpr _block_word_type kEmptyHeadMask = kDisabledFlag-1;
};


template <class _Ctnr>
class _CompactDoubleArrayBlockConstReference;

template <class _Ctnr>
class _CompactDoubleArrayBlockReference : public _CompactDoubleArrayBlockReferenceCommon<_Ctnr> {
    using _common = _CompactDoubleArrayBlockReferenceCommon<_Ctnr>;
    using typename _common::_container;
    
    using typename _common::_block_word_type;
    using _block_pointer = typename _container::_block_pointer;
    using _const_block_pointer = typename _container::_const_block_pointer;
    using typename _common::_index_type;
    using typename _common::_inset_type;
    
private:
    _block_pointer block_pointer_;
    
    friend typename _container::_self;
    friend class _CompactDoubleArrayBlockConstReference<_Ctnr>;
    
    template <size_t Offset>
    _index_type _index() const {return *reinterpret_cast<const _index_type*>(basic_ptr() + Offset);}
    
    template <size_t Offset>
    _index_type& _index() {return *reinterpret_cast<_index_type*>(basic_ptr() + Offset);}
    
    template <size_t Offset>
    _inset_type _inset() const {return *reinterpret_cast<const _inset_type*>(basic_ptr() + Offset);}
    
    template <size_t Offset>
    _inset_type& _inset() {return *reinterpret_cast<_inset_type*>(basic_ptr() + Offset);}
    
public:
    _CompactDoubleArrayBlockReference& init() {
        bit_util::set256_epi1(1, base_field_ptr());
        bit_util::set256_epi1(1, unit_field_ptr());
        *(basic_ptr()+_common::kNumEmptiesOffset) = _common::kBlockSize;
        error_reset();
        set_empty_head(0);
        return *this;
    }
    
    _block_pointer base_field_ptr() {return block_pointer_;}
    
    _const_block_pointer base_field_ptr() const {return block_pointer_;}
    
    _block_pointer unit_field_ptr() {return block_pointer_ + _common::kFieldQBytes;}
    
    _const_block_pointer unit_field_ptr() const {return block_pointer_ + _common::kFieldQBytes;}
    
    _block_pointer basic_ptr() {return block_pointer_ + _common::kFieldQBytes * 2;}
    
    _const_block_pointer basic_ptr() const {return block_pointer_ + _common::kFieldQBytes * 2;}
    
    _index_type pred() const {return _index<_common::kPredOffset>();}
    
    void set_pred(_index_type pred) {
        _index<_common::kPredOffset>() = pred;
    }
    
    _index_type succ() const {return _index<_common::kSuccOffset>();}
    
    void set_succ(_index_type succ) {
        _index<_common::kSuccOffset>() = succ;
    }
    
    size_t num_empties() const {return *(basic_ptr()+_common::kNumEmptiesOffset) bitand _common::kNumEmptiesMask;}
    
    bool filled() const {return num_empties() == 0;}
    
    void consume(size_t num) {
        assert(num_empties() >= num);
        auto& t = *(basic_ptr()+_common::kNumEmptiesOffset);
        t = (t bitand compl _common::kNumEmptiesMask) | (num_empties() - num);
        assert(num_empties() == bit_util::popcnt256(unit_field_ptr()));
    }
    
    void refill(size_t num) {
        assert(num_empties() + num <= 256);
        auto& t = *(basic_ptr()+_common::kNumEmptiesOffset);
        t = (t bitand compl _common::kNumEmptiesMask) | (num_empties() + num);
        assert(num_empties() == bit_util::popcnt256(unit_field_ptr()));
    }
    
    size_t error_count() const {return (*(basic_ptr()+_common::kErrorCountOffset) >> _common::kErrorCountInset);}
    
    void errored() {
        auto& v = *(basic_ptr()+_common::kErrorCountOffset) ;
        v = ((v bitand bit_util::width_mask<_common::kErrorCountInset>) bitor
             ((_block_word_type)(error_count()+1) << _common::kErrorCountInset));
    }
    
    void error_reset() {
        *(basic_ptr()+_common::kErrorCountOffset) &= bit_util::width_mask<_common::kErrorCountInset>;
    }
    
    bool empty_base_at(size_t index) const {
        return *(base_field_ptr()+(index/_common::kWordBits)) bitand bit_util::OffsetMask(index%_common::kWordBits);
    }
    
    void freeze_base_at(size_t index) {
        assert(empty_base_at(index));
        *(base_field_ptr()+(index/_common::kWordBits)) &= compl bit_util::OffsetMask(index%_common::kWordBits);
    }
    
    void thaw_base_at(size_t index) {
        assert(not empty_base_at(index));
        *(base_field_ptr()+(index/_common::kWordBits)) |= bit_util::OffsetMask(index%_common::kWordBits);
    }
    
    bool empty_element_at(size_t index) const {
        return *(unit_field_ptr()+(index/_common::kWordBits)) bitand bit_util::OffsetMask(index%_common::kWordBits);
    }
    
    void freeze_element_at(size_t index) {
        assert(empty_element_at(index));
        *(unit_field_ptr()+(index/_common::kWordBits)) &= compl bit_util::OffsetMask(index%_common::kWordBits);
    }
    
    void thaw_element_at(size_t index) {
        assert(not empty_element_at(index));
        *(unit_field_ptr()+(index/_common::kWordBits)) |= bit_util::OffsetMask(index%_common::kWordBits);
    }
    
    bool link_enabled() const {return not (*(basic_ptr()+_common::kEmptyHeadOffset) bitand _common::kDisabledFlag);}
    
    void disable_link() {
        *(basic_ptr()+_common::kEmptyHeadOffset) |= _common::kDisabledFlag;
    }
    
    _inset_type empty_head() const {
        assert(link_enabled());
        return _inset<_common::kEmptyHeadOffset>();
    }
    
    void set_empty_head(_inset_type empty_head) {
        assert(empty_element_at(empty_head));
        *(basic_ptr()+_common::kEmptyHeadOffset) = empty_head;
    }
    
private:
    _CompactDoubleArrayBlockReference(_block_pointer pointer) : block_pointer_(pointer) {}
    
};


template <class _Da>
class _CompactDoubleArrayBlockConstReference : public _CompactDoubleArrayBlockReferenceCommon<_Da> {
    using _common = _CompactDoubleArrayBlockReferenceCommon<_Da>;
    using typename _common::_container;
    
    using typename _common::_block_word_type;
    using _block_pointer = typename _container::_const_block_pointer;
    using typename _common::_index_type;
    using typename _common::_inset_type;
    
private:
    _block_pointer block_pointer_;
    
    friend typename _container::_self;
    
    template <size_t Offset>
    _index_type _index() const {return *reinterpret_cast<const _index_type*>(basic_ptr() + Offset);}
    
    template <size_t Offset>
    _inset_type _inset() const {return *reinterpret_cast<const _inset_type*>(basic_ptr() + Offset);}
    
public:
    _CompactDoubleArrayBlockConstReference(const _CompactDoubleArrayBlockReference<_Da> x) : block_pointer_(x.block_pointer_) {}
    
    _block_pointer base_field_ptr() const {return block_pointer_;}
    
    _block_pointer unit_field_ptr() const {return block_pointer_ + _common::kFieldQBytes;}
    
    _block_pointer basic_ptr() const {return block_pointer_ + _common::kFieldQBytes * 2;}
    
    _index_type pred() const {return _index<_common::kPredOffset>();}
    
    _index_type succ() const {return _index<_common::kSuccOffset>();}
    
    size_t num_empties() const {return *(basic_ptr()+_common::kNumEmptiesOffset) & _common::kNumEmptiesMask;}
    
    size_t error_count() const {return (*(basic_ptr()+_common::kErrorCountOffset) >> _common::kErrorCountInset);}
    
    bool filled() const {return num_empties() == 0;}
    
    bool empty_base_at(size_t index) const {
        return *(base_field_ptr()+(index/_common::kWordBits)) bitand bit_util::OffsetMask(index%_common::kWordBits);
    }
    
    bool empty_element_at(size_t index) const {
        return *(unit_field_ptr()+(index/_common::kWordBits)) bitand bit_util::OffsetMask(index%_common::kWordBits);
    }
    
    bool link_enabled() const {return not (*(basic_ptr()+_common::kEmptyHeadOffset) bitand _common::kDisabledFlag);}
    
    _inset_type empty_head() const {return _inset<_common::kEmptyHeadOffset>();}
    
private:
    _CompactDoubleArrayBlockConstReference(_block_pointer pointer) : block_pointer_(pointer) {}
    
};


template <class _Da>
class _CompactDoubleArrayBlockContainer {
public:
    using _self = _CompactDoubleArrayBlockContainer<_Da>;
    using _block_word_type = uint64_t;
    using _block_pointer = _block_word_type*;
    using _const_block_pointer = const _block_word_type*;
    
    using _block_reference = _CompactDoubleArrayBlockReference<_self>;
    using _const_block_reference = _CompactDoubleArrayBlockConstReference<_self>;
    
    using _index_type = typename _Da::_index_type;
    using _inset_type = typename _Da::_inset_type;
    static constexpr size_t kBlockSize = _Da::kBlockSize;
    
    static constexpr auto kBlockQBytes = _block_reference::kBlockQBytes;
    
private:
    aligned_vector<_block_word_type, 32> container_;
    
public:
    _block_reference operator[](size_t index) {
        return _block_reference(container_.data() + (kBlockQBytes * index));
    }
    
    _const_block_reference operator[](size_t index) const {
        return _const_block_reference(container_.data() + (kBlockQBytes * index));
    }
    
    size_t size() const {return container_.size() / kBlockQBytes;}
    
    void resize(size_t new_size) {
        container_.resize(kBlockQBytes * new_size);
    }
    
    size_t size_in_bytes() const {return size_vec(container_);}
    
    void write(std::ostream& os) const {
        write_vec(container_, os);
    }
    
    void read(std::istream& is) {
        read_vec(is, container_);
    }
    
};

}

#endif /* DoubleArrayBlock_hpp */
