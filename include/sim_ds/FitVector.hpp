
//
//  Vector.hpp
//  build
//
//  Created by 松本拓真 on 2018/05/04.
//

#ifndef Vector_hpp
#define Vector_hpp

#include "basic.hpp"
#include "calc.hpp"

namespace sim_ds {
    
    /*
     * Vector that fit to binary size of max-value of source vector integers.
     */
    class FitVector {
    public:
        static constexpr size_t kBitsSizeInElement = 8 * sizeof(id_type); // 64
        
        // MARK: - Constructor
        
        FitVector(size_t typeSize = kBitsSizeInElement) : sizeBits_reference_(typeSize), mask_reference_((1U << typeSize) - 1) {}
        
        FitVector(size_t typeSize, size_t size) : FitVector(typeSize) {
            resize(size);
        }
        
        FitVector(size_t typeSize, size_t size, size_t value) : FitVector(typeSize) {
            assign(size, value);
        }
        
        FitVector(std::istream &is) : FitVector(read_val<size_t>(is)) {
            size_ = read_val<size_t>(is);
            vector_ = read_vec<id_type>(is);
        }
        
        template<typename T>
        FitVector(const std::vector<T> &vector) : FitVector(typeSizeOfVector(vector)) {
            if (vector.empty())
                return;
            resize(vector.size());
            for (auto i = 0; i < vector.size(); i++) {
                set(i, vector[i]);
            }
        }
        
        FitVector(const FitVector &rhs) : FitVector(rhs.sizeBits_reference_) {
            size_ = rhs.size_;
            vector_ = rhs.vector_;
        }
        FitVector& operator=(const FitVector &rhs) noexcept {
            sizeBits_reference_ = rhs.sizeBits_reference_;
            mask_reference_ = rhs.mask_reference_;

            size_ = rhs.size_;
            vector_ = rhs.vector_;

            return *this;
        }
        FitVector(FitVector &&rhs) noexcept = default;
        FitVector& operator=(FitVector &&rhs) noexcept = default;

        ~FitVector() = default;
        
        // Used at constructor
        
        template<typename T>
        size_t typeSizeOfVector(const std::vector<T> &vector) const {
            if (vector.empty())
                return 0;
            auto maxV = *std::max_element(vector.begin(), vector.end());
            return sim_ds::calc::sizeFitInBits(maxV);
        }
        
        // MARK: Getter
        
        constexpr size_t operator[](size_t index) const {
            auto abs = abs_(index);
            auto rel = rel_(index);
            if (kBitsSizeInElement >= rel + kBitsSizeOfTypes_)
                return (vector_[abs] >> rel) & kMask_;
            else
                return ((vector_[abs] >> rel) | (vector_[abs + 1] << (kBitsSizeInElement - rel))) & kMask_;
        }
        
        size_t size() const {
            return size_;
        }
        
        bool empty() const {
            return size_ == 0;
        }
        
        // MARK: - setter
        
        void set(size_t index, size_t value) {
            auto abs = abs_(index);
            auto rel = rel_(index);
            vector_[abs] &= ~(kMask_ << rel);
            vector_[abs] |= value << rel;
            if (kBitsSizeOfTypes_ + rel > kBitsSizeInElement) {
                vector_[abs + 1] &= ~(kMask_ >> (kBitsSizeInElement - rel));
                vector_[abs + 1] |= value >> (kBitsSizeInElement - rel);
            }
        }
        
        void push_back(size_t value) {
            auto abs = abs_(size_);
            auto rel = rel_(size_);
            if (rel == 0) {
                vector_.emplace_back(value);
            } else {
                if (abs <= long(size_) - 1) {
                    vector_[abs] |= value << rel;
                }
                if (kBitsSizeOfTypes_ + rel > kBitsSizeInElement) {
                    vector_.emplace_back(value >> (kBitsSizeInElement - rel));
                }
            }
            size_++;
        }
        
        void resize(size_t size) {
            if (size == 0) {
                vector_.resize(0);
            } else {
                auto abs = abs_(size - 1);
                bool crossBoundary = rel_(size - 1) + kBitsSizeOfTypes_ > kBitsSizeInElement;
                vector_.resize(abs + (crossBoundary ? 2 : 1));
            }
            if (size < size_)
                vector_.shrink_to_fit();
            size_ = size;
        }
        
        void assign(size_t size, size_t value) {
            vector_.resize(abs_(size));
            for (auto i = 0; i < size; i++)
                set(i, value);
        }
        
        void reserve(size_t size) {
            float fs = size;
            auto offset = std::ceil(fs * kBitsSizeOfTypes_ / kBitsSizeInElement);
            vector_.reserve(offset);
        }
        
        // MARK: - method
        
        size_t sizeInBytes() const {
            auto size = sizeof(sizeBits_reference_) + sizeof(size_);
            size += size_vec(vector_);
            return size;
        }
        
        void write(std::ostream &os) const {
            write_val(sizeBits_reference_, os);
            write_val(size_, os);
            write_vec(vector_, os);
        }
        
    private:
        size_t sizeBits_reference_;
        size_t mask_reference_;
        
    protected:
        const size_t& kBitsSizeOfTypes_ = sizeBits_reference_;
        const size_t& kMask_ = mask_reference_;
        
    private:
        size_t size_ = 0;
        std::vector<id_type> vector_;
        
        // MARK: - private function
        
        constexpr size_t abs_(size_t index) const {
            return index * kBitsSizeOfTypes_ / kBitsSizeInElement;
        }
        
        constexpr size_t rel_(size_t index) const {
            return index * kBitsSizeOfTypes_ % kBitsSizeInElement;
        }
        
    };
    
}


#endif /* Vector_hpp */
