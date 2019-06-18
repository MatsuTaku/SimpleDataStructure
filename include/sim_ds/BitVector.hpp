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
    using _word_type = uint64_t;
protected:
    using _self = _BitVector;
    using _word_pointer = _word_type*;
    using _const_word_pointer = const _word_type*;
    static constexpr uint8_t kBitsPerWord = 64;
    static constexpr unsigned kBitsPerBlock = 256;
    
    friend class BitReference<_BitVector>;
    friend class BitConstReference<_BitVector>;
    friend class BitIterator<_BitVector, false>;
    friend class BitIterator<_BitVector, true>;
    
    using _word_container = aligned_vector<_word_type, 32>;
    
    size_type actual_size_;
    _word_container storage_;
    
    using _reference = BitReference<_BitVector>;
    using _const_reference = BitConstReference<_BitVector>;
    using _iterator = BitIterator<_BitVector, false>;
    using _const_iterator = BitIterator<_BitVector, true>;
    using _pointer = _iterator;
    using _const_pointer = _const_iterator;
    
    _BitVector() : actual_size_(0), storage_{0} {}
    
    explicit _BitVector(size_t size) : actual_size_(size), storage_(get_word_size(size)) {}
    
    explicit _BitVector(size_t size, bool initial_bit) : actual_size_(size), storage_(get_word_size(size)) {
        _fill(_make_iter(0), _make_iter(size), initial_bit);
    }
    
    size_t get_word_size(size_t bit_size) const {
        return (bit_size == 0 ? 1 :
                (bit_size-1) / kBitsPerWord + 1);
    }
    
    _reference _make_ref(size_t pos) {
        return _reference(storage_.data() + pos / kBitsPerWord, bit_util::OffsetMask(pos % kBitsPerWord));
    }
    
    _const_reference _make_ref(size_t pos) const {
        return _const_reference(storage_.data() + pos / kBitsPerWord, bit_util::OffsetMask(pos % kBitsPerWord));
    }
    
    _iterator _make_iter(size_t pos) {
        return _iterator(storage_.data() + pos / kBitsPerWord, pos % kBitsPerWord);
    }
    
    _const_iterator _make_iter(size_t pos) const {
        return _const_iterator(storage_.data() + pos / kBitsPerWord, pos % kBitsPerWord);
    }
    
    size_type _size() const {return actual_size_;}
    
    void _fill(_iterator begin, _iterator end, bool bit) {
        if (end <= begin)
            return;
        const _word_type mask_fill = _word_type(-1);
        _iterator back = end-1;
        auto eclz = kBitsPerWord - (back.ctz_ + 1);
        if (bit) { // Fill by 1
            if (begin.seg_ < back.seg_) {
                *begin.seg_ |= mask_fill << begin.ctz_;
                auto seg = begin.seg_ + 1;
                while (seg < back.seg_) {
                    *(seg++) = mask_fill;
                }
                *back.seg_ |= compl (mask_fill << end.ctz_);
            } else {
                *begin.seg_ |= ((mask_fill << begin.ctz_) bitand
                                (mask_fill >> eclz));
            }
        } else { // Fill by 0
            if (begin.seg_ < back.seg_) {
                *begin.seg_ &= compl (mask_fill << begin.ctz_);
                auto seg = begin.seg_ + 1;
                while (seg < back.seg_) {
                    *(seg++) = _word_type(0);
                }
                *back.seg_ &= mask_fill << end.ctz_;
            } else {
                *begin.seg_ &= compl ((mask_fill << begin.ctz_) bitand
                                      (mask_fill >> eclz));
            }
        }
    }
    
    void _resize(size_t new_size, bool bit) {
        if (new_size < actual_size_) {
            storage_.resize(get_word_size(new_size));
            actual_size_ = new_size;
        } else if (new_size > actual_size_) {
            auto prev_size = actual_size_;
            storage_.resize(get_word_size(new_size));
            _fill(_make_iter(prev_size), _make_iter(new_size), bit);
            actual_size_ = new_size;
        }
    }
    
    void _assign(size_t new_size, bool bit) {
        storage_.resize(get_word_size(new_size));
        actual_size_ = new_size;
        _fill(_make_iter(0), _make_iter(new_size), bit);
    }
    
    void _reserve(size_t reserved_size) {
        storage_.reserve(get_word_size(reserved_size));
    }
    
};


class BitVector : private _BitVector {
private:
    using _base = _BitVector;
public:
    using value_type = bool;
    
    using reference = _base::_reference;
    using const_reference = _base::_const_reference;
    using size_type = _base::size_type;
    using difference_type = _base::difference_type;
    using iterator = _base::_iterator;
    using const_iterator = _base::_const_iterator;
    using pointer = _base::_pointer;
    using const_pointer = _base::_const_pointer;
    
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
        resize(size() + 1, bit);
    }
    
    void fill(iterator begin, iterator end, bool bit) {
        _fill(begin, end, bit);
    }
    
    void resize(size_t new_size) {
        _resize(new_size, 0);
    }
    
    void resize(size_t new_size, bool bit) {
        _resize(new_size, bit);
    }
    
    void assign(size_t new_size, bool bit) {
        _assign(new_size, bit);
    }
    
    void reserve(size_t reserved_size) {
        _reserve(reserved_size);
    }
    
    void shrink_to_fit() {
        _base::storage_.shrink_to_fit();
    }
    
    size_t size() const {return _base::_size();}
    
    bool empty() const {return size() == 0;}
    
    // Pointer Must be aligned as 32 bytes.
    _word_pointer data() {return _base::storage_.data();}
    
    // Pointer Must be aligned as 32 bytes.
    _const_word_pointer data() const {return _base::storage_.data();}
    
    size_t size_in_bytes() const {
        auto size = sizeof(size_t);
        size +=  size_vec(_base::storage_);
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
