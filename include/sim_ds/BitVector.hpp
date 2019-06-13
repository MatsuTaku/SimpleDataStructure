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
    using word_type = id_type;
    using word_pointer = word_type*;
    using const_word_pointer = const word_type*;
    
    static constexpr uint8_t kBitsPerWord = 64;
    
    using reference = BitReference<BitVector>;
    using const_reference = BitConstReference<BitVector>;
    using iterator = BitIterator<BitVector, false>;
    using const_iterator = BitIterator<BitVector, true>;
    
    using value_type = bool;
    using difference_type = long long;
    using pointer = iterator;
    
    using storage_type = aligned_vector<word_type>;
    
private:
    storage_type base_;
    size_t size_;
    
public:
    BitVector() : base_(0), size_(0) {}
    
    BitVector(const BitVector&) = default;
    BitVector& operator=(const BitVector&) = default;
    
    BitVector(BitVector&&) = default;
    BitVector& operator=(BitVector&&) = default;
    
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
    
    reference operator[](size_t index) {return make_ref(index);}
    
    const_reference operator[](size_t index) const {return make_ref(index);}
    
    reference at(size_t index) {
        if (index >= size())
            throw std::out_of_range("Index out of range");
        
        return operator[](index);
    }
    
    const_reference at(size_t index) const {
        if (index >= size())
            throw std::out_of_range("Index out of range");
        
        return operator[](index);
    }
    
    iterator begin() {return make_iter(0);}
    
    const_iterator begin() const {return make_iter(0);}
    
    iterator end() {return make_iter(size());}
    
    const_iterator end() const {return make_iter(size());}
    
    reference front() {return operator[](0);}
    
    const_reference front() const {return operator[](0);}
    
    reference back() {return operator[](size() - 1);}
    
    const_reference back() const {return operator[](size() - 1);}
    
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
        if (new_size > prev_size and bit) {
            uint64_t mask = -1ull;
            base_[prev_size/kBitsPerWord] |= mask << (prev_size%kBitsPerWord);
            for (size_t i = prev_size/kBitsPerWord+1; i <= (new_size-1)/kBitsPerWord; i++)
                base_[i] = mask;
            base_[(new_size-1)/kBitsPerWord] &= mask >> (kBitsPerWord - (new_size-1)%kBitsPerWord - 1);
        }
    }
    
    void reserve(size_t reserved_size) {
        base_.reserve(reserved_size == 0 ? 0 : (reserved_size-1) / kBitsPerWord + 1);
    }
    
    void assign(size_t new_size, bool bit) {
        resize(new_size);
        uint64_t mask = !bit ? 0ull : -1ull;
        for (size_t i = 0; i < new_size/kBitsPerWord; i++)
            base_[i] = mask;
        base_[new_size/kBitsPerWord] = mask >> (kBitsPerWord - (new_size-1)%kBitsPerWord - 1);
    }
    
    void shrink_to_fit() {
        base_.shrink_to_fit();
    }
    
    size_t size() const {return size_;}
    
    bool empty() const {return size() == 0;}
    
    const word_type* data() const {return base_.data();}
    
    size_t size_in_bytes() const {
        auto size = sizeof(size_t);
        size +=  size_vec(base_);
        return size;
    }
    
    void Read(std::istream& is) {
        size_ = read_val<size_t>(is);
        read_vec(is, base_);
    }
    
    void Write(std::ostream& os) const {
        write_val(size_, os);
        write_vec(base_, os);
    }
    
private:
    reference make_ref(size_t pos) {
        assert(pos < size());
        return reference(base_.data() + pos / kBitsPerWord, bit_util::OffsetMask(pos % kBitsPerWord));
    }
    
    const_reference make_ref(size_t pos) const {
        assert(pos < size());
        return const_reference(base_.data() + pos / kBitsPerWord, bit_util::OffsetMask(pos % kBitsPerWord));
    }
    
    iterator make_iter(size_t pos) {
        return iterator(base_.data() + pos / kBitsPerWord, pos % kBitsPerWord);
    }
    
    const_iterator make_iter(size_t pos) const {
        return const_iterator(base_.data() + pos / kBitsPerWord, pos % kBitsPerWord);
    }
    
};


} // namespace sim_ds

#endif /* VitVector_hpp */
