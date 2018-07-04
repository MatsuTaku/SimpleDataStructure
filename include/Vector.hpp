
//
//  Vector.hpp
//  build
//
//  Created by 松本拓真 on 2018/05/04.
//

#ifndef Vector_hpp
#define Vector_hpp

#include "basic.hpp"
#include "Calc.hpp"

namespace sim_ds {
    
    /*
     * Vector that fit to binary size of max-value of source vector integers.
     */
    class Vector {
    public:
        using id_type = uint64_t;
        static constexpr size_t kBitsSizeInElement = 8 * sizeof(id_type); // 64
    public:
        // MARK: - Constructor
        
        Vector(size_t typeSize = kBitsSizeInElement) : sizeBits_reference_(typeSize), mask_reference_((1U << typeSize) - 1) {}
        
        Vector(size_t typeSize, size_t size) : Vector(typeSize) {
            resize(size);
        }
        
        Vector(size_t typeSize, size_t size, size_t value) : Vector(typeSize) {
            assign(size, value);
        }
        
        Vector(std::istream &is) : Vector(read_val<size_t>(is)) {
            size_ = read_val<size_t>(is);
            vector_ = read_vec<uint64_t>(is);
        }
        
        template<typename T>
        Vector(const std::vector<T> &vector) : Vector(typeSizeOfVector(vector)) {
            resize(vector.size());
            for (auto i = 0; i < vector.size(); i++) {
                set(i, vector[i]);
            }
        }
        
        Vector(const Vector &rhs) : Vector(rhs.sizeBits_reference_) {
            size_ = rhs.size_;
            vector_ = rhs.vector_;
        }
//        Vector& operator=(const Vector &rhs) noexcept = default;
        Vector& operator=(const Vector &rhs) noexcept {
            sizeBits_reference_ = rhs.sizeBits_reference_;
            mask_reference_ = rhs.mask_reference_;

            size_ = rhs.size_;
            vector_ = rhs.vector_;

            return *this;
        }
        Vector(Vector &&rhs) noexcept = default;
//        Vector(Vector &&rhs) : Vector(std::move(rhs.sizeBits_reference_)) {
//            size_ = std::move(rhs.size_);
//            vector_ = std::move(rhs.vector_);
//        }
        Vector& operator=(Vector &&rhs) noexcept = default;
        
        //        Vector(const Vector &rhs) : kBitsSizeOfTypes_(rhs.kBitsSizeOfTypes_), kMask_(rhs.kMask_) {
        //            size_ = rhs.size_;
        //            vector_ = rhs.vector_;
        //        }
        //        Vector& operator=(const Vector &rhs) noexcept {
        //            // const members
        //            size_t *ptr = const_cast<size_t*>(&this->kBitsSizeOfTypes_);
        //            *ptr = rhs.kBitsSizeOfTypes_;
        //            ptr = const_cast<size_t*>(&this->kMask_);
        //            *ptr = rhs.kMask_;
        //            // var members
        //            size_ = rhs.size_;
        //            vector_ = rhs.vector_;
        //
        //            return *this;
        //        }
        //
        //        Vector(Vector &&rhs) : kBitsSizeOfTypes_(rhs.kBitsSizeOfTypes_), kMask_(rhs.kMask_) {
        //            size_ = std::move(rhs.size_);
        //            vector_ = std::move(rhs.vector_);
        //        }
        //        Vector& operator=(Vector &&rhs) noexcept {
        //            // const members
        //            size_t *ptr = const_cast<size_t*>(&this->kBitsSizeOfTypes_);
        //            *ptr = std::move(rhs.kBitsSizeOfTypes_);
        //            ptr = const_cast<size_t*>(&this->kMask_);
        //            *ptr = std::move(rhs.kMask_);
        //            // var members
        //            size_ = std::move(rhs.size_);
        //            vector_ = std::move(rhs.vector_);
        //
        //            return *this;
        //        }

//        ~Vector() = default;

        ~Vector() = default;
        
        // Used at constructor
        
        template<typename T>
        size_t typeSizeOfVector(const std::vector<T> &vector) const {
            auto maxV = *std::max_element(vector.begin(), vector.end());
            return Calc::sizeFitInBits(maxV);
        }
        
        // MARK: - getter
        
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
            resize(size_ + 1);
            set(size_ - 1, value);
        }
        
        void resize(size_t size) {
            if (size == 0)
                vector_.resize(0);
            else {
                auto abs = abs_(size - 1);
                if (rel_(size - 1) + kBitsSizeOfTypes_ > kBitsSizeInElement)
                    vector_.resize(abs + 2);
                else
                    vector_.resize(abs + 1);
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
        const size_t &kBitsSizeOfTypes_ = sizeBits_reference_;
        const size_t &kMask_ = mask_reference_;
        
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
