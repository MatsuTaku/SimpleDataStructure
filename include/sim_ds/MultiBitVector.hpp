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
#include "bit_tools.hpp"

namespace sim_ds {
    
    template <unsigned int UNIT_SIZE>
    class MultiBitVector {
    public:
        static constexpr size_t kBitsUnitSize = UNIT_SIZE;
        static constexpr uint8_t kMaxNumTypes = 1U << kBitsUnitSize;
        static constexpr id_type kBitsMask = kMaxNumTypes - 1;
        static constexpr size_t kBlockSize = 0x100 * kBitsUnitSize;
        static constexpr size_t kBitsInType = 0x40 - (0x40 % kBitsUnitSize);
        static constexpr size_t kBlockCapacity = kBitsInType * 4;
        static constexpr uint8_t kBitSize = 0x40;
        static constexpr uint8_t kBlocksInTipSize = kBlockSize / kBitSize;
        
        MultiBitVector() = default;
        ~MultiBitVector() = default;
        
        // MARK: - getter
        
        constexpr uint8_t operator[](size_t index) const {
            return bits_[abs_(index)] >> rel_(index) & kBitsMask;
        }
        
        constexpr unsigned long long rank(size_t index) const;
        
        // MARK: - setter
        
        void set(size_t index, uint8_t value) {
            assert(value <= kBitsMask);
            checkResize(index);
            auto ri = rel_(index);
            auto &obj = bits_[abs_(index)];
            obj = (obj & ~(kBitsMask << ri)) | (id_type(value) << ri);
        }
        
        void build();
        
        void checkResize(size_t index) {
            if (abs_(index) < bits_.size()) return;
            resize(index);
        }
        
        void resize(size_t index) {
            bits_.resize(abs_(index) + 1);
        }
        
        // MARK: - ByteData
        
        size_t sizeInBytes() const {
            auto size = size_vec(bits_);
            for (auto &tips : rank_tips_)
                size += size_vec(tips);
            return size;
        }
        
        void write(std::ostream &os) const {
            write_vec(bits_, os);
            for (auto &tips : rank_tips_)
                write_vec(tips, os);
        }
        
        void read(std::istream &is) {
            bits_ = read_vec<id_type>(is);
            for (auto &tips : rank_tips_)
                tips = read_vec<RankTip>(is);
        }
        
        // MARK: - Copy guard
        
        MultiBitVector(const MultiBitVector&) = delete;
        MultiBitVector& operator=(const MultiBitVector&) = delete;
        
        MultiBitVector(MultiBitVector&&) noexcept = default;
        MultiBitVector& operator=(MultiBitVector&&) noexcept = default;
        
    private:
        std::vector<id_type> bits_;
        struct RankTip {
            id_type L1;
            uint8_t L2[kBlocksInTipSize];
        };
        std::vector<RankTip> rank_tips_[kMaxNumTypes];
        
        // MARK: - private methods
        
        constexpr size_t block_(size_t index) const {
            return index / kBlockCapacity;
        }
        
        constexpr size_t abs_(size_t index) const {
            return index / (kBitsInType / kBitsUnitSize);
        }
        
        constexpr size_t rel_(size_t index) const {
            return index * kBitsUnitSize % kBitsInType;
        }
        
        constexpr uint8_t popCount_(uint8_t bits, id_type value) const {
            return bit_tools::popCount<kBitsUnitSize>(bits, value);
        }
        
    };
    
    template <unsigned int S>
    inline constexpr unsigned long long MultiBitVector<S>::rank(size_t index) const {
        auto type = (*this)[index];
        assert(type > 0);
        const auto &tip = rank_tips_[type - 1][block_(index)];
        return tip.L1 + tip.L2[abs_(index) % kBlocksInTipSize] + popCount_(type, bits_[abs_(index)] & bit_tools::maskOfBits(rel_(index)));
    }
    
    template <unsigned int S>
    inline void MultiBitVector<S>::build() {
        if (bits_.size() == 0) return;
        
        auto numTypes = 0;
        for (size_t i = 0, size = bits_.size() * kBitsInType / kBitsUnitSize; i < size; i++)
            numTypes = std::max(numTypes, (*this)[i] + 1);
        
        const auto tipsSize = std::ceil(float(bits_.size()) / kBlocksInTipSize);
        // If bits == 0b00, don't make rank dict!
        for (auto type = 1; type < numTypes; type++) {
            auto b = type - 1;
            auto &tips = rank_tips_[b];
            tips.resize(tipsSize);
            size_t count = 0;
            for (auto i = 0; i < tips.size(); i++) {
                auto &tip = tips[i];
                tip.L1 = count;
                for (auto offset = 0; offset < kBlocksInTipSize; offset++) {
                    tip.L2[offset] = count - tip.L1;
                    auto index = i * kBlocksInTipSize + offset;
                    if (index < bits_.size()) {
                        count += popCount_(type, bits_[index]);
                    }
                }
            }
        }
    }
    
}

#endif /* MultiBitVector_hpp */
