//
//  FitValuesArray.hpp
//  bench
//
//  Created by 松本拓真 on 2018/01/05.
//

#ifndef FitValuesArray_hpp
#define FitValuesArray_hpp

#include "basic.hpp"

namespace sim_ds {
    
    class FitValuesArray {
    public:
        using IdType = uint8_t;
        
        void setValueSize(size_t index, size_t size);
        
        template <typename T>
        void setValueSizes(std::vector<T>& sizes) {
            value_sizes_ = {};
            value_positions_ = {};
            element_size_ = 0;
            for (auto i = 0; i < sizes.size(); i++)
                setValueSize(i, sizes[i]);
        }
        
        size_t offset(size_t index) const {
            return index * element_size_;
        }
        
        size_t get_element_size_() const {
            return element_size_;
        }
        
        size_t numElements() const {
            return bytes_.size() / element_size_;
        }
        
        template <typename T>
        T getValue(size_t index, size_t num) const;
        
        template <typename T>
        void setValue(size_t index, size_t num, T value);
        
        void resize(size_t indexSize) {
            bytes_.resize(offset(indexSize));
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const {
            auto size = size_vec(bytes_);
            size += size_vec(value_sizes_);
            return size;
        }
        
        void write(std::ostream &os) const {
            write_vec(bytes_, os);
            write_vec(value_sizes_, os);
        }
        
        void read(std::istream &is) {
            bytes_ = read_vec<IdType>(is);
            
            auto sizes = read_vec<uint8_t>(is);
            for (auto i = 0; i < sizes.size(); i++) {
                setValueSize(i, sizes[i]);
            }
        }
        
        // MARK: copy guard
        
        FitValuesArray() = default;
        ~FitValuesArray() = default;
        
        FitValuesArray(const FitValuesArray&) = delete;
        FitValuesArray& operator=(const FitValuesArray&) = delete;
        
        FitValuesArray(FitValuesArray &&rhs) noexcept = default;
        FitValuesArray& operator=(FitValuesArray &&rhs) noexcept = default;
        
    private:
        std::vector<IdType> bytes_ = {};
        std::vector<uint8_t> value_sizes_ = {};
        uint8_t element_size_ = 0;
        std::vector<size_t> value_positions_ = {};
        
    };
    
    // MARK: - inline function
    
    inline void FitValuesArray::setValueSize(size_t index, size_t size) {
        element_size_ += size;
        value_sizes_.insert(value_sizes_.begin() + index, size);
        
        auto pos = value_positions_.size() > 0 ? value_positions_[index - 1] + value_sizes_[index - 1] : 0;
        value_positions_.insert(value_positions_.begin() + index, pos);
        
        if (index == value_positions_.size() - 1)
            return;
        for (auto i = index + 1; i < value_positions_.size(); i++)
            value_positions_[i] += size;
    }
    
    template <typename T>
    inline T FitValuesArray::getValue(size_t index, size_t num) const {
        assert(sizeof(T) >= value_sizes_[num]);
        T value = 0;
        auto pos = offset(index) + value_positions_[num];
        for (size_t i = 0, size = value_sizes_[num]; i < size; i++)
            value |= bytes_[pos + i] << (i * 8);
        return value;
    }
    
    template <typename T>
    inline void FitValuesArray::setValue(size_t index, size_t num, T value) {
        assert(sizeof(T) >= value_sizes_[num]);
        auto pos = offset(index) + value_positions_[num];
        for (auto i = 0; i < value_sizes_[num]; i++)
            bytes_[pos + i] = static_cast<IdType>(value >> (8 * i));
    }
    
}

#endif /* FitValuesArray_hpp */
