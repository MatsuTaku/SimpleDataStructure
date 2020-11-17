#ifndef SIDS_INCLUDE_SIM_DS_CHT_H_
#define SIDS_INCLUDE_SIM_DS_CHT_H_

#include "BijectiveHash.hpp"
#include "FitVector.hpp"
#include "bit_util.hpp"

#include <bitset>

namespace sim_ds {

template <
    unsigned ValueBits,
    unsigned MaxLoadFactorPercent = 50,
    typename BijectiveHash = SplitMixHash>
class CHT {
    static_assert(MaxLoadFactorPercent < 100);
public:
    static constexpr size_t kDefaultBucketSize = 8;
    static constexpr unsigned kFlagBits = 3;
    static constexpr uint64_t kOccupiedMask = 1u << 0;
    static constexpr uint64_t kContinuationMask = 1u << 1;
    static constexpr uint64_t kShiftedMask = 1u << 2;

private:
    unsigned key_bits_;
    size_t bucket_size_;
    unsigned bucket_bits_;
    uint64_t bucket_mask_;
    size_t max_size_;
    size_t min_size_;
    BijectiveHash hasher_;
    FitVector arr_;
    FitVector values_;
    uint64_t quo_mask_;
    size_t size_ = 0;
    size_t used_ = 0;

    bool IsOccupied(size_t i) const { return arr_[i] & kOccupiedMask; }
    bool IsContinuation(size_t i) const { return arr_[i] & kContinuationMask; }
    bool IsShifted(size_t i) const { return arr_[i] & kShiftedMask; }
    bool IsNull(size_t i) const { return (arr_[i] % (1ull<<kFlagBits)) == 0; }

    uint64_t Quotient(size_t i) const { return (arr_[i] & quo_mask_) >> kFlagBits; }

public:
    CHT(unsigned key_bits, size_t bucket_size = kDefaultBucketSize) :
        key_bits_(key_bits),
        bucket_size_(bucket_size),
        bucket_bits_(bit_util::ctz(bucket_size)),
        bucket_mask_((1ull<<bucket_bits_)-1),
        max_size_(bucket_size*MaxLoadFactorPercent/100),
        min_size_(bucket_size*(MaxLoadFactorPercent-1)/(MaxLoadFactorPercent+100)+1),
        values_(ValueBits, bucket_size_),
        hasher_(key_bits_) {
        assert(bit_util::popcnt(bucket_size) == 1);
        unsigned quo_bits = std::max(0, (int)key_bits - (int)bucket_bits_);
        arr_ = FitVector(quo_bits + kFlagBits, bucket_size_);
        quo_mask_ = ((1ull << quo_bits)-1) << kFlagBits;
    }

    size_t ToClusterHead(size_t i) const {
        if (!IsShifted(i))
            return i;
        size_t t = 0;
        do {
            i = pred(i);
            if (IsOccupied(i))
                ++t;
        } while (IsShifted(i));
        while (t) {
            i = succ(i);
            if (!IsContinuation(i))
                --t;
        }
        return i;
    }

    std::pair<bool, uint64_t> get(uint64_t key) const {
        assert(64-bit_util::clz(key) <= key_bits_);

        uint64_t initial_h = hasher_.hash(key);
        auto quo = initial_h >> bucket_bits_;
        size_t i = initial_h & bucket_mask_;
        if (!IsOccupied(i))
            return {false, 0};
        i = ToClusterHead(i);
        do {
            if (Quotient(i) == quo) {
                return {true, values_[i]};
            }
            i = succ(i);
        } while (IsContinuation(i));
        return {false, 0};
    }

    bool IsFilled() const {
        return used_ == max_size_;
    }

    void set(size_t key, uint64_t value) {
        assert(64-bit_util::clz(key) <= key_bits_);
        assert(64-bit_util::clz(value) <= ValueBits);

        if (IsFilled()) {
            reserve(size()*2);
        }

        uint64_t initial_h = hasher_.hash(key);
        auto quo = initial_h >> bucket_bits_;
        size_t initial_i = initial_h & bucket_mask_;
        auto i = ToClusterHead(initial_i);
        if (IsOccupied(initial_i)) {
            do {
                if (Quotient(i) == quo) {
                    values_[i] = value;
                    return;
                }
                i = succ(i);
            } while (IsContinuation(i));
        }
        if (!IsNull(i)) {
            auto j = i;
            do {
                j = succ(j);
            } while (!IsNull(j));
            do {
                auto pj = pred(j);
                arr_[j] = (
                    (arr_[j] & kOccupiedMask) |
                    (arr_[pj] & (quo_mask_|kContinuationMask)) |
                    kShiftedMask
                );
                values_[j] = values_[pj];
                j = pj;
            } while (i != j);
            assert(j == i);
        }
        if (!IsOccupied(initial_i)) { // New cluster
            arr_[initial_i] = arr_[initial_i] | kOccupiedMask;
            arr_[i] = (
                (arr_[i] & kOccupiedMask) |
                (quo << kFlagBits) |
                (i != initial_i ? kShiftedMask : 0)
            );
        } else { // Already exists cluster
            arr_[i] = (
                (arr_[i] & kOccupiedMask) |
                (quo << kFlagBits) |
                kContinuationMask |
                kShiftedMask
            );
        }
        ++size_;
        ++used_;
        values_[i] = value;
    }

    size_t succ(size_t i) const {
        i++;
        [[unlikely]] if (i == bucket())
            i = 0;
        return i;
    }

    size_t pred(size_t i) const {
        [[unlikely]] if (i == 0)
            return bucket()-1;
        else
            return i-1;
    }

    void print_for_debug() const {
        int cnt=0;
        for (int i = 0; i < bucket(); i++) {
            std::cout << i << "] "
                      << bool(arr_[i]&kOccupiedMask)
                      << bool(arr_[i]&kContinuationMask)
                      << bool(arr_[i]&kShiftedMask)
                      << std::endl;
            if (IsShifted(i))
                ++cnt;
        }
        std::cout<<"cnt shifted: "<<cnt<<std::endl;
    }

    void reserve(size_t _size) {
        if (_size <= size())
            return;
        auto need_size = _size * (100-1)/MaxLoadFactorPercent+1;
        auto new_bucket_bits = 64-bit_util::clz(need_size-1);
        _resize(1ull << new_bucket_bits);
    }
    
    size_t size() const {return size_;}
    size_t bucket() const {return bucket_size_;}

private:
    void _resize(size_t new_bucket_size) {
        CHT next(key_bits_, new_bucket_size);
        size_t i = 0;
        size_t cnt = 0;
        while (cnt < size()) {
            while (!(IsOccupied(i) and !IsShifted(i))) {
                i = succ(i);
            }
            std::queue<uint64_t> qs;
            uint64_t f;
            do {
                if (IsOccupied(i)) {
                    qs.push(i);
                }
                if (!IsContinuation(i)) {
                    assert(!qs.empty());
                    f = qs.front();
                    qs.pop();
                }
                uint64_t x = (Quotient(i) << bucket_bits_) | f;
                x = hasher_.ihash(x);
                next.set(x, values_[i]);
                cnt++;
                i = succ(i);
            } while (IsShifted(i));
        }
        *this = next;
    }

};

}

#endif //SIDS_INCLUDE_SIM_DS_CHT_H_
