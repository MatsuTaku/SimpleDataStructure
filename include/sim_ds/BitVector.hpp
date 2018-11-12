//
//  VitVector.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/10/30.
//

#ifndef VitVector_hpp
#define VitVector_hpp

#include "basic.hpp"
#include "log.hpp"
#include "bit_tools.hpp"
#include "FitVector.hpp"
#include "calc.hpp"

namespace sim_ds {
    
    template<class _Seq>
    class __bit_reference {
    private:
        using __pointer_type = typename _Seq::__pointer_type;
        using __mask_type = typename _Seq::__word_type;
        
    public:
        __bit_reference(__pointer_type p, __mask_type m) : __pointer_(p), __mask_(m) {}
        
        constexpr operator bool() const {
            return static_cast<bool>(*__pointer_ & __mask_);
        }
        
        __bit_reference& operator=(bool x) {
            if (x)
                *__pointer_ |= __mask_;
            else
                *__pointer_ &= ~__mask_;
            return *this;
        }
        
    private:
        __pointer_type __pointer_;
        __mask_type __mask_;
        
    };
    
    
    class BitVector {
    private:
        friend class __bit_reference<BitVector>;
        using __pointer_type = id_type*;
        using __word_type = id_type;
        
    public:
        using reference = __bit_reference<BitVector>;
        
        using block_type = uint8_t;
        static constexpr uint8_t kSBlockSize = sizeof(id_type) * 8; // 64
        static constexpr size_t kLBlockSize = 0x100;
        static constexpr uint8_t kBlocksInTip = kLBlockSize / kSBlockSize; // (256) / 64 = 4
        static constexpr size_t kNum1sPerTip = 0x200;
        
        BitVector() = default;
        ~BitVector() = default;
        
        template <class BITSET>
        BitVector(const BITSET& bools, bool useRank, bool useSelect = false) {
            resize(bools.size());
            for (auto i = 0; i < bools.size(); i++)
                operator[](i) = bools[i];
            if (useRank)
                build(useSelect);
        }
        
        BitVector(const BitVector& rhs) noexcept  {
            size_ = rhs.size_;
            bits_ = rhs.bits_;
            l_blocks_ = rhs.l_blocks_;
            s_block_units_ = rhs.s_block_units_;
        }
        
        BitVector& operator=(const BitVector& rhs) noexcept {
            size_ = rhs.size_;
            bits_ = rhs.bits_;
            l_blocks_ = rhs.l_blocks_;
            s_block_units_ = rhs.s_block_units_;
            
            return *this;
        }
        
        BitVector(BitVector&& rhs) noexcept {
            size_ = std::move(rhs.size_);
            bits_ = std::move(rhs.bits_);
            l_blocks_ = std::move(rhs.l_blocks_);
            s_block_units_ = std::move(rhs.s_block_units_);
        }
        
        BitVector& operator=(BitVector&& rhs) noexcept {
            size_ = std::move(rhs.size_);
            bits_ = std::move(rhs.bits_);
            l_blocks_ = std::move(rhs.l_blocks_);
            s_block_units_ = std::move(rhs.s_block_units_);
            return *this;
        }
        
        bool operator[](size_t index) const {
            return static_cast<bool>(bits_[abs_(index)] & (1UL << rel_(index)));
        }
        
        reference operator[](size_t index) {
            return reference(&bits_[abs_(index)], 1ULL << rel_(index));
        }
        
        unsigned long long rank(size_t index) const {
            return tipL_(index) + tipS_(index) + bit_tools::popCount(bits_[abs_(index)] & ((1UL << rel_(index)) - 1));
        }
        
        unsigned long long rank0(size_t index) const {
            return index - rank(index);
        }
        
        unsigned long long select(size_t index) const;
        
        size_t size() const {
            return size_;
        }
        
        void push_back(bool bit) {
            auto size = size_;
            resize(size_ + 1);
            operator[](size) = bit;
        }
        
        void build(bool useSelect);
        
        void buildRank();
        
        void buildSelect();
        
        void resize(size_t size) {
            size_t abs = std::ceil(float(size) / kSBlockSize);
            bits_.resize(abs);
            size_ = size;
        }
        
        void reserve(size_t size) {
            size_t abs = std::ceil(float(size) / kSBlockSize);
            bits_.reserve(abs);
        }
        
        void swapBits(BitVector &rhs) {
            std::swap(size_, rhs.size_);
            bits_.swap(rhs.bits_);
        }
        
        size_t sizeInBytes() const {
            return sizeof(size_t) + size_vec(bits_) + l_blocks_.sizeInBytes() + size_vec(s_block_units_);
        }
        
        void write(std::ostream& os) const {
            write_val(size_, os);
            write_vec(bits_, os);
            l_blocks_.write(os);
            write_vec(s_block_units_, os);
        }
        void read(std::istream& is) {
            size_ = read_val<size_t>(is);
            bits_ = read_vec<id_type>(is);
            l_blocks_ = FitVector(is);
            s_block_units_ = read_vec<uint8_t>(is);
        }
        
    private:
        size_t size_ = 0;
        std::vector<id_type> bits_;
        FitVector l_blocks_;
        std::vector<block_type> s_block_units_;
        std::vector<id_type> select_tips_;
        
        constexpr size_t block_(size_t index) const {
            return index / kLBlockSize;
        }
        
        constexpr size_t abs_(size_t index) const {
            return index / kSBlockSize;
        }
        
        constexpr size_t rel_(size_t index) const {
            return index % kSBlockSize;
        }
        
        size_t tipL_(size_t index) const {
            return l_blocks_[block_(index)];
        }
        
        size_t tipS_(size_t index) const {
            return s_block_units_[abs_(index)];
        }
        
    };
    
    
    
    unsigned long long BitVector::select(size_t index) const {
        id_type left = 0, right = l_blocks_.size();
        auto i = index;
        
        if (select_tips_.size() != 0) {
            auto selectTipId = i / kNum1sPerTip;
            left = select_tips_[selectTipId];
            right = select_tips_[selectTipId + 1] + 1;
        }
        
        while (left + 1 < right) {
            const auto center = (left + right) / 2;
            if (i < l_blocks_[center]) {
                right = center;
            } else {
                left = center;
            }
        }
        
        i += 1; // for i+1 th
        i -= l_blocks_[left];
        
        uint32_t offset = 1;
        for (; offset < kBlocksInTip; offset++) {
            if (i <= s_block_units_[left * kBlocksInTip + offset]) {
                break;
            }
        }
        i -= s_block_units_[left * kBlocksInTip + --offset];
    
        auto ret = (left * kLBlockSize) + (offset * kSBlockSize);
        auto bits = bits_[ret / 0x20];
        
        {
            auto count = bit_tools::popCount(bits % 0x10000);
            auto shiftBlock = 0;
            while ((0x10000 - 1) >> (8 * ++shiftBlock));
            shiftBlock *= 8;
            if (count < i) {
                bits >>= shiftBlock;
                ret += shiftBlock;
                i -= count;
            }
        }
        {
            auto count = bit_tools::popCount(bits % 0x100);
            auto shiftBlock = 0;
            while ((0x100 - 1) >> (8 * ++shiftBlock));
            shiftBlock *= 8;
            if (count < i) {
                bits >>= shiftBlock;
                ret += shiftBlock;
                i -= count;
            }
        }
        
        ret += bit_tools::kSelectTable[i][bits % 0x100];
        return ret - 1;
    }
    
    
    void BitVector::build(bool useSelect = false) {
        if (bits_.empty()) return;
        
        buildRank();
        
        if (useSelect) buildSelect();
        
    }
    
    
    void BitVector::buildRank() {
        l_blocks_ = FitVector(calc::sizeFitInBits(size_), std::ceil(float(size_) / kLBlockSize));
        s_block_units_.resize(std::ceil(float(size_) / kSBlockSize));
        
        auto count = 0;
        for (auto i = 0; i < l_blocks_.size(); i++) {
            l_blocks_[i] = count;
            
            for (auto offset = 0; offset < kBlocksInTip; offset++) {
                auto index = i * kBlocksInTip + offset;
                if (index > s_block_units_.size() - 1)
                    break;
                s_block_units_[index] = count - l_blocks_[i];
                if (index < bits_.size()) {
                    count += bit_tools::popCount(bits_[index]);
                }
            }
            
        }
        
    }
    
    
    void BitVector::buildSelect() {
        auto count = kNum1sPerTip;
        select_tips_.emplace_back(0);
        for (id_type i = 0; i < l_blocks_.size(); i++) {
            if (count < l_blocks_[i]) {
                select_tips_.emplace_back(i - 1);
                count += kNum1sPerTip;
            }
        }
        
        select_tips_.emplace_back(l_blocks_.size() - 1);
    }
    
}

#endif /* VitVector_hpp */
