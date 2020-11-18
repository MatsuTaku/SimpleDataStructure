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
    explicit CDRWArray(size_t size=0) : size_(size) {
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


class CDRWVector {
private:
    size_t size_=0;
    size_t cap_=0;
    CDRWArray arr_;

public:
    explicit CDRWVector(size_t size=0) : size_(size), cap_(size), arr_(size) {}

    size_t size() const { return size_; }

    size_t capacity() const { return cap_; }

    uint64_t get(size_t i) const {
        return arr_.get(i);
    }

    void set(size_t i, uint64_t value) {
        arr_.set(i, value);
    }

    void resize(size_t new_size) {
        if (new_size > cap_) {
            _allocate(std::max(new_size, cap_*2));
        }
        size_ = new_size;
    }

    void reserve(size_t reserved_size) {
        if (reserved_size <= cap_)
            return;
        _allocate(std::max(reserved_size, cap_*2));
    }

    void shrink_to_fit() {
        _allocate(size());
    }

    void push_back(uint64_t value) {
        if (size() == cap_)
            _allocate(size()*2);
        size_++;
        set(size()-1, value);
    }

    void pop_back() {
        if (size() == 0)
            return;
        if (size()*3 < cap_)
            _allocate(size()*2);
        size_--;
    }

    void clear() {
        size_ = 0;
    }

    bool empty() const { return size() == 0; }

private:
    void _allocate(size_t new_cap) {
        assert(new_cap >= size());
        assert(bit_util::popcnt(new_cap) == 1);
        CDRWArray next(new_cap);
        for (size_t i = 0; i < size(); i++)
            next.set(i, get(i));
        arr_ = next;
        cap_ = new_cap;
    }

};

}

#endif //SIDS_INCLUDE_SIM_DS_CDRWARRAY_HPP_
