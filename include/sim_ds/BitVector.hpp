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
    
    template<class SequenceType>
    class BitReference {
        using pointer = typename SequenceType::pointer;
        using mask_type = typename SequenceType::storage_type;
        
        friend typename SequenceType::self;
        
    public:
        constexpr operator bool() const {
            return static_cast<bool>(*pointer_ & mask_);
        }
        
        BitReference& operator=(bool x) {
            if (x)
                *pointer_ |= mask_;
            else
                *pointer_ &= ~mask_;
            return *this;
        }
        
    private:
        constexpr BitReference(pointer p, mask_type m) noexcept : pointer_(p), mask_(m) {}
        
        pointer pointer_;
        mask_type mask_;
        
    };
    
    
    template<class SequenceType>
    class BitConstReference {
        using pointer = typename SequenceType::const_pointer;
        using mask_type = typename SequenceType::storage_type;
        
        friend typename SequenceType::self;
        
    public:
        constexpr operator bool() const {
            return static_cast<bool>(*pointer_ & mask_);
        }
        
    private:
        constexpr BitConstReference(pointer p, mask_type m) noexcept : pointer_(p), mask_(m) {}
        
        pointer pointer_;
        mask_type mask_;
        
    };
    
    
    class BitVector {
        using self = BitVector;
        using storage_type = id_type;
        using pointer = storage_type*;
        using const_pointer = const storage_type*;
        
        using s_block_type = uint8_t;
        
        static constexpr uint8_t kSmallBlockBits = sizeof(id_type) * 8; // 64
        static constexpr size_t kLargeBlockBits = 0x100;
        static constexpr uint8_t kNumSPerL = kLargeBlockBits / kSmallBlockBits; // (256) / 64 = 4
        static constexpr size_t kBitsPerSelectTips = 0x200;
        
        size_t size_ = 0;
        std::vector<storage_type> bits_;
        FitVector l_blocks_;
        std::vector<s_block_type> s_blocks_;
        FitVector select_tips_;
        
        friend class BitReference<BitVector>;
        friend class BitConstReference<BitVector>;
        
        using reference = BitReference<BitVector>;
        using const_reference = BitConstReference<BitVector>;
        
    public:
        template <class _BitSet>
        BitVector(const _BitSet& bools, bool useRank, bool useSelect = false) {
            resize(bools.size());
            for (auto i = 0; i < bools.size(); i++)
                operator[](i) = bools[i];
            if (useRank)
                build(useSelect);
        }
        
        reference operator[](size_t index) {
            assert(index < size_);
            return reference(&bits_[abs_(index)], bit_tools::maskOfOffset(rel_(index)));
        }
        
        const_reference operator[](size_t index) const {
            assert(index < size_);
            return const_reference(&(bits_[abs_(index)]), bit_tools::maskOfOffset(rel_(index)));
        }
        
        reference front() {
            return operator[](0);
        }
        
        const_reference front() const {
            return operator[](0);
        }
        
        reference back() {
            return operator[](size() - 1);
        }
        
        const_reference back() const {
            return operator[](size() - 1);
        }
        
        size_t rank(size_t index) const {
            return tipL_(index) + tipS_(index) + bit_tools::popCount(bits_[abs_(index)] & bit_tools::maskOfBits(rel_(index)));
        }
        
        size_t rank_1(size_t index) const {
            return rank(index);
        }
        
        size_t rank_0(size_t index) const {
            return index - rank(index);
        }
        
        size_t select(size_t index) const;
        
        size_t size() const {
            return size_;
        }
        
        void push_back(bool bit) {
            resize(size() + 1);
            back() = bit;
        }
        
        void buildRank();
        
        void buildSelect();
        
        void build(bool useSelect = false) {
            if (bits_.empty())
                return;
            buildRank();
            if (useSelect)
                buildSelect();
        }
        
        void resize(size_t size) {
            size_t abs = std::ceil(float(size) / kSmallBlockBits);
            bits_.resize(abs);
            size_ = size;
        }
        
        void reserve(size_t size) {
            size_t abs = std::ceil(float(size) / kSmallBlockBits);
            bits_.reserve(abs);
        }
        
        size_t sizeInBytes() const {
            auto size = sizeof(size_t);
            size +=  size_vec(bits_);
            size += l_blocks_.sizeInBytes();
            size += size_vec(s_blocks_);
            size += select_tips_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream& os) const {
            write_val(size_, os);
            write_vec(bits_, os);
            l_blocks_.write(os);
            write_vec(s_blocks_, os);
            select_tips_.write(os);
        }
        
        void read(std::istream& is) {
            size_ = read_val<size_t>(is);
            bits_ = read_vec<storage_type>(is);
            l_blocks_ = FitVector(is);
            s_blocks_ = read_vec<s_block_type>(is);
            select_tips_ = FitVector(is);
        }
        
    private:
        
        constexpr size_t block_(size_t index) const {
            return index / kLargeBlockBits;
        }
        
        constexpr size_t abs_(size_t index) const {
            return index / kSmallBlockBits;
        }
        
        constexpr size_t rel_(size_t index) const {
            return index % kSmallBlockBits;
        }
        
        size_t tipL_(size_t index) const {
            return l_blocks_[block_(index)];
        }
        
        size_t tipS_(size_t index) const {
            return s_blocks_[abs_(index)];
        }
        
    public:
        
        BitVector() = default;
        ~BitVector() = default;
        
        BitVector(const BitVector& rhs) noexcept  {
            size_ = rhs.size_;
            bits_ = rhs.bits_;
            l_blocks_ = rhs.l_blocks_;
            s_blocks_ = rhs.s_blocks_;
        }
        BitVector& operator=(const BitVector& rhs) noexcept {
            size_ = rhs.size_;
            bits_ = rhs.bits_;
            l_blocks_ = rhs.l_blocks_;
            s_blocks_ = rhs.s_blocks_;
            return *this;
        }
        
        BitVector(BitVector&& rhs) noexcept {
            size_ = std::move(rhs.size_);
            bits_ = std::move(rhs.bits_);
            l_blocks_ = std::move(rhs.l_blocks_);
            s_blocks_ = std::move(rhs.s_blocks_);
        }
        BitVector& operator=(BitVector&& rhs) noexcept {
            size_ = std::move(rhs.size_);
            bits_ = std::move(rhs.bits_);
            l_blocks_ = std::move(rhs.l_blocks_);
            s_blocks_ = std::move(rhs.s_blocks_);
            return *this;
        }
        
    };
    
    
    size_t BitVector::select(size_t index) const {
        id_type left = 0, right = l_blocks_.size();
        auto i = index;
        
        if (select_tips_.size() != 0) {
            auto selectTipId = i / kBitsPerSelectTips;
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
        for (; offset < kNumSPerL; offset++) {
            if (i <= s_blocks_[left * kNumSPerL + offset]) {
                break;
            }
        }
        i -= s_blocks_[left * kNumSPerL + --offset];
    
        auto ret = (left * kLargeBlockBits) + (offset * kSmallBlockBits);
        auto bits = bits_[ret / kSmallBlockBits];
        
        auto compress = [&bits, &ret, &i](const id_type block) {
            auto count = bit_tools::popCount(bits % block);
            auto shiftBlock = calc::sizeFitsInBytes(block - 1) * 8;
            if (count < i) {
                bits >>= shiftBlock;
                ret += shiftBlock;
                i -= count;
            }
        };
        
#ifndef USE_X86
        compress(0x100000000);
#endif
        compress(0x10000);
        compress(0x100);
        
        ret += bit_tools::selectTable(bits % 0x100, i);
        return ret - 1;
    }
    
    
    void BitVector::buildRank() {
        l_blocks_ = FitVector(calc::sizeFitsInBits(size_), std::ceil(float(size_) / kLargeBlockBits));
        s_blocks_.resize(std::ceil(float(size_) / kSmallBlockBits) + 1);
        
        auto count = 0;
        for (auto i = 0; i < l_blocks_.size(); i++) {
            l_blocks_[i] = count;
            
            for (auto offset = 0; offset < kNumSPerL; offset++) {
                auto index = i * kNumSPerL + offset;
                if (index > s_blocks_.size() - 1)
                    break;
                s_blocks_[index] = count - l_blocks_[i];
                if (index < bits_.size()) {
                    count += bit_tools::popCount(bits_[index]);
                }
            }
            
        }
        
    }
    
    
    void BitVector::buildSelect() {
        select_tips_ = FitVector(calc::sizeFitsInBits(l_blocks_.size()));
        auto count = kBitsPerSelectTips;
        select_tips_.push_back(0);
        for (id_type i = 0; i < l_blocks_.size(); i++) {
            if (count < l_blocks_[i]) {
                select_tips_.push_back(i - 1);
                count += kBitsPerSelectTips;
            }
        }
        
        select_tips_.push_back(l_blocks_.size() - 1);
    }
    
}

#endif /* VitVector_hpp */
