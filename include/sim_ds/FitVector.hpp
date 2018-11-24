
//
//  Vector.hpp
//  build
//
//  Created by 松本拓真 on 2018/05/04.
//

#ifndef Vector_hpp
#define Vector_hpp

#include "basic.hpp"
#include "bit_tools.hpp"
#include "calc.hpp"

namespace sim_ds {
    
template<class Sequence>
class BitsReference {
    using Pointer = typename Sequence::Pointer;
    using entity_type = typename Sequence::entity_type;
    using Mask = typename Sequence::storage_type;
    
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
    
    BitsReference& operator=(entity_type value) {
        *pointer_ = (*pointer_ & ~(mask_ << offset_)) | (value << offset_);
        if (bits_per_element_ + offset_ > kBitsPerWord) {
            auto roffset = kBitsPerWord - offset_;
            *(pointer_ + 1) = (*(pointer_ + 1) & ~(mask_ >> roffset)) | value >> roffset;
        }
        return *this;
    }
    
private:
    constexpr BitsReference(Pointer pointer, size_t offset, size_t bits_per_element) noexcept : pointer_(pointer), offset_(offset), bits_per_element_(bits_per_element), mask_(bit_tools::BitsMask(bits_per_element)) {}
    
    Pointer pointer_;
    size_t offset_;
    size_t bits_per_element_;
    Mask mask_;
    
};


template<class Sequence>
class BitsConstReference {
    using Pointer = typename Sequence::ConstPointer;
    using entity_type = typename Sequence::entity_type;
    using Mask = typename Sequence::storage_type;
    
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
    constexpr BitsConstReference(Pointer pointer, size_t offset, size_t bits_per_element) noexcept : pointer_(pointer), offset_(offset), bits_per_element_(bits_per_element), mask_(bit_tools::BitsMask(bits_per_element)) {}
    
    Pointer pointer_;
    size_t offset_;
    size_t bits_per_element_;
    Mask mask_;
    
};


/*
 * Vector that fit to binary size of max-value of source vector integers.
 */
class FitVector {
    using Self = FitVector;
    using entity_type = id_type;
    using storage_type = id_type;
    using Pointer = storage_type*;
    using ConstPointer = const storage_type*;
    
    // * Initialized only in constructor
    size_t bits_per_element_;
    storage_type mask_;
    // *
    
    size_t size_ = 0;
    std::vector<storage_type> vector_;
    
    friend class BitsReference<FitVector>;
    friend class BitsConstReference<FitVector>;
    
    using Reference = BitsReference<FitVector>;
    using ConstReference = BitsConstReference<FitVector>;
    
    static constexpr size_t kBitsPerWord = 8 * sizeof(id_type); // 64
    
public:
    // MARK: Constructor
    
    FitVector(size_t wordBits = kBitsPerWord) : bits_per_element_(wordBits), mask_(bit_tools::BitsMask(wordBits)) {}
    
    FitVector(size_t wordBits, size_t size) : FitVector(wordBits) {
        resize(size);
    }
    
    FitVector(size_t wordBits, size_t size, size_t value) : FitVector(wordBits) {
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
        return calc::size_fits_in_bits(max_e);
    }
    
    // MARK: Operator
    
    Reference operator[](size_t index) {
        assert(index < size());
        return Reference(&vector_[abs_(index)], rel_(index), bits_per_element_);
    }
    
    ConstReference operator[](size_t index) const {
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
    
    // MARK: Getter
    
    size_t size() const {
        return size_;
    }
    
    bool empty() const {
        return size_ == 0;
    }
    
    // MARK: setter
    
    void push_back(size_t value) {
        auto backI = size();
        resize(size() + 1);
        operator[](backI) = value;
    }
    
    void resize(size_t size) {
        auto newSize = ceil(float(size) * bits_per_element_ / kBitsPerWord);
        vector_.resize(newSize);
        size_ = size;
    }
    
    void assign(size_t size, size_t value) {
        resize(size);
        for (auto i = 0; i < size; i++) {
            operator[](i) = value;
        }
    }
    
    void reserve(size_t size) {
        float fs = size;
        auto offset = std::ceil(fs * bits_per_element_ / kBitsPerWord);
        vector_.reserve(offset);
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
    
    size_t abs_(size_t index) const {
        return index * bits_per_element_ / kBitsPerWord;
    }
    
    size_t rel_(size_t index) const {
        return index * bits_per_element_ % kBitsPerWord;
    }
    
public:
    
    ~FitVector() = default;
    
    FitVector(const FitVector&) = default;
    FitVector& operator=(const FitVector&) = default;
    
    FitVector(FitVector&&) noexcept = default;
    FitVector& operator=(FitVector&&) noexcept = default;
    
};
    
} // namespace sim_ds


#endif /* Vector_hpp */
