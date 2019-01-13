
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
    
    using Reference = BitsReference<FitVector>;
    using ConstReference = BitsConstReference<FitVector>;
    using Iterator = BitsIterator<FitVector, false>;
    using ConstIterator = BitsIterator<FitVector, true>;
    
    using value_type = id_type;
    using difference_type = long long;
    using pointer = Iterator;
    
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
    
    Reference operator[](size_t index) {return make_ref(index);}
    
    ConstReference operator[](size_t index) const {return make_ref(index);}
    
    Reference at(size_t index) {
        if (index >= size())
            throw std::out_of_range("Index out of range");
        
        return operator[](index);
    }
    
    ConstReference at(size_t index) const {
        if (index >= size())
            throw std::out_of_range("Index out of range");
        
        return operator[](index);
    }
    
    Iterator begin() {return make_iter(0);}
    
    ConstIterator begin() const {return make_iter(0);}
    
    Iterator end() {return make_iter(size());}
    
    ConstIterator end() const {return make_iter(size());}
    
    Reference front() {return operator[](0);}
    
    ConstReference front() const {return operator[](0);}
    
    Reference back() {return operator[](size() - 1);}
    
    ConstReference back() const {return operator[](size() - 1);}
    
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
    
    Reference make_ref(size_t pos) {
        assert(pos < size());
        return Reference(vector_.data() + abs_(pos), rel_(pos), bits_per_element_, mask_);
    }
    
    ConstReference make_ref(size_t pos) const {
        assert(pos < size());
        return ConstReference(vector_.data() + abs_(pos), rel_(pos), bits_per_element_, mask_);
    }
    
    Iterator make_iter(size_t pos) {
        return Iterator(vector_.data() + abs_(pos), rel_(pos), bits_per_element_);
    }
    
    ConstIterator make_iter(size_t pos) const {
        return ConstIterator(vector_.data() + abs_(pos), rel_(pos), bits_per_element_);
    }
    
};
    
} // namespace sim_ds


#endif /* Vector_hpp */
