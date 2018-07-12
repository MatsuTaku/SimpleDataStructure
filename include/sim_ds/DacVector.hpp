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
#include "Vector.hpp"
#include "Calc.hpp"

namespace sim_ds {
    
    class DacVector {
    public:
        using idType = uint64_t;
        static constexpr size_t kMaxSeparates = 8;
        
        // MARK: Constructor
        
        DacVector() = default;
        
        DacVector(std::istream &is) {
            read(is);
        }
        template<typename T>
        DacVector(const std::vector<T> &vector, size_t minCost = 1, size_t maxLevels = 8) {
            setFromVector(vector, Calc::optimizedBitsListForDac(vector, minCost, maxLevels));
        }
        template<typename T>
        DacVector(const std::vector<T> &vector, const std::vector<size_t> &sizes) {
            setFromVector(vector, sizes);
        }
        DacVector(const Vector &vector, size_t minCost = 1, size_t maxLevels = 8) {
            std::vector<size_t> vec(vector.size());
            for (auto i = 0; i < vector.size(); i++) {
                vec[i] = vector[i];
            }
            setFromVector(vec, Calc::optimizedBitsListForDac(vec, minCost, maxLevels));
        }
        DacVector(const Vector &vector, const std::vector<size_t> &sizes) {
            std::vector<size_t> vec(vector.size());
            for (auto i = 0; i < vector.size(); i++) {
                vec[i] = vector[i];
            }
            setFromVector(vec, sizes);
        }
        
        ~DacVector() = default;
        
        DacVector(const DacVector&) = delete;
        DacVector& operator =(const DacVector&) = delete;
        
        DacVector(DacVector&& rhs) noexcept {
            size_ = std::move(rhs.size_);
            num_units_ = std::move(rhs.num_units_);
            for (auto i = 0; i < sizeof(idType) - 1; i++)
                bits_list_[i] = std::move(rhs.bits_list_[i]);
            for (auto i = 0; i < sizeof(idType); i++)
                units_[i] = std::move(rhs.units_[i]);
            for (auto i = 0; i < kMaxSeparates; i++)
                bits_sizes_[i] = std::move(rhs.bits_sizes_[i]);
        }
        DacVector& operator =(DacVector&& rhs) noexcept = default;
        
    public:
        // MARK: Function
        
        static std::string name() {
            return "DACs";
        }
        
        template<typename C>
        void setFromVector(const C &vector, const std::vector<size_t> &optimized) {
            for (auto i = 0; i < optimized.size(); i++)
                bits_sizes_[i] = optimized[i];
            expand(optimized.size());
            for (auto i = 0; i < vector.size(); i++)
                set(i, vector[i]);
            build();
        }
        
        void expand(size_t size) {
            if (num_units_ >= size) return;
            for (auto i = num_units_; i < size; i++)
                units_[i] = Vector(bits_sizes_[i]);
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
        
        void set(size_t index, size_t value);
        
        void showStatus(std::ostream &os) const;
        
        size_t sizeInBytes() const {
            auto size = sizeof(num_units_) + sizeof(size_);
            if (num_units_ > 0)
                for (auto i = 0; i == 0 || i + 1 < num_units_; i++)
                    size += bits_list_[i].sizeInBytes();
            for (auto i = 0; i < num_units_; i++)
                size += units_[i].sizeInBytes();
            size += sizeof(bits_sizes_);
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
                write_val(bits_sizes_[i], os);
        }
        
        void read(std::istream &is) {
            size_ = read_val<size_t>(is);
            num_units_ = read_val<size_t>(is);
            if (num_units_ > 0)
                for (auto i = 0; i == 0 || i + 1 < num_units_; i++)
                    bits_list_[i].read(is);
            for (auto i = 0; i < num_units_; i++)
                units_[i] = Vector(is);
            for (auto i = 0; i < num_units_; i++)
                bits_sizes_[i] = read_val<size_t>(is);
        }
        
    private:
        size_t size_ = 0;
        size_t num_units_ = 0;
        BitVector bits_list_[sizeof(idType) - 1];
        Vector units_[sizeof(idType)];
        size_t bits_sizes_[kMaxSeparates] = {8, 8, 8, 8, 8, 8, 8, 8};
        
    };
    
    // MARK: - inline function
    
    inline size_t DacVector::get(size_t index) const {
        size_t value = units_[0][index];
        if (!bits_list_[0][index]) return value;
        auto shiftSize = bits_sizes_[0];
        for (auto depth = 1; depth < num_units_; depth++) {
            auto &bits = bits_list_[depth - 1];
            if (!bits[index]) break;
            index = bits.rank(index);
            value |= units_[depth][index] << shiftSize;
            shiftSize += bits_sizes_[depth];
        }
        return value;
    }
    
    inline void DacVector::set(size_t index, size_t value) {
        size_ = std::max(size_, index + 1);
        
        auto size = Calc::sizeFitAsSizeList(value, bits_sizes_);
        if (size > num_units_)
            expand(size);
        for (auto depth = 0; depth < size; depth++) {
            auto &unit = units_[depth];
            auto curBitsSize = bits_sizes_[depth];
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
            os << "[" << bits_sizes_[i] << "]"<< endl;
    }
    
}

#endif /* DACs_hpp */
