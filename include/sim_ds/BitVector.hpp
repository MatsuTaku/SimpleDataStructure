//
//  VitVector.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/10/30.
//

#ifndef VitVector_hpp
#define VitVector_hpp

#include "basic.hpp"
#include "log.hpp"
#include "bit_util.hpp"
#include "FitVector.hpp"
#include "MultipleVector.hpp"
#include "calc.hpp"

namespace sim_ds {


template <class BitSequence>
class BitReference {
    using storage_type = typename BitSequence::storage_type;
    using storage_pointer = typename BitSequence::storage_pointer;
    
    friend typename BitSequence::Self;
    
public:
    constexpr operator bool() const {
        return static_cast<bool>(*pointer_ & mask_);
    }
    
    constexpr BitReference& operator=(bool x) {
        if (x)
            *pointer_ |= mask_;
        else
            *pointer_ &= ~mask_;
        return *this;
    }
    
private:
    constexpr BitReference(storage_pointer p, storage_type m) noexcept : pointer_(p), mask_(m) {}
    
    storage_pointer pointer_;
    storage_type mask_;
    
};


template <class BitSequence>
class BitConstReference {
    using storage_type = typename BitSequence::storage_type;
    using storage_pointer = typename BitSequence::const_storage_pointer;
    
    friend typename BitSequence::Self;
    
public:
    constexpr operator bool() const {
        return static_cast<bool>(*pointer_ & mask_);
    }
    
private:
    constexpr BitConstReference(storage_pointer p, storage_type m) noexcept : pointer_(p), mask_(m) {}
    
    storage_pointer pointer_;
    storage_type mask_;
    
};


class BitVector {
public:
    using Self = BitVector;
    using storage_type = id_type;
    using storage_pointer = storage_type*;
    using const_storage_pointer = const storage_type*;
    
    static constexpr uint8_t kBitsPerWord = sizeof(id_type) * 8; // 64
    
    friend class BitReference<BitVector>;
    friend class BitConstReference<BitVector>;
    
    using Reference = BitReference<BitVector>;
    using ConstReference = BitConstReference<BitVector>;
    
private:
    std::vector<storage_type> base_;
    size_t size_ = 0;
    
    constexpr size_t abs_(size_t index) const {
        return index / kBitsPerWord;
    }
    
    constexpr size_t rel_(size_t index) const {
        return index % kBitsPerWord;
    }
    
public:
    BitVector() = default;
    
    template <typename BitSet>
    explicit BitVector(const BitSet& bits) {
        resize(bits.size());
        for (auto i = 0; i < bits.size(); i++)
            operator[](i) = bits[i];
    }
    
    Reference operator[](size_t index) {
        assert(index < size());
        return Reference(&base_[index/kBitsPerWord], bit_util::OffsetMask(index%kBitsPerWord));
    }
    
    ConstReference operator[](size_t index) const {
        assert(index < size());
        return ConstReference(&(base_[index/kBitsPerWord]), bit_util::OffsetMask(index%kBitsPerWord));
    }
    
    Reference front() {
        return operator[](0);
    }
    
    ConstReference front() const {
        return operator[](0);
    }
    
    Reference back() {
        return operator[](size() - 1);
    }
    
    ConstReference back() const {
        return operator[](size() - 1);
    }
    
    void resize(size_t size) {
        base_.resize(std::ceil(double(size) / kBitsPerWord));
        size_ = size;
    }
    
    void reserve(size_t size) {
        base_.reserve(std::ceil(double(size) / kBitsPerWord));
    }
    
    void shrink_to_fit() {
        base_.shrink_to_fit();
    }
    
    size_t size() const {
        return size_;
    }
    
    bool empty() const {
        return size() == 0;
    }
    
    void push_back(bool bit) {
        resize(size() + 1);
        back() = bit;
    }
    
    auto data() const {
        return base_.data();
    }
    
    size_t size_in_bytes() const {
        auto size = sizeof(size_t);
        size +=  size_vec(base_);
        return size;
    }
    
    void Read(std::istream& is) {
        size_ = read_val<size_t>(is);
        base_ = read_vec<storage_type>(is);
    }
    
    void Write(std::ostream& os) const {
        write_val(size_, os);
        write_vec(base_, os);
    }
    
};


} // namespace sim_ds

#endif /* VitVector_hpp */
