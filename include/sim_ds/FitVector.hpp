
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
    
    template<class SequenceType>
    class BitsReference {
        using pointer = typename SequenceType::pointer;
        using entity_type = typename SequenceType::entity_type;
        using mask_type = typename SequenceType::storage_type;
        
        friend typename SequenceType::self;
        
        static constexpr size_t kBitsPerWord = sizeof(id_type) * 8;
        
    public:
        constexpr operator entity_type() const {
            if (bits_per_unit_ + offset_ <= kBitsPerWord) {
                return (*pointer_ >> offset_) & mask_;
            } else {
                return ((*pointer_ >> offset_) | (*(pointer_ + 1) << (kBitsPerWord - offset_))) & mask_;
            }
        }
        
        BitsReference& operator=(entity_type value) {
            *pointer_ = (*pointer_ & ~(mask_ << offset_)) | (value << offset_);
            if (bits_per_unit_ + offset_ > kBitsPerWord) {
                auto roffset = kBitsPerWord - offset_;
                *(pointer_ + 1) = (*(pointer_ + 1) & ~(mask_ >> roffset)) | value >> roffset;
            }
            return *this;
        }
        
    private:
        constexpr BitsReference(pointer pointer, size_t offset, size_t bitsPerUnit) noexcept : pointer_(pointer), offset_(offset), bits_per_unit_(bitsPerUnit), mask_(bit_tools::maskOfBits(bitsPerUnit)) {}
        
        pointer pointer_;
        size_t offset_;
        size_t bits_per_unit_;
        mask_type mask_;
        
    };
    
    
    template<class SequenceType>
    class BitsConstReference {
        using pointer = typename SequenceType::const_pointer;
        using entity_type = typename SequenceType::entity_type;
        using mask_type = typename SequenceType::storage_type;
        
        friend typename SequenceType::self;
        
        static constexpr size_t kBitsPerWord = sizeof(id_type) * 8;
        
    public:
        constexpr operator entity_type() const {
            if (bits_per_unit_ + offset_ <= kBitsPerWord) {
                return (*pointer_ >> offset_) & mask_;
            } else {
                return ((*pointer_ >> offset_) | (*(pointer_ + 1) << (kBitsPerWord - offset_))) & mask_;
            }
        }
        
    private:
        constexpr BitsConstReference(pointer pointer, size_t offset, size_t bitsPerUnit) noexcept : pointer_(pointer), offset_(offset), bits_per_unit_(bitsPerUnit), mask_(bit_tools::maskOfBits(bitsPerUnit)) {}
        
        pointer pointer_;
        size_t offset_;
        size_t bits_per_unit_;
        mask_type mask_;
        
    };
    
    
    /*
     * Vector that fit to binary size of max-value of source vector integers.
     */
    class FitVector {
        using self = FitVector;
        using entity_type = id_type;
        using storage_type = id_type;
        using pointer = storage_type*;
        using const_pointer = const storage_type*;
        
        // * Initialized only in constructor
        size_t bits_per_unit_;
        id_type mask_;
        // *
        
        size_t size_ = 0;
        std::vector<id_type> vector_;
        
        friend class BitsReference<FitVector>;
        friend class BitsConstReference<FitVector>;
        
        using reference = BitsReference<FitVector>;
        using const_reference = BitsConstReference<FitVector>;
        
        static constexpr size_t kBitsPerWord = 8 * sizeof(id_type); // 64
        
    public:
        // MARK: Constructor
        
        FitVector(size_t wordBits = kBitsPerWord) : bits_per_unit_(wordBits), mask_(bit_tools::maskOfBits(wordBits)) {}
        
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
        FitVector(const std::vector<T> &vector) : FitVector(typeSizeOfVector(vector)) {
            if (vector.empty())
                return;
            resize(vector.size());
            for (auto i = 0; i < vector.size(); i++) {
                operator[](i) = vector[i];
            }
        }
        
        // Used at constructor
        
        template<typename T>
        size_t typeSizeOfVector(const std::vector<T> &vector) const {
            if (vector.empty())
                return 0;
            auto maxV = *std::max_element(vector.begin(), vector.end());
            return calc::sizeFitsInBits(maxV);
        }
        
        // MARK: Operator
        
        reference operator[](size_t index) {
            assert(index < size());
            return reference(&vector_[abs_(index)], rel_(index), bits_per_unit_);
        }
        
        const_reference operator[](size_t index) const {
            assert(index < size());
            return const_reference(&vector_[abs_(index)], rel_(index), bits_per_unit_);
        }
        
        reference front() {
            return operator[](0);
        }
        
        const_reference front() const {
            return operator[](0);
        }
        
        reference back() {
            return operator[](size() - 1);
        }
        
        const_reference back() const {
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
            auto abs = abs_(size_);
            auto rel = rel_(size_);
            if (rel == 0) {
                vector_.emplace_back(value);
            } else {
                if (abs <= long(size_) - 1) {
                    vector_[abs] |= value << rel;
                }
                if (bits_per_unit_ + rel > kBitsPerWord) {
                    vector_.emplace_back(value >> (kBitsPerWord - rel));
                }
            }
            size_++;
        }
        
        void resize(size_t size) {
            if (size == 0) {
                vector_.resize(0);
            } else {
                auto abs = abs_(size - 1);
                bool crossBoundary = rel_(size - 1) + bits_per_unit_ > kBitsPerWord;
                vector_.resize(abs + (crossBoundary ? 2 : 1));
            }
            if (size < size_)
                vector_.shrink_to_fit();
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
            auto offset = std::ceil(fs * bits_per_unit_ / kBitsPerWord);
            vector_.reserve(offset);
        }
        
        // MARK: method
        
        size_t sizeInBytes() const {
            auto size = sizeof(bits_per_unit_) + sizeof(size_);
            size += size_vec(vector_);
            return size;
        }
        
        void write(std::ostream &os) const {
            write_val(bits_per_unit_, os);
            write_val(size_, os);
            write_vec(vector_, os);
        }
        
    private:
        
        size_t abs_(size_t index) const {
            return index * bits_per_unit_ / kBitsPerWord;
        }
        
        size_t rel_(size_t index) const {
            return index * bits_per_unit_ % kBitsPerWord;
        }
        
    public:
        
        ~FitVector() = default;
        
        FitVector(const FitVector&) = default;
        FitVector& operator=(const FitVector&) = default;
        
        FitVector(FitVector&&) noexcept = default;
        FitVector& operator=(FitVector&&) noexcept = default;
        
    };
    
}


#endif /* Vector_hpp */
