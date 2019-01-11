//
//  VitVector.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/10/30.
//

#ifndef VitVector_hpp
#define VitVector_hpp

#include "BitReference.hpp"
#include "basic.hpp"
#include "log.hpp"
#include "bit_util.hpp"
#include "FitVector.hpp"
#include "MultipleVector.hpp"
#include "calc.hpp"

namespace sim_ds {


class BitVector {
public:
    using Self = BitVector;
    using storage_type = id_type;
    using storage_pointer = storage_type*;
    using const_storage_pointer = const storage_type*;
    
    static constexpr uint8_t kBitsPerWord = 64;
    
    using Reference = BitReference<BitVector>;
    using ConstReference = BitConstReference<BitVector>;
    using Iterator = BitIterator<BitVector, false>;
    using ConstIterator = BitIterator<BitVector, true>;
    
    using value_type = bool;
    using pointer = Iterator;
    
private:
    std::vector<storage_type> base_;
    size_t size_;
    
public:
    BitVector() : base_(0), size_(0) {}
    
    template <typename BitSet>
    BitVector(const BitSet& bits) {
        resize(bits.size());
        for (auto i = 0; i < bits.size(); i++)
            operator[](i) = static_cast<bool>(bits[i]);
    }
    
    explicit BitVector(size_t size) {
        resize(size);
    }
    
    explicit BitVector(size_t size, bool initial_bit) {
        assign(size, initial_bit);
    }
    
    Reference operator[](size_t index) {return make_ref(index);}
    
    ConstReference operator[](size_t index) const {return make_ref(index);}
    
    Iterator begin() {return make_iter(0);}
    
    ConstIterator begin() const {return make_iter(0);}
    
    Iterator end() {return make_iter(size());}
    
    ConstIterator end() const {return make_iter(size());}
    
    Reference front() {return operator[](0);}
    
    ConstReference front() const {return operator[](0);}
    
    Reference back() {return operator[](size() - 1);}
    
    ConstReference back() const {return operator[](size() - 1);}
    
    void push_back(bool bit) {
        resize(size() + 1);
        back() = bit;
    }
    
    void resize(size_t new_size) {
        base_.resize(new_size == 0 ? 0 : (new_size-1) / kBitsPerWord + 1);
        size_ = new_size;
    }
    
    void resize(size_t new_size, bool bit) {
        auto prev_size = size();
        resize(new_size);
        for (size_t i = prev_size; i < new_size; i++)
                operator[](i) = bit;
    }
    
    void reserve(size_t reserved_size) {
        base_.reserve(reserved_size == 0 ? 0 : (reserved_size-1) / kBitsPerWord + 1);
    }
    
    void assign(size_t size, bool bit) {
        resize(size);
        for (size_t i = 0; i < size; i++)
            operator[](i) = bit;
    }
    
    void shrink_to_fit() {
        base_.shrink_to_fit();
    }
    
    size_t size() const {return size_;}
    
    bool empty() const {return size() == 0;}
    
    const storage_type* data() const {
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
    
private:
    Reference make_ref(size_t pos) {
        assert(pos < size());
        return Reference(base_.data() + pos / kBitsPerWord, bit_util::OffsetMask(pos % kBitsPerWord));
    }
    
    ConstReference make_ref(size_t pos) const {
        assert(pos < size());
        return ConstReference(base_.data() + pos / kBitsPerWord, bit_util::OffsetMask(pos % kBitsPerWord));
    }
    
    Iterator make_iter(size_t pos) {
        return Iterator(base_.data() + pos / kBitsPerWord, pos % kBitsPerWord);
    }
    
    ConstIterator make_iter(size_t pos) const {
        return ConstIterator(base_.data() + pos / kBitsPerWord, pos % kBitsPerWord);
    }
    
};


} // namespace sim_ds

#endif /* VitVector_hpp */
