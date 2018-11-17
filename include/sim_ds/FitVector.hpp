
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
    
    template<class _Seq>
    class __bits_reference {
    private:
        using __pointer_type = typename _Seq::__pointer_type;
        using __mask_type = typename _Seq::__mask_type;
        
    public:
        static constexpr size_t _bits_per_word = sizeof(id_type) * 8;
        
        __bits_reference(__pointer_type pointer, size_t offset, size_t bitsPerUnit) : __pointer_(pointer), __offset_(offset), __bits_per_unit_(bitsPerUnit), __mask_((1ULL << bitsPerUnit) - 1) {}
        
        constexpr operator id_type() const {
            if (__bits_per_unit_ + __offset_ <= _bits_per_word) {
                return (*__pointer_ >> __offset_) & __mask_;
            } else {
                return ((*__pointer_ >> __offset_) | (*(__pointer_ + 1) << (_bits_per_word - __offset_))) & __mask_;
            }
        }
        
        __bits_reference& operator=(id_type value) {
            *__pointer_ = (*__pointer_ & ~(__mask_ << __offset_)) | (value << __offset_);
            if (__bits_per_unit_ + __offset_ > _bits_per_word) {
                auto roffset = _bits_per_word - __offset_;
                *(__pointer_ + 1) = (*(__pointer_ + 1) & ~(__mask_ >> roffset)) | value >> roffset;
            }
            return *this;
        }
        
    private:
        __pointer_type __pointer_;
        size_t __offset_;
        size_t __bits_per_unit_;
        __mask_type __mask_;
        
    };
    
    
    /*
     * Vector that fit to binary size of max-value of source vector integers.
     */
    class FitVector {
    private:
        friend class __bits_reference<FitVector>;
        using __pointer_type = id_type*;
        using __mask_type = id_type;
        
    public:
        using reference = __bits_reference<FitVector>;
        static constexpr size_t _bits_per_word = 8 * sizeof(id_type); // 64
        
        // MARK: - Constructor
        
        FitVector(size_t wordBits = _bits_per_word) : _bits_per_unit_(wordBits), _mask_((1U << wordBits) - 1) {}
        
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
        
        id_type operator[](size_t index) const {
            assert(index < size_);
            auto abs = abs_(index);
            auto rel = rel_(index);
            if (_bits_per_word >= rel + _bits_per_unit_)
                return (vector_[abs] >> rel) & _mask_;
            else
                return ((vector_[abs] >> rel) | (vector_[abs + 1] << (_bits_per_word - rel))) & _mask_;
        }
        
        reference operator[](size_t index) {
            assert(index < size_);
            return reference(&vector_[abs_(index)], rel_(index), _bits_per_unit_);
        }
        
        // MARK: Getter
        
        size_t size() const {
            return size_;
        }
        
        bool empty() const {
            return size_ == 0;
        }
        
        // MARK: - setter
        
        void push_back(size_t value) {
            auto abs = abs_(size_);
            auto rel = rel_(size_);
            if (rel == 0) {
                vector_.emplace_back(value);
            } else {
                if (abs <= long(size_) - 1) {
                    vector_[abs] |= value << rel;
                }
                if (_bits_per_unit_ + rel > _bits_per_word) {
                    vector_.emplace_back(value >> (_bits_per_word - rel));
                }
            }
            size_++;
        }
        
        void resize(size_t size) {
            if (size == 0) {
                vector_.resize(0);
            } else {
                auto abs = abs_(size - 1);
                bool crossBoundary = rel_(size - 1) + _bits_per_unit_ > _bits_per_word;
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
            auto offset = std::ceil(fs * _bits_per_unit_ / _bits_per_word);
            vector_.reserve(offset);
        }
        
        // MARK: method
        
        size_t sizeInBytes() const {
            auto size = sizeof(_bits_per_unit_) + sizeof(size_);
            size += size_vec(vector_);
            return size;
        }
        
        void write(std::ostream &os) const {
            write_val(_bits_per_unit_, os);
            write_val(size_, os);
            write_vec(vector_, os);
        }
        
        ~FitVector() = default;
        
        FitVector(const FitVector&) = default;
        FitVector& operator=(const FitVector&) = default;
        
        FitVector(FitVector&&) noexcept = default;
        FitVector& operator=(FitVector&&) noexcept = default;
        
    private:
        // * Initialized only in constructor
        size_t _bits_per_unit_;
        id_type _mask_;
        // *
        
        size_t size_ = 0;
        std::vector<id_type> vector_;
        
        size_t abs_(size_t index) const {
            return index * _bits_per_unit_ / _bits_per_word;
        }
        
        size_t rel_(size_t index) const {
            return index * _bits_per_unit_ % _bits_per_word;
        }
        
    };
    
}


#endif /* Vector_hpp */
