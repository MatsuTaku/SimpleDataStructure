//
//  SACs.hpp
//  sim_ds
//
//  Created by 松本拓真 on 2018/01/20.
//

#ifndef SACs_hpp
#define SACs_hpp

#include "basic.hpp"
#include "Calc.hpp"
#include "MultiBitVector.hpp"
#include "Vector.hpp"
#include "WaveletTree.hpp"

namespace sim_ds {
    
    template <class SEQUENCES>
    class SACs {
    public:
        using idType = uint64_t;
        static constexpr size_t kMaxNumSeparates = 8;
        
        // MARK: Constructor
        
        SACs() = default;
        ~SACs() = default;
        
        SACs(const SACs&) = delete;
        SACs& operator =(const SACs&) = delete;
        
        SACs(SACs&& rhs) noexcept {
            size_ = std::move(rhs.size_);
            num_units_ = std::move(rhs.num_units_);
            sequences_ = std::move(rhs.sequences_);
            for (auto i = 0; i < kMaxNumSeparates; i++) {
                units_[i] = std::move(rhs.units_[i]);
                bits_sizes_[i] = std::move(rhs.bits_sizes_[i]);
            }
        }
        SACs& operator=(SACs&& rhs) noexcept = default;
        
        SACs(std::istream &is) {
            read(is);
        }
        
        template<typename T>
        SACs(const std::vector<T> &vector, size_t minCost = 1, size_t maxLevels = 8) {
            setFromVector(vector, Calc::optimizedBitsListForDac(vector, minCost, maxLevels));
        }
        SACs(const Vector &vector, size_t minCost = 1, size_t maxLevels = 8) {
            std::vector<size_t> vec(vector.size());
            for (auto i = 0; i < vector.size(); i++)
                vec[i] = vector[i];
            setFromVector(vector, Calc::optimizedBitsListForDac(vec, minCost, maxLevels));
        }
        template<typename C>
        SACs(const C &vector, const std::vector<size_t> &sizes) {
            setFromVector(vector, sizes);
        }
        
        // MARK: Function
        
        static std::string name() {
            return "SACs";
        }
        
        template<typename C>
        void setFromVector(const C &vector, const std::vector<size_t> &sizes) {
            for (auto i = 0; i < sizes.size(); i++)
                bits_sizes_[i] = sizes[i];
            expand(sizes.size());
            for (auto i = 0; i < vector.size(); i++)
                setValue(i, vector[i]);
            build();
        }
        
        void expand(size_t size) {
            if (num_units_ >= size) return;
            auto bitsCount = 0;
            for (auto i = 0; i < size; i++) {
                if (i > 0)
                    bitsCount += bits_sizes_[i];
                if (i >= num_units_)
                    units_[i] = Vector(i == 0 ? bits_sizes_[0] : bitsCount);
            }
            num_units_ = size;
        }
        
        void build() {
            sequences_.build();
        }
        
        size_t size() const {
            return size_;
        }
        
        size_t operator[](size_t index) const;
        
        void setValue(size_t index, size_t value);
        
        void showStatus(std::ostream &os) const;
        
        size_t sizeInBytes() const {
            auto size = sizeof(num_units_) + sizeof(size_);
            for (auto &unit : units_)
                size += unit.sizeInBytes();
            size += sequences_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream &os) const {
            write_val(size_, os);
            sequences_.write(os);
            write_val(num_units_, os);
            for (auto i = 0; i < kMaxNumSeparates; i++)
                units_[i].write(os);
            for (auto i = 0; i < kMaxNumSeparates; i++)
                write_val(bits_sizes_[i], os);
        }
        
        void read(std::istream &is) {
            size_ = read_val<size_t>(is);
            sequences_.read(is);
            num_units_ = read_val<size_t>(is);
            for (auto i = 0; i < kMaxNumSeparates; i++)
                units_[i] = Vector(is);
            for (auto i = 0; i < kMaxNumSeparates; i++)
                bits_sizes_[i] = read_val<size_t>(is);
        }
        
    private:
        size_t size_ = 0;
        size_t num_units_ = 0;
        SEQUENCES sequences_;
        Vector units_[kMaxNumSeparates];
        size_t bits_sizes_[kMaxNumSeparates] = {8, 8, 8, 8, 8, 8, 8, 8};
        
    };
    
    using SACs2 = SACs<MultiBitVector<1>>;
    using SACs4 = SACs<MultiBitVector<2>>;
    using SACs8 = SACs<MultiBitVector<3>>;
    using SACsForLCP = SACs8;
    using SACsWV = SACs<WaveletTree>;
    
    // MARK: - inline function
    
    template <class C>
    inline size_t SACs<C>::operator[](size_t index) const {
        size_t value = units_[0][index];
        auto type = sequences_[index];
        if (type > 0)
            value |= (units_[type][sequences_.rank(index)] << bits_sizes_[0]);
        return value;
    }
    
    template <class C>
    inline void SACs<C>::setValue(size_t index, size_t value) {
        size_ = std::max(size_, index + 1);
        
        auto size = Calc::sizeFitAsSizeList(value, bits_sizes_);
        if (size > num_units_)
            expand(size);
        units_[0].push_back(value & ((1U << bits_sizes_[0]) - 1));
        sequences_.set(index, size - 1);
        if (size > 1)
            units_[size - 1].push_back(value >> bits_sizes_[0]);
    }
    
    template <class C>
    void SACs<C>::showStatus(std::ostream &os) const {
        using std::endl;
        os << "---- Stat of " << "SACs " << " ----" << endl;
        os << "number of elements: " << size_ << endl;
        auto size = sizeInBytes();
        os << "size:   " << size << endl;
        os << "sizeBits/element:   " << float(size * 8) / size_ << endl;
        os << "size multi_bits:   " << sequences_.sizeInBytes() << endl;
        auto sizeFlow = 0;
        for (auto &u : units_)
            sizeFlow += u.sizeInBytes();;
        os << "size flows:   " << sizeFlow << endl;
        os << "--- element map ---" << endl;
        os << "num_units: " << num_units_ << endl;
        for (auto i = 0; i < num_units_; i++)
            os << "[" << bits_sizes_[i] << "]"<< endl;
    }
    
}

#endif /* SACs_hpp */
