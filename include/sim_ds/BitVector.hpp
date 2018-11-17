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
        
        operator bool() const {
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
    public:
        using reference = __bit_reference<BitVector>;
        
    private:
        friend class __bit_reference<BitVector>;
        using __pointer_type = id_type*;
        using __word_type = id_type;
        using __s_block_type = uint8_t;
        
        static constexpr uint8_t __small_block_bits_ = sizeof(id_type) * 8; // 64
        static constexpr size_t __large_block_bits_ = 0x100;
        static constexpr uint8_t __num_s_per_l_ = __large_block_bits_ / __small_block_bits_; // (256) / 64 = 4
        static constexpr size_t __bits_per_select_tips_ = 0x200;
        
        size_t size_ = 0;
        std::vector<__word_type> bits_;
        FitVector l_blocks_;
        std::vector<__s_block_type> s_blocks_;
        FitVector select_tips_;
        
    public:
        template <class BITSET>
        BitVector(const BITSET& bools, bool useRank, bool useSelect = false) {
            resize(bools.size());
            for (auto i = 0; i < bools.size(); i++)
                operator[](i) = bools[i];
            if (useRank)
                build(useSelect);
        }
        
        reference operator[](size_t index) {
            assert(index < size_);
            return reference(&bits_[abs_(index)], 1ULL << rel_(index));
        }
        
        bool operator[](size_t index) const {
            assert(index < size_);
            return static_cast<bool>(bits_[abs_(index)] & (1UL << rel_(index)));
        }
        
        reference front() {
            return operator[](0);
        }
        
        bool front() const {
            return operator[](0);
        }
        
        reference back() {
            return operator[](size() - 1);
        }
        
        bool back() const {
            return operator[](size() - 1);
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
            size_t abs = std::ceil(float(size) / __small_block_bits_);
            bits_.resize(abs);
            size_ = size;
        }
        
        void reserve(size_t size) {
            size_t abs = std::ceil(float(size) / __small_block_bits_);
            bits_.reserve(abs);
        }
        
        void swapBits(BitVector &rhs) {
            std::swap(size_, rhs.size_);
            bits_.swap(rhs.bits_);
        }
        
        size_t sizeInBytes() const {
            return sizeof(size_t) + size_vec(bits_) + l_blocks_.sizeInBytes() + size_vec(s_blocks_);
        }
        
        void write(std::ostream& os) const {
            write_val(size_, os);
            write_vec(bits_, os);
            l_blocks_.write(os);
            write_vec(s_blocks_, os);
        }
        void read(std::istream& is) {
            size_ = read_val<size_t>(is);
            bits_ = read_vec<id_type>(is);
            l_blocks_ = FitVector(is);
            s_blocks_ = read_vec<uint8_t>(is);
        }
        
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
        
    private:
        
        constexpr size_t block_(size_t index) const {
            return index / __large_block_bits_;
        }
        
        constexpr size_t abs_(size_t index) const {
            return index / __small_block_bits_;
        }
        
        constexpr size_t rel_(size_t index) const {
            return index % __small_block_bits_;
        }
        
        size_t tipL_(size_t index) const {
            return l_blocks_[block_(index)];
        }
        
        size_t tipS_(size_t index) const {
            return s_blocks_[abs_(index)];
        }
        
    };
    
    
    unsigned long long BitVector::select(size_t index) const {
        id_type left = 0, right = l_blocks_.size();
        auto i = index;
        
        if (select_tips_.size() != 0) {
            auto selectTipId = i / __bits_per_select_tips_;
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
        for (; offset < __num_s_per_l_; offset++) {
            if (i <= s_blocks_[left * __num_s_per_l_ + offset]) {
                break;
            }
        }
        i -= s_blocks_[left * __num_s_per_l_ + --offset];
    
        auto ret = (left * __large_block_bits_) + (offset * __small_block_bits_);
        auto bits = bits_[ret / __small_block_bits_];
        
        auto compress = [&bits, &ret, &i](const id_type block) {
            auto count = bit_tools::popCount(bits % block);
            auto shiftBlock = 0;
            while ((block - 1) >> (8 * ++shiftBlock));
            shiftBlock *= 8;
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
        
        ret += bit_tools::kSelectTable[i][bits % 0x100];
        return ret - 1;
    }
    
    
    void BitVector::buildRank() {
        l_blocks_ = FitVector(calc::sizeFitsInBits(size_), std::ceil(float(size_) / __large_block_bits_));
        s_blocks_.resize(std::ceil(float(size_) / __small_block_bits_));
        
        auto count = 0;
        for (auto i = 0; i < l_blocks_.size(); i++) {
            l_blocks_[i] = count;
            
            for (auto offset = 0; offset < __num_s_per_l_; offset++) {
                auto index = i * __num_s_per_l_ + offset;
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
        auto count = __bits_per_select_tips_;
        select_tips_.push_back(0);
        for (id_type i = 0; i < l_blocks_.size(); i++) {
            if (count < l_blocks_[i]) {
                select_tips_.push_back(i - 1);
                count += __bits_per_select_tips_;
            }
        }
        
        select_tips_.push_back(l_blocks_.size() - 1);
    }
    
}

#endif /* VitVector_hpp */
