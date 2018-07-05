//
//  VitVector.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/10/30.
//

#ifndef VitVector_hpp
#define VitVector_hpp

#include "basic.hpp"
#include "Log.hpp"
#include "bit_tools.hpp"
#include "Vector.hpp"

namespace sim_ds {
    
    // inspired by marisa-trie
    constexpr uint8_t kSelectTable[9][256] = {
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {
            8, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            8, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
            5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1
        },
        {
            8, 8, 8, 2, 8, 3, 3, 2, 8, 4, 4, 2, 4, 3, 3, 2,
            8, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
            8, 6, 6, 2, 6, 3, 3, 2, 6, 4, 4, 2, 4, 3, 3, 2,
            6, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
            8, 7, 7, 2, 7, 3, 3, 2, 7, 4, 4, 2, 4, 3, 3, 2,
            7, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
            7, 6, 6, 2, 6, 3, 3, 2, 6, 4, 4, 2, 4, 3, 3, 2,
            6, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
            8, 8, 8, 2, 8, 3, 3, 2, 8, 4, 4, 2, 4, 3, 3, 2,
            8, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
            8, 6, 6, 2, 6, 3, 3, 2, 6, 4, 4, 2, 4, 3, 3, 2,
            6, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
            8, 7, 7, 2, 7, 3, 3, 2, 7, 4, 4, 2, 4, 3, 3, 2,
            7, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2,
            7, 6, 6, 2, 6, 3, 3, 2, 6, 4, 4, 2, 4, 3, 3, 2,
            6, 5, 5, 2, 5, 3, 3, 2, 5, 4, 4, 2, 4, 3, 3, 2
        },
        {
            8, 8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 4, 8, 4, 4, 3,
            8, 8, 8, 5, 8, 5, 5, 3, 8, 5, 5, 4, 5, 4, 4, 3,
            8, 8, 8, 6, 8, 6, 6, 3, 8, 6, 6, 4, 6, 4, 4, 3,
            8, 6, 6, 5, 6, 5, 5, 3, 6, 5, 5, 4, 5, 4, 4, 3,
            8, 8, 8, 7, 8, 7, 7, 3, 8, 7, 7, 4, 7, 4, 4, 3,
            8, 7, 7, 5, 7, 5, 5, 3, 7, 5, 5, 4, 5, 4, 4, 3,
            8, 7, 7, 6, 7, 6, 6, 3, 7, 6, 6, 4, 6, 4, 4, 3,
            7, 6, 6, 5, 6, 5, 5, 3, 6, 5, 5, 4, 5, 4, 4, 3,
            8, 8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 4, 8, 4, 4, 3,
            8, 8, 8, 5, 8, 5, 5, 3, 8, 5, 5, 4, 5, 4, 4, 3,
            8, 8, 8, 6, 8, 6, 6, 3, 8, 6, 6, 4, 6, 4, 4, 3,
            8, 6, 6, 5, 6, 5, 5, 3, 6, 5, 5, 4, 5, 4, 4, 3,
            8, 8, 8, 7, 8, 7, 7, 3, 8, 7, 7, 4, 7, 4, 4, 3,
            8, 7, 7, 5, 7, 5, 5, 3, 7, 5, 5, 4, 5, 4, 4, 3,
            8, 7, 7, 6, 7, 6, 6, 3, 7, 6, 6, 4, 6, 4, 4, 3,
            7, 6, 6, 5, 6, 5, 5, 3, 6, 5, 5, 4, 5, 4, 4, 3
        },
        {
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
            8, 8, 8, 8, 8, 8, 8, 5, 8, 8, 8, 5, 8, 5, 5, 4,
            8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 4,
            8, 8, 8, 6, 8, 6, 6, 5, 8, 6, 6, 5, 6, 5, 5, 4,
            8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 4,
            8, 8, 8, 7, 8, 7, 7, 5, 8, 7, 7, 5, 7, 5, 5, 4,
            8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 4,
            8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
            8, 8, 8, 8, 8, 8, 8, 5, 8, 8, 8, 5, 8, 5, 5, 4,
            8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 4,
            8, 8, 8, 6, 8, 6, 6, 5, 8, 6, 6, 5, 6, 5, 5, 4,
            8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 4,
            8, 8, 8, 7, 8, 7, 7, 5, 8, 7, 7, 5, 7, 5, 5, 4,
            8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 4,
            8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4
        },
        {
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
            8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 5,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
            8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 5,
            8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6,
            8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 5,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
            8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 6, 8, 6, 6, 5,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
            8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 5,
            8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6,
            8, 8, 8, 7, 8, 7, 7, 6, 8, 7, 7, 6, 7, 6, 6, 5
        },
        {
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
            8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
            8, 8, 8, 8, 8, 8, 8, 7, 8, 8, 8, 7, 8, 7, 7, 6
        },
        {
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7
        },
        {
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
        }
    };
    
    class BitVector {
    protected:
        using id_type = uint64_t;
        using block_type = uint8_t;
        static constexpr uint8_t kSBlockSize = sizeof(id_type) * 8; // 64
        static constexpr size_t kLBlockSize = 0x100 + kSBlockSize;
        static constexpr uint8_t kBlocksInTip = kLBlockSize / kSBlockSize; // (256 + 64) / 64 = 5
        static constexpr size_t kNum1sPerTip = 0x200;
        
    public:
        BitVector() = default;
        
        BitVector(const std::vector<bool> &bools, bool useRank, bool useSelect = false) {
            for (auto i = 0; i < bools.size(); i++)
                set(i, bools[i]);
            if (useRank)
                build(useSelect);
        }
        
        ~BitVector() = default;
        
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
        
//        BitVector(BitVector&&) noexcept = default;
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
        
        constexpr bool operator[](size_t index) const {
            return access(index);
        }
        
        constexpr bool access(size_t index) const {
            return (bits_[abs(index)] & (1UL << rel(index))) != 0;
        }
        
        constexpr unsigned long long rank(size_t index) const;
        
        constexpr unsigned long long rank0(size_t index) const;
        
        constexpr unsigned long long select(size_t index) const;
        
        size_t size() const {
            return size_;
        }
        
        void set(size_t index, bool bit) {
            check_resize(index);
            if (bit)
                bits_[abs(index)] |= (1UL << rel(index));
            else 
                bits_[abs(index)] &= ~(1UL << rel(index));
        }
        
        void push_back(bool bit) {
            set(size_, bit);
        }
        
        void build(bool useSelect);
        
        void buildRank();
        
        void buildSelect();
        
        void check_resize(size_t index) {
            if (index + 1 > size_)
                size_ = index + 1;
            if (abs(index) + 1 > bits_.size()) {
                resize(index + 1);
            }
        }
        
        void resize(size_t size) {
            if (size == 0)
                bits_.resize(0);
            else
                bits_.resize(abs(size - 1) + 1);
            size_ = size;
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
            l_blocks_ = Vector(is);
            s_block_units_ = read_vec<uint8_t>(is);
        }
        
    protected:
        size_t size_ = 0;
        std::vector<id_type> bits_;
        Vector l_blocks_;
        std::vector<block_type> s_block_units_;
        std::vector<id_type> select_tips_;
        
        constexpr size_t block(size_t index) const {
            return index / kLBlockSize;
        }
        
        constexpr size_t abs(size_t index) const {
            return index / kSBlockSize;
        }
        
        constexpr size_t rel(size_t index) const {
            return index % kSBlockSize;
        }
        
        constexpr size_t tipL(size_t index) const {
            return l_blocks_[block(index)];
        }
        
        constexpr size_t tipS(size_t index) const {
            return s_block_units_[abs(index)];
        }
        
    };
    
    inline constexpr unsigned long long BitVector::rank(size_t index) const {
        auto relI = rel(index);
        return tipL(index) + tipS(index) + (relI == 0 ? 0 : bit_tools::popCount(bits_[abs(index)] & ((1UL << relI) - 1)));
    }
    
    inline constexpr unsigned long long BitVector::rank0(size_t index) const {
        return index - rank(index);
    }
    
    inline constexpr unsigned long long BitVector::select(size_t index) const {
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
        
        ret += kSelectTable[i][bits % 0x100];
        return ret - 1;
    }
    
    inline void BitVector::build(bool useSelect = false) {
        
        if (bits_.empty()) return;
        
        buildRank();
        
        if (useSelect) buildSelect();
        
    }
    
    inline void BitVector::buildRank() {
        
        auto blockTypeSize = Calc::sizeFitInBytes(size_);
        l_blocks_ = Vector(8 * blockTypeSize, size_ / kLBlockSize + 1);
//        std::vector<size_t> lBlocks(size_ / kLBlockSize + 1);
        s_block_units_.resize(size_ / kSBlockSize + 1);
        
        auto count = 0;
        for (auto i = 0; i < l_blocks_.size(); i++) {
            l_blocks_.set(i, count);
//            lBlocks[i] = count;
            
            for (auto offset = 0; offset < kBlocksInTip; offset++) {
                auto index = i * kBlocksInTip + offset;
                s_block_units_[index] = count - l_blocks_[i];
                if (index < bits_.size()) {
                    count += bit_tools::popCount(bits_[index]);
                }
            }
            
        }
        
//        l_blocks_ = Vector(lBlocks);
        
    }
    
    inline void BitVector::buildSelect() {
        
        auto count = kNum1sPerTip;
        select_tips_.push_back(0);
        for (id_type i = 0; i < l_blocks_.size(); i++) {
            if (count < l_blocks_[i]) {
                select_tips_.push_back(i - 1);
                count += kNum1sPerTip;
            }
        }
        
        select_tips_.push_back(l_blocks_.size() - 1);
    }
    
}

#endif /* VitVector_hpp */
