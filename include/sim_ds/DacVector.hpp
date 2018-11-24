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
    
private:
    size_t num_layers_ = 0;
    size_t layer_unit_sizes_[kMaxSplits] = {8, 8, 8, 8, 8, 8, 8, 8};
    FitVector layers_[kMaxSplits];
    BitVector paths_[kMaxSplits - 1];
    
public:
    // MARK: Constructor
    
    DacVector(std::istream &is) {
        Read(is);
    }
    template<class _Seq>
    DacVector(const _Seq& vector) {
        ConvertFromVector(vector);
    }
    template<class _Seq, typename UINT>
    DacVector(const _Seq& vector, const std::vector<UINT>& sizes) {
        ConvertFromVector(vector, sizes);
    }
    
    
    // MARK: Function
    
    template<typename Sequence>
    void ConvertFromVector(const Sequence& vector) {
        ConvertFromVector(vector, calc::split_positions_optimized_for_dac(vector));
    }
    
    template<typename Sequence, typename UInt>
    void ConvertFromVector(const Sequence& vector, std::vector<UInt> unitBitSizes) {
        if (vector.empty())
            return;
        if (unitBitSizes.empty())
            unitBitSizes = calc::split_positions_optimized_for_dac(vector);
        for (auto i = 0; i < unitBitSizes.size(); i++)
            layer_unit_sizes_[i] = unitBitSizes[i];
        Expand_(unitBitSizes.size());
        {
            std::vector<size_t> cf = calc::cummulative_frequency_list(vector);
            for (auto i = 0, t = 0; i < unitBitSizes.size(); i++) {
                layers_[i].resize(0);
                layers_[i].reserve(cf[t]);
                t += unitBitSizes[i];
            }
            for (auto i = 0, t = 0; i < std::max(size_t(1), unitBitSizes.size() - 1); i++) {
                paths_[i].resize(0);
                paths_[i].resize(cf[t]);
                t += unitBitSizes[i];
            }
        }
        
        for (auto v : vector) {
            push_back(v);
        }
        Build();
    }
    
    void push_back(id_type value) {
        auto size = calc::size_fits_as_list(value, layer_unit_sizes_);
        if (size > num_layers_)
            Expand_(size);
        for (auto depth = 0, index = 0; depth < size; depth++) {
            auto& unit = layers_[depth];
            auto curBitsSize = layer_unit_sizes_[depth];
            unit.push_back(value & bit_tools::BitsMask(curBitsSize));
            value >>= curBitsSize;
            index = unit.size() - 1;
            if (depth == 0 || depth + 1 < num_layers_)
                paths_[depth][index] = depth < size - 1;
        }
    }
    
    void Build() {
        for (auto& bits : paths_)
            bits.Build();
    }
    
    size_t size() const {
        return layers_[0].size();
    }
    
    size_t operator[](size_t index) const {
        size_t value = layers_[0][index];
        if (!paths_[0][index]) return value;
        auto shiftSize = layer_unit_sizes_[0];
        for (auto depth = 1; depth < num_layers_; depth++) {
            auto &bits = paths_[depth - 1];
            if (!bits[index]) break;
            index = bits.rank(index);
            value |= layers_[depth][index] << shiftSize;
            shiftSize += layer_unit_sizes_[depth];
        }
        return value;
    }
    
    /**
     Get number of splits of element at index.
     0 ~ max_splits
     */
    size_t depth(size_t index) const {
        size_t depth = 0;
        for (; depth < num_layers_ - 1; depth++) {
            if (!paths_[depth][index])
                break;
            index = paths_[depth].rank(index);
        }
        return depth;
    }
    
    void ShowStatus(std::ostream& os) const;
    
    size_t size_in_bytes() const {
        auto size = sizeof(num_layers_);
        size += sizeof(*layer_unit_sizes_) * num_layers_;
        for (auto i = 0; i < num_layers_; i++)
            size += layers_[i].size_in_bytes();
        if (num_layers_ > 0)
            for (auto i = 0; i == 0 || i + 1 < num_layers_; i++)
                size += paths_[i].size_in_bytes();
        
        return size;
    }
    
    void Write(std::ostream& os) const {
        write_val(num_layers_, os);
        for (auto i = 0; i < num_layers_; i++)
            write_val(layer_unit_sizes_[i], os);
        for (auto i = 0; i < num_layers_; i++)
            layers_[i].Write(os);
        if (num_layers_ > 0)
            for (auto i = 0; i == 0 || i + 1 < num_layers_; i++)
                paths_[i].Write(os);
    }
    
    void Read(std::istream& is) {
        num_layers_ = read_val<size_t>(is);
        for (auto i = 0; i < num_layers_; i++)
            layer_unit_sizes_[i] = read_val<size_t>(is);
        for (auto i = 0; i < num_layers_; i++)
            layers_[i] = FitVector(is);
        if (num_layers_ > 0)
            for (auto i = 0; i == 0 || i + 1 < num_layers_; i++)
                paths_[i].Read(is);
    }
    
private:
    
    void Expand_(size_t size) {
        if (num_layers_ >= size) return;
        for (auto i = num_layers_; i < size; i++)
            layers_[i] = FitVector(layer_unit_sizes_[i]);
        num_layers_ = size;
    }
    
    
public:
    // MARK: Copy guard
    
    DacVector() = default;
    ~DacVector() = default;
    
    DacVector(const DacVector&) = delete;
    DacVector& operator =(const DacVector&) = delete;
    
    DacVector(DacVector&& rhs) noexcept {
        num_layers_ = std::move(rhs.num_layers_);
        for (auto i = 0; i < kMaxSplits - 1; i++)
            paths_[i] = std::move(rhs.paths_[i]);
        for (auto i = 0; i < kMaxSplits; i++)
            layers_[i] = std::move(rhs.layers_[i]);
        for (auto i = 0; i < kMaxSplits; i++)
            layer_unit_sizes_[i] = std::move(rhs.layer_unit_sizes_[i]);
    }
    DacVector& operator =(DacVector&& rhs) noexcept = default;
    
};


void DacVector::ShowStatus(std::ostream &os) const {
    using std::endl;
    os << "--- Stat of " << "DACs " << " ---" << endl;
    os << "number of elements: " << size() << endl;
    auto size_bytes = size_in_bytes();
    os << "size:   " << size_bytes << endl;
    os << "sizeBits/element:   " << (float(size_bytes * 8) / size()) << endl;
    auto bitsSize = 0;
    for (auto i = 0; i == 0 || i + 1 < num_layers_; i++)
        bitsSize += paths_[i].size_in_bytes();
    os << "size bits:   " << bitsSize << endl;
    auto flowSize = 0;
    for (auto i = 0; i < num_layers_; i++)
        flowSize += layers_[i].size_in_bytes();
    os << "size flows:   " << flowSize << endl;
    os << "--- element map ---" << endl;
    os << "num_units: " << num_layers_ << endl;
    for (auto i = 0; i < num_layers_; i++)
        os << "[" << layer_unit_sizes_[i] << "]"<< endl;
}
    
} // namespace sim_ds

#endif /* DACs_hpp */
