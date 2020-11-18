#ifndef SIDS_INCLUDE_SIM_DS_CDRWARRAY_HPP_
#define SIDS_INCLUDE_SIM_DS_CDRWARRAY_HPP_

#include "bit_util.hpp"
#include "FitVector.hpp"
#include "CHT.hpp"

namespace sim_ds {

class CDRWArray {
private:
    size_t size_;
    FitVector widthes_;
    std::array<CHT<0>, 64> sat_table_{};

public:
    explicit CDRWArray(size_t size) : size_(size) {
        if (size == 0) return;
        auto size_bits = 64-bit_util::clz((uint64_t)size-1);
        widthes_ = FitVector(7, size);
        for (int i = 0; i < 64; i++) {
            sat_table_[i] = CHT<0>(size_bits);
            sat_table_[i].set_value_width(i+1);
        }
    }

    template<typename Iter>
    explicit CDRWArray(Iter begin, Iter end) : CDRWArray(end-begin) {
        for (auto it = begin; it != end; ++it) {
            set(it-begin, *it);
        }
    }

    uint64_t get(size_t i) const {
        auto w = widthes_[i];
        if (w == 0)
            return 0;
        else {
            auto [s, v] = sat_table_[w-1].get(i);
            assert(s);
            return v;
        }
    }

    void set(size_t i, uint64_t value) {
        auto value_bits = 64-bit_util::clz((uint64_t)value);
        auto pw = widthes_[i];
        if (pw != 0 and pw != value_bits)
            sat_table_[pw].erase(i);
        widthes_[i] = value_bits;
        if (value_bits > 0)
            sat_table_[value_bits-1].set(i, value);
    }

    size_t size() const { return size_; }

};

}

#endif //SIDS_INCLUDE_SIM_DS_CDRWARRAY_HPP_
