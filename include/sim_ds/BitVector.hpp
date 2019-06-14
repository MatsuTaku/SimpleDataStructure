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


class _BitVector {
public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using value_type = bool;
    using word_type = uint64_t;
protected:
    using _self = _BitVector;
    using word_pointer = word_type*;
    using const_word_pointer = const word_type*;
    static constexpr uint8_t kBitsPerWord = 64;
    static constexpr unsigned kBitsPerBlock = 256;
    
    friend class BitReference<_BitVector>;
    friend class BitConstReference<_BitVector>;
    friend class BitIterator<_BitVector, false>;
    friend class BitIterator<_BitVector, true>;
    
    using storage_type = aligned_vector<word_type, 32>;
    
    storage_type storage_;
    size_type actual_size_;
    
    using reference = BitReference<_BitVector>;
    using const_reference = BitConstReference<_BitVector>;
    using iterator = BitIterator<_BitVector, false>;
    using const_iterator = BitIterator<_BitVector, true>;
    using pointer = iterator;
    using const_pointer = const_iterator;
    
    _BitVector() : storage_{0}, actual_size_(0) {}
    
    explicit _BitVector(size_t size) : storage_(get_word_size(size)), actual_size_(size) {}
    
    explicit _BitVector(size_t size, bool initial_bit) : storage_(get_word_size(size)), actual_size_(size) {
        _fill(_make_iter(0), _make_iter(size), initial_bit);
    }
    
    size_t get_word_size(size_t bit_size) const {return bit_size == 0 ? 1 : (bit_size-1) / kBitsPerWord + 1;}
    
    reference _make_ref(size_t pos) {
        return reference(reinterpret_cast<word_pointer>(storage_.data()) + pos / kBitsPerWord, bit_util::OffsetMask(pos % kBitsPerWord));
    }
    
    const_reference _make_ref(size_t pos) const {
        return const_reference(reinterpret_cast<const_word_pointer>(storage_.data()) + pos / kBitsPerWord, bit_util::OffsetMask(pos % kBitsPerWord));
    }
    
    iterator _make_iter(size_t pos) {
        return iterator(reinterpret_cast<word_pointer>(storage_.data()) + pos / kBitsPerWord, pos % kBitsPerWord);
    }
    
    const_iterator _make_iter(size_t pos) const {
        return const_iterator(reinterpret_cast<const_word_pointer>(storage_.data()) + pos / kBitsPerWord, pos % kBitsPerWord);
    }
    
    void _fill(iterator begin, iterator end, bool bit) {
        if (end <= begin)
            return;
        const word_type mask_fill = word_type(-1);
        if (bit) {
            auto back = end-1;
            auto ectz = back.ctz_ + 1;
            if (begin.seg_ < back.seg_) {
                *begin.seg_ |= mask_fill << begin.ctz_;
                auto seg = begin.seg_ + 1;
                while (seg < back.seg_)
                    *(seg++) = mask_fill;
                *back.seg_ |= mask_fill >> (kBitsPerWord - ectz);
            } else {
                *begin.seg_ |= ((mask_fill << begin.ctz_) bitand
                                (mask_fill >> (kBitsPerWord - ectz)));
            }
        } else {
            auto back = end-1;
            auto bctz = (begin.ctz_ + kBitsPerWord - 1) % kBitsPerWord + 1;
            auto ectz = back.ctz_ + 1;
            if (begin.seg_ < back.seg_) {
                *begin.seg_ &= mask_fill >> (kBitsPerWord - bctz);
                auto seg = begin.seg_ + 1;
                while (seg < back.seg_) 
                    *(seg++) = 0;
                *back.seg_ &= mask_fill << ectz;
            } else {
                *begin.seg_ &= ((mask_fill >> (kBitsPerWord - bctz)) bitor
                                (mask_fill << ectz));
            }
        }
    }
    
    void _reserve(size_t reserved_size) {
        storage_.reserve(get_word_size(reserved_size));
    }
    
    void _resize(size_t new_size, bool bit) {
        if (new_size <= actual_size_) {
            storage_.resize(get_word_size(new_size));
        } else {
            auto prev_size = actual_size_;
            storage_.resize(get_word_size(new_size));
            _fill(_make_iter(prev_size), _make_iter(new_size), bit);
        }
        actual_size_ = new_size;
    }
    
    void _resize(size_t new_size) {
        _resize(new_size, 0);
    }
    
    void _assign(size_t new_size, bool bit) {
        storage_.resize(get_word_size(new_size));
        actual_size_ = new_size;
        _fill(_make_iter(0), _make_iter(new_size), bit);
    }
    
};


class BitVector : private _BitVector {
private:
    using _base = _BitVector;
public:
    using value_type = bool;
    
    using reference = _base::reference;
    using const_reference = _base::const_reference;
    using size_type = _base::size_type;
    using difference_type = _base::difference_type;
    using iterator = _base::iterator;
    using const_iterator = _base::const_iterator;
    using pointer = _base::pointer;
    using const_pointer = _base::const_pointer;
    
public:
    BitVector() : _base() {}
    
    BitVector(size_t size) : _base(size) {}
    
    BitVector(size_t size, bool initial_bit) : _base(size, initial_bit) {}
    
    BitVector(std::initializer_list<bool> bits) : _base(bits.size()) {
        std::transform(bits.begin(), bits.end(), _base::_make_iter(0), [](bool b) {return b;});
    }

    template <typename BitArray>
    BitVector(const BitArray& bits) : _base(bits.size()) {
        std::transform(bits.begin(), bits.end(), _base::_make_iter(0), [](auto b) {return bool(b);});
    }
    
    reference operator[](size_t index) {return _base::_make_ref(index);}
    
    const_reference operator[](size_t index) const {return _base::_make_ref(index);}
    
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
    
    iterator begin() {return _base::_make_iter(0);}
    
    const_iterator begin() const {return _base::_make_iter(0);}
    
    iterator end() {return _base::_make_iter(size());}
    
    const_iterator end() const {return _base::_make_iter(size());}
    
    reference front() {return operator[](0);}
    
    const_reference front() const {return operator[](0);}
    
    reference back() {return operator[](size() - 1);}
    
    const_reference back() const {return operator[](size() - 1);}
    
    void push_back(bool bit) {
        resize(size() + 1);
        back() = bit;
    }
    
    void fill(iterator begin, iterator end, bool bit) {
        _base::_fill(begin, end, bit);
    }
    
    void reserve(size_t reserved_size) {
        _base::_reserve(reserved_size);
    }
    
    void resize(size_t new_size) {
        _base::_resize(new_size);
    }
    
    void resize(size_t new_size, bool bit) {
        _base::_resize(new_size, bit);
    }
    
    void assign(size_t new_size, bool bit) {
        _base::_assign(new_size, bit);
    }
    
    void shrink_to_fit() {
        storage_.shrink_to_fit();
    }
    
    size_t size() const {return _base::actual_size_;}
    
    bool empty() const {return size() == 0;}
    
    const_word_pointer data() const {return reinterpret_cast<const_word_pointer>(storage_.data());}
    
    size_t size_in_bytes() const {
        auto size = sizeof(size_t);
        size +=  size_vec(storage_);
        return size;
    }
    
    void Read(std::istream& is) {
        _base::actual_size_ = read_val<size_t>(is);
        read_vec(is, _base::storage_);
    }
    
    void Write(std::ostream& os) const {
        write_val(_base::actual_size_, os);
        write_vec(_base::storage_, os);
    }
    
};


} // namespace sim_ds

#endif /* VitVector_hpp */
