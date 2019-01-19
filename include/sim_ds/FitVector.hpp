
//
//  Vector.hpp
//  build
//
//  Created by 松本拓真 on 2018/05/04.
//

#ifndef Vector_hpp
#define Vector_hpp

#include "BitsReference.hpp"
#include "basic.hpp"
#include "bit_util.hpp"
#include "calc.hpp"
#include "log.hpp"

namespace sim_ds {


/*
 * Vector that fit to binary size of max-value of source vector integers.
 */
class FitVector {
public:
    using Self = FitVector;
    using storage_type = id_type;
    using storage_pointer = storage_type*;
    using const_storage_pointer = const storage_type*;
    
    using reference = BitsReference<FitVector>;
    using const_reference = BitsConstReference<FitVector>;
    using iterator = BitsIterator<FitVector, false>;
    using const_iterator = BitsIterator<FitVector, true>;
    
    using value_type = id_type;
    using difference_type = long long;
    using pointer = iterator;
    
    static constexpr size_t kBitsPerWord = 8 * sizeof(id_type); // 64
    
private:
    // * Initialized only in constructor
    size_t bits_per_element_; // const
    storage_type mask_; // const
    // *
    
    size_t size_;
    std::vector<storage_type> vector_;
    
public:
    explicit FitVector(size_t unit_len = kBitsPerWord) : bits_per_element_(unit_len), mask_(bit_util::WidthMask(unit_len)), size_(0), vector_(0) {}
    
    explicit FitVector(size_t unit_len, size_t size) : FitVector(unit_len) {
        resize(size);
    }
    
    explicit FitVector(size_t unit_len, size_t size, size_t value) : FitVector(unit_len) {
        assign(size, value);
    }
    
    FitVector(std::istream& is) : FitVector(read_val<size_t>(is)) {
        size_ = read_val<size_t>(is);
        vector_ = read_vec<id_type>(is);
    }
    
    template<typename Vector>
    FitVector(const Vector& vector) : FitVector(minimal_word_size(vector)) {
        if (vector.empty())
            return;
        for (auto x : vector)
            push_back(static_cast<value_type>(x));
    }
    
    // Used at constructor
    
    template<typename Vector>
    size_t minimal_word_size(const Vector& vector) const {
        if (vector.empty())
            return 0;
        auto max_e = *std::max_element(vector.begin(), vector.end());
        return calc::SizeFitsInBits(max_e);
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
    
    size_t size() const {return size_;}
    
    bool empty() const {return size() == 0;}
    
    void resize(size_t size) {
        vector_.resize(size == 0 ? 0 : ((size * bits_per_element_ - 1) / kBitsPerWord) + 1);
        size_ = size;
    }
    
    void resize(size_t new_size, value_type value) {
        auto prev_size = size();
        resize(new_size);
        for (size_t i = prev_size; i < new_size; i++)
            operator[](i) = value;
    }
    
    void assign(size_t size, value_type value) {
        resize(size);
        for (auto i = 0; i < size; i++)
            operator[](i) = value;
    }
    
    void reserve(size_t size) {
        vector_.reserve(size == 0 ? 0 : ((size * bits_per_element_ - 1) / kBitsPerWord) + 1);
    }
    
    void push_back(value_type value) {
        resize(size() + 1);
        back() = value;
    }
    
    // MARK: method
    
    size_t size_in_bytes() const {
        auto size = sizeof(bits_per_element_) + sizeof(size_);
        size += size_vec(vector_);
        return size;
    }
    
    void Write(std::ostream &os) const {
        write_val(bits_per_element_, os);
        write_val(size_, os);
        write_vec(vector_, os);
    }
    
private:
    size_t abs_(size_t index) const {return index * bits_per_element_ / kBitsPerWord;}
    
    size_t rel_(size_t index) const {return index * bits_per_element_ % kBitsPerWord;}
    
    reference make_ref(size_t pos) {
        assert(pos < size());
        return reference(vector_.data() + abs_(pos), rel_(pos), bits_per_element_, mask_);
    }
    
    const_reference make_ref(size_t pos) const {
        assert(pos < size());
        return const_reference(vector_.data() + abs_(pos), rel_(pos), bits_per_element_, mask_);
    }
    
    iterator make_iter(size_t pos) {
        return iterator(vector_.data() + abs_(pos), rel_(pos), bits_per_element_);
    }
    
    const_iterator make_iter(size_t pos) const {
        return const_iterator(vector_.data() + abs_(pos), rel_(pos), bits_per_element_);
    }
    
};
    
} // namespace sim_ds


#endif /* Vector_hpp */
