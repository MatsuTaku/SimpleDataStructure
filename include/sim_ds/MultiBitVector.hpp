//
//  MultiBitVector.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/03/30.
//

#ifndef MultiBitVector_hpp
#define MultiBitVector_hpp

#include "basic.hpp"
#include "calc.hpp"
#include "log.hpp"
#include "bit_util.hpp"

namespace sim_ds {
    
template <unsigned int UnitSize>
class MultiBitVector {
    static_assert(UnitSize <= 3, "MultipleBitVector::UnitSize is overflow");
public:
    static constexpr size_t kBitsUnitSize = UnitSize;
    static constexpr uint8_t kMaxNumTypes = 1ULL << kBitsUnitSize;
    static constexpr id_type kBitsMask = kMaxNumTypes - 1;
    static constexpr size_t kBlockSize = 0x100 * kBitsUnitSize;
    static constexpr size_t kBitsInType = 0x40 - (0x40 % kBitsUnitSize);
    static constexpr size_t kBlockCapacity = kBitsInType * 4;
    static constexpr uint8_t kBitSize = 0x40;
    static constexpr uint8_t kBlocksInTipSize = kBlockSize / kBitSize;
    
private:
    std::vector<id_type> bits_;
    struct RankTip {
        id_type L1;
        uint8_t L2[kBlocksInTipSize];
    };
    std::vector<RankTip> rank_tips_[kMaxNumTypes];
    
    constexpr size_t block_(size_t index) const {
        return index / kBlockCapacity;
    }
    
    constexpr size_t abs_(size_t index) const {
        return index / (kBitsInType / kBitsUnitSize);
    }
    
    constexpr size_t rel_(size_t index) const {
        return index * kBitsUnitSize % kBitsInType;
    }
    
    constexpr uint8_t popcnt_(uint8_t bits, id_type word) const {
        if constexpr (kBitsUnitSize == 1) {
            if (bits == 0)
                return bit_util::popcnt(~word);
            else
                return bit_util::popcnt(word);
        } else if (kBitsUnitSize == 2) {
            if (bits == 0b00)
                return bit_util::popcnt11(~word);
            if (bits == 0b01)
                return bit_util::popcnt10(~word);
            if (bits == 0b10)
                return bit_util::popcnt10(word);
            else // bits == 0b11
                return bit_util::popcnt11(word);
        } else if (kBitsUnitSize == 3) {
            if (bits & 4)
                if (bits & 2)
                    if (bits & 1)
                        return bit_util::popcnt111(word);
                    else
                        return bit_util::popcnt110(word);
                else
                    if (bits & 1)
                        return bit_util::popcnt101(word);
                    else
                        return bit_util::popcnt100(word);
            else
                if (bits & 2)
                    if (bits & 1) // bits = 0b011
                        return bit_util::popcnt100(~word);
                    else          // bits = 0b010
                        return bit_util::popcnt101(~word);
                else
                    if (bits & 1) // bits = 0b001
                        return bit_util::popcnt110(~word);
                    else          // bits = 0b000
                        return bit_util::popcnt111(~word);
        }
    }
    
    constexpr uint8_t popcnt_(uint8_t bits, id_type word, size_t width) const {
        if constexpr (kBitsUnitSize == 1) {
            if (bits == 0)
                return bit_util::popcnt(~word & bit_util::WidthMask(width));
            else
                return bit_util::popcnt(word & bit_util::WidthMask(width));
        } else if (kBitsUnitSize == 2) {
            if (bits == 0b00)
                return bit_util::popcnt11(~word & bit_util::WidthMask(width));
            if (bits == 0b01)
                return bit_util::popcnt10(~word & bit_util::WidthMask(width));
            if (bits == 0b10)
                return bit_util::popcnt10(word & bit_util::WidthMask(width));
            else // bits == 0b11
                return bit_util::popcnt11(word & bit_util::WidthMask(width));
        } else if (kBitsUnitSize == 3) {
            if (bits & 4)
                if (bits & 2)
                    if (bits & 1)
                        return bit_util::popcnt111(word & bit_util::WidthMask(width));
                    else
                        return bit_util::popcnt110(word & bit_util::WidthMask(width));
                else
                    if (bits & 1)
                        return bit_util::popcnt101(word & bit_util::WidthMask(width));
                    else
                        return bit_util::popcnt100(word & bit_util::WidthMask(width));
            else
                if (bits & 2)
                    if (bits & 1) // bits = 0b011
                        return bit_util::popcnt100(~word & bit_util::WidthMask(width));
                    else          // bits = 0b010
                        return bit_util::popcnt101(~word & bit_util::WidthMask(width));
                else
                    if (bits & 1) // bits = 0b001
                        return bit_util::popcnt110(~word & bit_util::WidthMask(width));
                    else          // bits = 0b000
                        return bit_util::popcnt111(~word & bit_util::WidthMask(width));
        }
    }
    
public:
    MultiBitVector() = default;
    
    constexpr uint8_t operator[](size_t index) const {
        return bits_[abs_(index)] >> rel_(index) & kBitsMask;
    }
    
    constexpr unsigned long long rank(size_t index) const;
    
    // MARK: Setter
    
    void set(size_t index, uint8_t value) {
        assert(value <= kBitsMask);
        ResizeCheck(index);
        auto ri = rel_(index);
        auto &obj = bits_[abs_(index)];
        obj = (obj & ~(kBitsMask << ri)) | (id_type(value) << ri);
    }
    
    void build();
    
    void ResizeCheck(size_t index) {
        if (abs_(index) < bits_.size()) return;
        resize(index);
    }
    
    void resize(size_t index) {
        bits_.resize(abs_(index) + 1);
    }
    
    // MARK: - ByteData
    
    size_t size_in_bytes() const {
        auto size = size_vec(bits_);
        for (auto &tips : rank_tips_)
            size += size_vec(tips);
        return size;
    }
    
    void Read(std::istream &is) {
        read_vec(is, bits_);
        for (auto &tips : rank_tips_)
            read_vec(is, tips);
    }
    
    void Write(std::ostream &os) const {
        write_vec(bits_, os);
        for (auto &tips : rank_tips_)
            write_vec(tips, os);
    }
    
};

template <unsigned int S>
inline constexpr unsigned long long MultiBitVector<S>::rank(size_t index) const {
    auto type = (*this)[index];
    assert(type > 0);
    const auto& tip = rank_tips_[type - 1][block_(index)];
    auto abs = abs_(index);
    auto rel = rel_(index);
    if (rel == 0) {
        return tip.L1 + tip.L2[abs % kBlocksInTipSize];
    } else {
        return tip.L1 + tip.L2[abs % kBlocksInTipSize] + popcnt_(type, bits_[abs] & bit_util::WidthMask(rel));
    }
}

template <unsigned int S>
inline void MultiBitVector<S>::build() {
    if (bits_.size() == 0) return;
    
    auto num_types = 0;
    for (size_t i = 0, size = bits_.size() * kBitsInType / kBitsUnitSize; i < size; i++)
        num_types = std::max(num_types, (*this)[i] + 1);
    
    const auto tips_size = std::ceil(double(bits_.size()) / kBlocksInTipSize);
    // If bits == 0b00, don't make rank dict!
    for (auto type = 1; type < num_types; type++) {
        auto b = type - 1;
        auto &tips = rank_tips_[b];
        tips.resize(tips_size);
        size_t count = 0;
        for (auto i = 0; i < tips.size(); i++) {
            auto &tip = tips[i];
            tip.L1 = count;
            for (auto offset = 0; offset < kBlocksInTipSize; offset++) {
                tip.L2[offset] = count - tip.L1;
                auto index = i * kBlocksInTipSize + offset;
                if (index < bits_.size()) {
                    count += popcnt_(type, bits_[index]);
                }
            }
        }
    }
}
    
} // namespace sim_ds

#endif /* MultiBitVector_hpp */
