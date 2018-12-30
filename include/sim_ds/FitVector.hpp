
//
//  Vector.hpp
//  build
//
//  Created by 松本拓真 on 2018/05/04.
//

#ifndef Vector_hpp
#define Vector_hpp

#include "basic.hpp"
#include "bit_util.hpp"
#include "calc.hpp"
#include "log.hpp"

namespace sim_ds {


template<class Sequence>
class BitsReference {
    using storage_type = typename Sequence::storage_type;
    using storage_pointer = typename Sequence::storage_pointer;
    using entity_type = typename Sequence::entity_type;
    
    friend typename Sequence::Self;
    
    static constexpr size_t kBitsPerWord = sizeof(id_type) * 8;
    
public:
    constexpr operator entity_type() const {
        if (bits_per_element_ + offset_ <= kBitsPerWord) {
            return (*pointer_ >> offset_) & mask_;
        } else {
            return ((*pointer_ >> offset_) | (*(pointer_ + 1) << (kBitsPerWord - offset_))) & mask_;
        }
    }
    
    constexpr BitsReference& operator=(entity_type value) {
        *pointer_ = (*pointer_ & ~(mask_ << offset_)) | (value << offset_);
        if (bits_per_element_ + offset_ > kBitsPerWord) {
            auto roffset = kBitsPerWord - offset_;
            *(pointer_ + 1) = (*(pointer_ + 1) & ~(mask_ >> roffset)) | value >> roffset;
        }
        return *this;
    }
    
private:
    constexpr BitsReference(storage_pointer pointer,
                            size_t offset,
                            size_t bits_per_element
                            ) noexcept : pointer_(pointer), offset_(offset), bits_per_element_(bits_per_element), mask_(bit_util::WidthMask(bits_per_element)) {}
    
    storage_pointer pointer_;
    size_t offset_;
    size_t bits_per_element_;
    storage_type mask_;
    
};


template<class Sequence>
class BitsConstReference {
    using storage_type = typename Sequence::storage_type;
    using storage_pointer = typename Sequence::const_storage_pointer;
    using entity_type = typename Sequence::entity_type;
    
    friend typename Sequence::Self;
    
    static constexpr size_t kBitsPerWord = sizeof(id_type) * 8;
    
public:
    constexpr operator entity_type() const {
        if (bits_per_element_ + offset_ <= kBitsPerWord) {
            return (*pointer_ >> offset_) & mask_;
        } else {
            return ((*pointer_ >> offset_) | (*(pointer_ + 1) << (kBitsPerWord - offset_))) & mask_;
        }
    }
    
private:
    constexpr BitsConstReference(storage_pointer pointer,
                                 size_t offset,
                                 size_t bits_per_element
                                 ) noexcept : pointer_(pointer), offset_(offset), bits_per_element_(bits_per_element), mask_(bit_util::WidthMask(bits_per_element)) {}
    
    storage_pointer pointer_;
    size_t offset_;
    size_t bits_per_element_;
    storage_type mask_;
    
};


/*
 * Vector that fit to binary size of max-value of source vector integers.
 */
class FitVector {
    using Self = FitVector;
    using storage_type = id_type;
    using storage_pointer = storage_type*;
    using const_storage_pointer = const storage_type*;
    using entity_type = id_type;
    
    friend class BitsReference<FitVector>;
    friend class BitsConstReference<FitVector>;
    
    using Reference = BitsReference<FitVector>;
    using ConstReference = BitsConstReference<FitVector>;
    
    static constexpr size_t kBitsPerWord = 8 * sizeof(id_type); // 64
    
private:
    // * Initialized only in constructor
    size_t bits_per_element_;
    storage_type mask_;
    // *
    
    size_t size_ = 0;
    std::vector<storage_type> vector_;
    
    size_t abs_(size_t index) const {
        return index * bits_per_element_ / kBitsPerWord;
    }
    
    size_t rel_(size_t index) const {
        return index * bits_per_element_ % kBitsPerWord;
    }
    
public:
    FitVector() : FitVector(kBitsPerWord) {}
    
    FitVector(size_t unit_width) : bits_per_element_(unit_width), mask_(bit_util::WidthMask(unit_width)) {}
    
    FitVector(size_t unit_width, size_t size) : FitVector(unit_width) {
        resize(size);
    }
    
    FitVector(size_t unit_width, size_t size, size_t value) : FitVector(unit_width) {
        assign(size, value);
    }
    
    FitVector(std::istream& is) : FitVector(read_val<size_t>(is)) {
        size_ = read_val<size_t>(is);
        vector_ = read_vec<id_type>(is);
    }
    
    template<typename T>
    FitVector(const std::vector<T>& vector) : FitVector(optimal_size_of_element(vector)) {
        if (vector.empty())
            return;
        resize(vector.size());
        for (auto i = 0; i < vector.size(); i++) {
            operator[](i) = vector[i];
        }
    }
    
    // Used at constructor
    
    template<typename T>
    size_t optimal_size_of_element(const std::vector<T>& vector) const {
        if (vector.empty())
            return 0;
        auto max_e = *std::max_element(vector.begin(), vector.end());
        return calc::SizeFitsInBits(max_e);
    }
    
    // MARK: Operator
    
    constexpr Reference operator[](size_t index) {
        assert(index < size());
        return Reference(&vector_[abs_(index)], rel_(index), bits_per_element_);
    }
    
    constexpr ConstReference operator[](size_t index) const {
        assert(index < size());
        return ConstReference(&vector_[abs_(index)], rel_(index), bits_per_element_);
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
    
    // MARK: setter
    
    void resize(size_t size) {
        auto new_size = ceil(double(size) * bits_per_element_ / kBitsPerWord);
        vector_.resize(new_size);
        size_ = size;
    }
    
    void assign(size_t size, entity_type value) {
        resize(size);
        for (auto i = 0; i < size; i++) {
            operator[](i) = value;
        }
    }
    
    void reserve(size_t size) {
        float fs = size;
        auto offset = ceil(fs * bits_per_element_ / kBitsPerWord);
        vector_.reserve(offset);
    }
    
    void push_back(entity_type value) {
        auto backI = size();
        resize(size() + 1);
        operator[](backI) = value;
    }
    
    // MARK: Getter
    
    size_t size() const {
        return size_;
    }
    
    bool empty() const {
        return size() == 0;
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
    
    
};
    
} // namespace sim_ds


#endif /* Vector_hpp */
