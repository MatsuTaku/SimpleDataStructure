//
//  DacVector.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2018/01/07.
//

#ifndef DacVector_hpp
#define DacVector_hpp

#include "basic.hpp"
#include "BitVector.hpp"
#include "SuccinctBitVector.hpp"
#include "FitVector.hpp"
#include "bit_util.hpp"
#include "calc.hpp"
#include <stdexcept>
#include <cstdlib>

namespace sim_ds {
    
    
template <class Sequence>
class IndexConstIterator {
public:
    using value_type = typename Sequence::value_type;
    using differencce_type = typename Sequence::difference_type;
    
private:
    const Sequence& source_;
    size_t pos_;
    
public:
    value_type operator*() const {
        return source_[pos_];
    }
    
    IndexConstIterator& operator++() {
        ++pos_;
        return *this;
    }
    
    IndexConstIterator operator++(int) {
        IndexConstIterator itr = *this;
        ++(*this);
        return itr;
    }
    
    IndexConstIterator& operator--() {
        --pos_;
        return *this;
    }
    
    IndexConstIterator operator--(int) {
        IndexConstIterator itr = *this;
        --(*this);
        return itr;
    }
    
    IndexConstIterator& operator+=(differencce_type distance) {
        pos_ += distance;
        return *this;
    }
    
    IndexConstIterator& operator-=(differencce_type distance) {
        return *this += -distance;
    }
    
    IndexConstIterator operator+(differencce_type distance) const {
        IndexConstIterator itr = *this;
        itr += distance;
        return itr;
    }
    
    IndexConstIterator operator-(differencce_type distance) const {
        IndexConstIterator itr = *this;
        itr -= distance;
        return itr;
    }
    
    friend IndexConstIterator operator+(long long distance, const IndexConstIterator& x) {return x + distance;}
    
    friend differencce_type operator-(const IndexConstIterator& x, const IndexConstIterator& y) {return x.pos_ - y.pos_;}
    
    value_type operator[](size_t pos) const {return *(*this + pos);}
    
    friend bool operator==(const IndexConstIterator& x, const IndexConstIterator& y) {return x.pos_ == y.pos_;}
    
    friend bool operator!=(const IndexConstIterator& x, const IndexConstIterator& y) {return !(x == y);}
    
    friend bool operator<(const IndexConstIterator& x, const IndexConstIterator& y) {return x.pos_ < y.pos_;}
    
    friend bool operator>(const IndexConstIterator& x, const IndexConstIterator& y) {return y < x;}
    
    friend bool operator<=(const IndexConstIterator& x, const IndexConstIterator& y) {return !(x > y);}
    
    friend bool operator>=(const IndexConstIterator& x, const IndexConstIterator& y) {return !(x < y);}
    
private:
    IndexConstIterator(const Sequence& source, size_t pos) : source_(source), pos_(pos) {}
    
    friend typename Sequence::Self;
    
};


class DacVector {
public:
    using Self = DacVector;
    using const_iterator = IndexConstIterator<DacVector>;
    using value_type = id_type;
    using difference_type = long long;
    
    using Layer = FitVector;
    using RankSupportBV = SuccinctBitVector<false>;
    
    static constexpr size_t kMaxSplits = 8;
    
    static std::string name() {
        return typeid(DacVector).name();
    }
    
private:
    size_t num_layers_ = 0;
    std::vector<size_t> layers_unit_bits_ = {8, 8, 8, 8, 8, 8, 8, 8};
    std::vector<Layer> layers_;
    std::vector<RankSupportBV> paths_;
    
public:
    DacVector() : num_layers_(0), layers_unit_bits_({8, 8, 8, 8, 8, 8, 8, 8}) {
        layers_.assign(8, Layer(size_t(8)));
        paths_.resize(7);
    }
    
    template <class Vector>
    DacVector(const Vector& vector) : DacVector(vector, std::vector<size_t>{}) {}
    
    template <class Vector, typename T>
    DacVector(const Vector& vector, std::vector<T> unit_bit_list) {
        // Empty vector input return with no works.
        if (vector.empty())
            return;
        
        if (unit_bit_list.empty()) {
            calc::split_positions_optimized_for_dac(vector, &layers_unit_bits_);
        } else {
            layers_unit_bits_.reserve(unit_bit_list.size());
            std::transform(unit_bit_list.begin(), unit_bit_list.end(), std::back_inserter(layers_unit_bits_), [](auto x) {return x;});
        }
        
        size_t num_layers = layers_unit_bits_.size();
        num_layers_ = num_layers;
        layers_.reserve(num_layers);
        std::transform(layers_unit_bits_.begin(), layers_unit_bits_.end(), std::back_inserter(layers_), [](auto unit) {return Layer(unit);});
        
        std::vector<BitVector> paths_src_(num_layers - 1);
        {
        std::vector<size_t> cf;
        calc::cummulative_frequency_list(vector, &cf);
        for (size_t i = 0, t = 0; i < num_layers; t += layers_unit_bits_[i], i++) {
            layers_[i].reserve(cf[t]);
            if (i < num_layers - 1)
                paths_src_[i].reserve(cf[t]);
        }
        }
        
        for (auto v : vector) {
            value_type x = v;
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
        
        paths_.reserve(paths_src_.size());
        std::transform(paths_src_.begin(), paths_src_.end(), std::back_inserter(paths_), [](auto&& src) {return RankSupportBV(src);});
    }
    
    DacVector(std::istream& is) {
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
    
    value_type operator[](size_t index) const {
        value_type value = layers_[0][index];
        for (size_t depth = 1, shift_bits = layers_unit_bits_[depth - 1], i = index;
             depth < num_layers();
             depth++, shift_bits += layers_unit_bits_[depth - 1])
        {
            auto& path = paths_[depth - 1];
            if (!path[i])
                break;
            i = path.rank(i);
            value_type unit = layers_[depth][i];
            value |= unit << shift_bits;
        }
        return value;
    }
    
    value_type at(size_t index) const {
        if (index >= size())
            throw std::out_of_range("Index out of range");
        
        return operator[](index);
    }
    
    const_iterator begin() const {
        return const_iterator(*this, 0);
    }
    
    const_iterator end() const {
        return const_iterator(*this, size());
    }
    
    value_type front() const {
        return operator[](0);
    }
    
    value_type back() const {
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
        auto size = sizeof(num_layers_);
        size += size_vec(layers_unit_bits_);
        for (size_t i = 0; i < num_layers(); i++)
            size += layers_[i].size_in_bytes();
        if (num_layers() > 0)
            for (size_t i = 0; i < num_layers() - 1; i++)
                size += paths_[i].size_in_bytes();
        
        return size;
    }
    
    void Read(std::istream& is) {
        layers_unit_bits_.resize(0);
        layers_.resize(0);
        paths_.resize(0);
        
        num_layers_ = read_val<size_t>(is);
        
        layers_unit_bits_.reserve(num_layers_);
        for (size_t i = 0; i < num_layers_; i++)
            layers_unit_bits_.push_back(read_val<size_t>(is));
        
        layers_.reserve(num_layers_);
        for (size_t i = 0; i < num_layers_; i++)
            layers_.push_back(Layer(is));
        
        if (num_layers_ > 1) {
            paths_.reserve(num_layers_ - 1);
            for (size_t i = 0; i < num_layers_ - 1; i++)
                paths_.push_back(RankSupportBV(is));
        }
    }
    
    void Write(std::ostream& os) const {
        write_val(num_layers(), os);
        for (auto& unit : layers_unit_bits_)
            write_val(unit, os);
        for (auto& layer : layers_)
            layer.Write(os);
        for (auto& path : paths_)
            path.Write(os);
    }
    
    void ShowStatus(std::ostream& os) const {
        using std::endl;
        os << "--- Stat of " << "DACs " << " ---" << endl;
        os << "number of elements: " << size() << endl;
        auto size_bytes = size_in_bytes();
        os << "size:   " << size_bytes << endl;
        os << "sizeBits/element:   " << (float(size_bytes * 8) / size()) << endl;
        auto bitsSize = 0;
        for (size_t i = 0; i == 0 || i + 1 < num_layers(); i++)
            bitsSize += paths_[i].size_in_bytes();
        os << "size bits:   " << bitsSize << endl;
        auto flowSize = 0;
        for (size_t i = 0; i < num_layers(); i++)
            flowSize += layers_[i].size_in_bytes();
        os << "size flows:   " << flowSize << endl;
        os << "--- element map ---" << endl;
        os << "num_units: " << num_layers() << endl;
        for (size_t i = 0; i < num_layers(); i++)
            os << "[" << layers_unit_bits_[i] << "]"<< endl;
    }
    
};

    
} // namespace sim_ds

#endif /* DACs_hpp */
