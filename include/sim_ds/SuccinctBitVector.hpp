//
//  SuccinctBitVector.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2018/12/30.
//

#ifndef SuccinctBitVector_hpp
#define SuccinctBitVector_hpp

#include "BitVector.hpp"

namespace sim_ds {


template <bool UseSelect = true>
class SuccinctBitVector {
private:
    BitVector bits_;
    /* Contains large/small block as follows:
     * || large tip - 64bits - || small tip reversal - 9bits - | ... * 8||   -- 127bits per block
     */
    std::vector<uint64_t> basic_block_;
    // For select support
    std::vector<uint32_t> select_tips_;
    
public:
    SuccinctBitVector() = default;
    
    explicit SuccinctBitVector(const BitVector& bits) : bits_(bits) {
        if (bits.empty()) {
            basic_block_.assign(2, 0);
            return;
        }
        
        size_t basic_block_size = bits.size() / 512 + 1;
        basic_block_.resize(basic_block_size * 2);
        
        const uint64_t* data = bits.data();
        size_t sum = bit_util::popcnt(*data);
        uint64_t sum_word = 0;
        basic_block_[0] = basic_block_[1] = 0;
        size_t i = 0;
        for (i = 1; i < bits.size() / 64; i++) {
            if (i % 8 == 0) {
                size_t j = i/8*2;
                basic_block_[j - 1] = sum_word;
                basic_block_[j] = basic_block_[j - 2] + sum;
                sum_word = sum = 0;
            } else {
                sum_word |= sum << (63 - 9*(i%8));
            }
            sum += bit_util::popcnt(*(++data));
        }
        if (i % 8 != 0) {
            size_t j = i/8*2;
            sum_word |= sum << (63 - 9*(i%8));
            basic_block_[j + 1] = sum_word;
        } else {
            size_t j = i/8*2;
            basic_block_[j - 1] = sum_word;
            basic_block_[j] = basic_block_[j - 2] + sum;
            basic_block_[j + 1] = 0;
        }
        
        if constexpr (UseSelect) {
            auto sum_shreshold = 512;
            select_tips_.push_back(0);
            for (id_type i = 1; i < basic_block_size; i++) {
                if (sum_shreshold <= basic_block_[i * 2]) {
                    select_tips_.push_back(i - 1);
                    sum_shreshold += 512;
                }
            }
            if (select_tips_.back() != basic_block_size - 1)
                select_tips_.push_back(basic_block_size - 1);
        }
    }
    
    template <class BitSequence>
    explicit SuccinctBitVector(const BitSequence& bits) : SuccinctBitVector(BitVector(bits)) {}
    
    
    constexpr bool operator[](size_t index) const {
        return bits_[index];
    }
    
    constexpr size_t rank(size_t index) const {
        return rank_1(index);
    }
    
    constexpr size_t rank_0(size_t index) const {
        return index - rank_1(index);
    }
    
    constexpr size_t rank_1(size_t index) const {
        size_t block_index = index / 512 * 2;
        size_t before_sum = basic_block_[block_index] + ((basic_block_[block_index+1] >> (63-9*((index/64)%8))) & bit_util::width_mask<9>);
        size_t inword_offset = index % 64;
        if (inword_offset == 0) {
            return before_sum;
        } else {
            return before_sum + bit_util::popcnt(bits_.data()[index/64], inword_offset);
        }
    }
    
    constexpr size_t select(size_t index) const {
        id_type left = 0, right = num_blocks();
        size_t i = index;
        
        if (select_tips_.size() != 0) {
            auto select_tip_index = i / 512;
            left = select_tips_[select_tip_index];
            right = select_tips_[select_tip_index + 1] + 1;
        }
        while (left + 1 < right) {
            const auto center = (left + right) / 2;
            if (i < basic_block_[center * 2]) {
                right = center;
            } else {
                left = center;
            }
        }
        i += 1; // for i+1 th
        i -= basic_block_[left * 2];
        
        auto second_tip = [&](size_t index) {
            return (basic_block_[index/8*2+1] >> (63-9*(index%8))) & bit_util::width_mask<9>;
        };
        size_t offset = 1;
        size_t second_tip_size = size()/64+1;
        for (size_t index = left*8 + offset;
             offset < 8 && index < second_tip_size && i > second_tip(index);
             offset++, index++) continue;
        i -= second_tip(left*8 + --offset);
        
        size_t ret = left*512 + offset*64;
        auto bits = bits_.data()[ret/64];
        auto Compress = [&bits, &ret, &i](const id_type shift_bits) {
            auto count = bit_util::popcnt(bits, shift_bits);
            if (count < i) {
                bits >>= shift_bits;
                ret += shift_bits;
                i -= count;
            }
        };
        Compress(32);
        Compress(16);
        Compress(8);
        
        ret += bit_util::SelectTable(bits & 0xFF, i);
        
        return ret - 1;
    }
    
    size_t size() const {
        return bits_.size();
    }
    
    size_t num_blocks() const {
        return basic_block_.size() / 2;
    }
    
    size_t size_in_bytes() const {
        auto size = bits_.size_in_bytes();
        size += size_vec(basic_block_);
        if constexpr (UseSelect)
            size += size_vec(select_tips_);
        return size;
    }
    
    void Read(std::istream& is) {
        bits_.Read(is);
        basic_block_ = read_vec<uint64_t>(is);
        if constexpr (UseSelect)
            select_tips_ = read_vec<uint32_t>(is);
    }
    
    void Write(std::ostream& os) const {
        bits_.Write(os);
        write_vec(basic_block_, os);
        if constexpr (UseSelect)
            write_vec(select_tips_, os);
    }
    
};

}

#endif /* SuccinctBitVector_hpp */
