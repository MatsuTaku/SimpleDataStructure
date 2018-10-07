//
//  DACs.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2018/01/07.
//

#ifndef DACs_hpp
#define DACs_hpp

#include "basic.hpp"
#include "BitVector.hpp"
#include "FitVector.hpp"
#include "calc.hpp"

namespace sim_ds {
    
    class DacVector {
    public:
        static constexpr size_t kMaxSplits = 8;
        
        static std::string name() {
            return "DACs";
        }
        
        // MARK: Constructor
        
        DacVector() = default;
        
        DacVector(std::istream &is) {
            read(is);
        }
        template<class CONTAINER>
        DacVector(const CONTAINER& vector) {
            setVector(vector);
        }
        template<class CONTAINER, typename UINT>
        DacVector(const CONTAINER& vector, const std::vector<UINT>& sizes) {
            setFromVector(vector, sizes);
        }
        
        ~DacVector() = default;
        
        DacVector(const DacVector&) = delete;
        DacVector& operator =(const DacVector&) = delete;
        
        DacVector(DacVector&& rhs) noexcept {
            size_ = std::move(rhs.size_);
            num_units_ = std::move(rhs.num_units_);
            for (auto i = 0; i < kMaxSplits - 1; i++)
                bits_list_[i] = std::move(rhs.bits_list_[i]);
            for (auto i = 0; i < kMaxSplits; i++)
                units_[i] = std::move(rhs.units_[i]);
            for (auto i = 0; i < kMaxSplits; i++)
                unit_sizes_[i] = std::move(rhs.unit_sizes_[i]);
        }
        DacVector& operator =(DacVector&& rhs) noexcept = default;
        
        
        // MARK: Function
        
        template<typename C>
        void setVector(const C &vector) {
            setVector(vector, calc::splitPositionsOptimizedForSize(vector));
        }
        
        template<typename C, typename UINT>
        void setVector(const C &vector, std::vector<UINT> unitBitSizes) {
            if (vector.empty())
                return;
            if (unitBitSizes.empty())
                unitBitSizes = calc::splitPositionsOptimizedForSize(vector);
            for (auto i = 0; i < unitBitSizes.size(); i++)
                unit_sizes_[i] = unitBitSizes[i];
            expand(unitBitSizes.size());
            {
                std::vector<size_t> cf = calc::cummulativeFrequency(vector);
                for (auto i = 0, t = 0; i < unitBitSizes.size(); i++) {
                    units_[i].reserve(cf[t]);
                    t += unitBitSizes[i];
                }
                for (auto i = 0, t = 0; i < std::max(size_t(1), unitBitSizes.size() - 1); i++) {
                    bits_list_[i].resize(cf[t]);
                    t += unitBitSizes[i];
                }
            }
            
            for (auto i = 0; i < vector.size(); i++)
                set_(i, vector[i]);
            build();
        }
        
        void expand(size_t size) {
            if (num_units_ >= size) return;
            for (auto i = num_units_; i < size; i++)
                units_[i] = FitVector(unit_sizes_[i]);
            num_units_ = size;
        }
        
        void build() {
            for (auto &bits : bits_list_)
                bits.build();
        }
        
        size_t size() const {
            return size_;
        }
        
        size_t operator [](size_t index) const {
            return get(index);
        }
        
        size_t get(size_t index) const;
        
        size_t depth(size_t index) const;
        
        void showStatus(std::ostream &os) const;
        
        size_t sizeInBytes() const {
            auto size = sizeof(num_units_) + sizeof(size_);
            if (num_units_ > 0)
                for (auto i = 0; i == 0 || i + 1 < num_units_; i++)
                    size += bits_list_[i].sizeInBytes();
            for (auto i = 0; i < num_units_; i++)
                size += units_[i].sizeInBytes();
            size += sizeof(unit_sizes_);
            return size;
        }
        
        void write(std::ostream &os) const {
            write_val(size_, os);
            write_val(num_units_, os);
            if (num_units_ > 0)
                for (auto i = 0; i == 0 || i + 1 < num_units_; i++)
                    bits_list_[i].write(os);
            for (auto i = 0; i < num_units_; i++)
                units_[i].write(os);
            for (auto i = 0; i < num_units_; i++)
                write_val(unit_sizes_[i], os);
        }
        
        void read(std::istream &is) {
            size_ = read_val<size_t>(is);
            num_units_ = read_val<size_t>(is);
            if (num_units_ > 0)
                for (auto i = 0; i == 0 || i + 1 < num_units_; i++)
                    bits_list_[i].read(is);
            for (auto i = 0; i < num_units_; i++)
                units_[i] = FitVector(is);
            for (auto i = 0; i < num_units_; i++)
                unit_sizes_[i] = read_val<size_t>(is);
        }
        
    private:
        size_t size_ = 0;
        size_t num_units_ = 0;
        BitVector bits_list_[kMaxSplits - 1];
        FitVector units_[kMaxSplits];
        size_t unit_sizes_[kMaxSplits] = {8, 8, 8, 8, 8, 8, 8, 8};
        
        void set_(size_t index, size_t value);
        
    };
    
    // MARK: - inline function
    
    inline size_t DacVector::get(size_t index) const {
        size_t value = units_[0][index];
        if (!bits_list_[0][index]) return value;
        auto shiftSize = unit_sizes_[0];
        for (auto depth = 1; depth < num_units_; depth++) {
            auto &bits = bits_list_[depth - 1];
            if (!bits[index]) break;
            index = bits.rank(index);
            value |= units_[depth][index] << shiftSize;
            shiftSize += unit_sizes_[depth];
        }
        return value;
    }
    
    /**
     Get number of splits of element at index.
     0 ~ max_splits
     */
    inline size_t DacVector::depth(size_t index) const {
        size_t depth = 0;
        for (; depth < num_units_ - 1; depth++) {
            if (!bits_list_[depth][index])
                break;
            index = bits_list_[depth].rank(index);
        }
        return depth;
    }
    
    inline void DacVector::set_(size_t index, size_t value) {
        size_ = std::max(size_, index + 1);
        
        auto size = calc::sizeFitAsSizeList(value, unit_sizes_);
        if (size > num_units_)
            expand(size);
        for (auto depth = 0; depth < size; depth++) {
            auto &unit = units_[depth];
            auto curBitsSize = unit_sizes_[depth];
            unit.push_back(value & ((1U << curBitsSize) - 1));
            value >>= curBitsSize;
            index = unit.size() - 1;
            if (depth == 0 || depth + 1 < num_units_)
                bits_list_[depth].set(index, depth < size - 1);
        }
    }
    
    inline void DacVector::showStatus(std::ostream &os) const {
        using std::endl;
        os << "--- Stat of " << "DACs " << " ---" << endl;
        os << "number of elements: " << size_ << endl;
        auto size = sizeInBytes();
        os << "size:   " << size << endl;
        os << "sizeBits/element:   " << float(size * 8) / size_ << endl;
        auto bitsSize = 0;
        for (auto i = 0; i == 0 || i + 1 < num_units_; i++)
            bitsSize += bits_list_[i].sizeInBytes();
        os << "size bits:   " << bitsSize << endl;
        auto flowSize = 0;
        for (auto i = 0; i < num_units_; i++)
            flowSize += units_[i].sizeInBytes();
        os << "size flows:   " << flowSize << endl;
        os << "--- element map ---" << endl;
        os << "num_units: " << num_units_ << endl;
        for (auto i = 0; i < num_units_; i++)
            os << "[" << unit_sizes_[i] << "]"<< endl;
    }
    
}

#endif /* DACs_hpp */
