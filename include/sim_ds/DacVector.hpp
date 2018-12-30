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
#include "SuccinctBitVector.hpp"
#include "FitVector.hpp"
#include "calc.hpp"

namespace sim_ds {


class DacVector {
public:
    using layer_v = FitVector;
    using path_bv = SuccinctBitVector<false>;
    
    static constexpr size_t kMaxSplits = 8;
    
    static std::string name() {
        return typeid(DacVector).name();
    }
    
private:
    size_t num_layers_ = 0;
    size_t layers_unit_bits_[kMaxSplits] = {8, 8, 8, 8, 8, 8, 8, 8};
    layer_v layers_[kMaxSplits];
    path_bv paths_[kMaxSplits - 1];
    
public:
    DacVector() = default;
    
    template<class Seq>
    DacVector(const Seq& vector) : DacVector(vector, calc::split_positions_optimized_for_dac(vector)) {}
    
    template<class T, class UInt>
    DacVector(const std::vector<T>& vector, std::vector<UInt> unit_bit_list_) {
        // Empty vector input return with no works.
        if (vector.empty())
            return;
        
        if (unit_bit_list_.empty())
            unit_bit_list_ = calc::split_positions_optimized_for_dac(vector);
        for (auto i = 0; i < unit_bit_list_.size(); i++)
            layers_unit_bits_[i] = unit_bit_list_[i];
        
        size_t num_layers = unit_bit_list_.size();
        for (auto i = 0; i < num_layers; i++)
            layers_[i] = FitVector(layers_unit_bits_[i]);
        num_layers_ = num_layers;
        
        std::vector<BitVector> paths_src_(num_layers - 1);
        
        std::vector<size_t> cf = calc::cummulative_frequency_list(vector);
        for (auto i = 0, t = 0; i < num_layers; t += unit_bit_list_[i], i++) {
            layers_[i].reserve(cf[t]);
            if (i < num_layers - 1)
                paths_src_[i].reserve(cf[t]);
        }
        
        for (id_type v : vector) {
            id_type x = v;
            layers_[0].push_back(x & bit_util::WidthMask(layers_unit_bits_[0]));
            x >>= layers_unit_bits_[0];
            for (size_t depth = 1; depth < num_layers; depth++) {
                bool exist = x > 0;
                paths_src_[depth - 1].push_back(exist);
                if (!exist)
                    break;
                auto unit_bits = layers_unit_bits_[depth];
                layers_[depth].push_back(x & bit_util::WidthMask(unit_bits));
                x >>= unit_bits;
            }
        }
        
        for (size_t i = 0; i < paths_src_.size(); i++) {
            paths_[i] = path_bv(paths_src_[i]);
        }
    }
    
    DacVector(std::istream &is) {
        Read(is);
    }
    
    // MARK: getter
    
    size_t size() const {
        return layers_[0].size();
    }
    
    bool empty() const {
        return size() == 0;
    }
    
    size_t num_layers() const {
        return num_layers_;
    }
    
    id_type operator[](size_t index) const {
        id_type value = layers_[0][index];
        for (size_t depth = 1, shift_bits = layers_unit_bits_[depth - 1], i = index;
             depth < num_layers();
             depth++, shift_bits += layers_unit_bits_[depth - 1]) {
            auto& path = paths_[depth - 1];
            if (!path[i])
                break;
            i = path.rank(i);
            id_type unit = layers_[depth][i];
            value |= unit << shift_bits;
        }
        return value;
    }
    
    id_type front() const {
        return operator[](0);
    }
    
    id_type back() const {
        return operator[](size() - 1);
    }
    
    /**
     Get number of splits of element at index.
     0 ~ max_splits
     */
    size_t depth(size_t index) const {
        size_t depth = 0;
        for (; depth < num_layers() - 1; depth++) {
            if (!paths_[depth][index])
                break;
            index = paths_[depth].rank(index);
        }
        return depth;
    }
    
    size_t size_in_bytes() const {
        auto size = sizeof(num_layers());
        size += sizeof(*layers_unit_bits_) * num_layers();
        for (auto i = 0; i < num_layers(); i++)
            size += layers_[i].size_in_bytes();
        if (num_layers() > 0)
            for (auto i = 0; i == 0 || i + 1 < num_layers(); i++)
                size += paths_[i].size_in_bytes();
        
        return size;
    }
    
    void Read(std::istream& is) {
        num_layers_ = read_val<size_t>(is);
        for (auto i = 0; i < num_layers(); i++)
            layers_unit_bits_[i] = read_val<size_t>(is);
        for (auto i = 0; i < num_layers(); i++)
            layers_[i] = FitVector(is);
        if (num_layers() > 0)
            for (auto i = 0; i == 0 || i + 1 < num_layers(); i++)
                paths_[i].Read(is);
    }
    
    void Write(std::ostream& os) const {
        write_val(num_layers(), os);
        for (auto i = 0; i < num_layers(); i++)
            write_val(layers_unit_bits_[i], os);
        for (auto i = 0; i < num_layers(); i++)
            layers_[i].Write(os);
        if (num_layers() > 0)
            for (auto i = 0; i == 0 || i + 1 < num_layers(); i++)
                paths_[i].Write(os);
    }
    
    void ShowStatus(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << "DACs " << " ---" << endl;
        os << "number of elements: " << size() << endl;
        auto size_bytes = size_in_bytes();
        os << "size:   " << size_bytes << endl;
        os << "sizeBits/element:   " << (float(size_bytes * 8) / size()) << endl;
        auto bitsSize = 0;
        for (auto i = 0; i == 0 || i + 1 < num_layers(); i++)
            bitsSize += paths_[i].size_in_bytes();
        os << "size bits:   " << bitsSize << endl;
        auto flowSize = 0;
        for (auto i = 0; i < num_layers(); i++)
            flowSize += layers_[i].size_in_bytes();
        os << "size flows:   " << flowSize << endl;
        os << "--- element map ---" << endl;
        os << "num_units: " << num_layers() << endl;
        for (auto i = 0; i < num_layers(); i++)
            os << "[" << layers_unit_bits_[i] << "]"<< endl;
    }
    
};

    
} // namespace sim_ds

#endif /* DACs_hpp */
