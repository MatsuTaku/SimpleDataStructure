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
#include "MultipleVector.hpp"
#include "calc.hpp"

namespace sim_ds {

template <class BitSequence>
class BitReference {
    using storage_type = typename BitSequence::storage_type;
    using storage_pointer = typename BitSequence::storage_pointer;
    
    friend typename BitSequence::Self;
    
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
    constexpr BitReference(storage_pointer p, storage_type m) noexcept : pointer_(p), mask_(m) {}
    
    storage_pointer pointer_;
    storage_type mask_;
    
};


template <class BitSequence>
class BitConstReference {
    using storage_type = typename BitSequence::storage_type;
    using storage_pointer = typename BitSequence::const_storage_pointer;
    
    friend typename BitSequence::Self;
    
public:
    constexpr operator bool() const {
        return static_cast<bool>(*pointer_ & mask_);
    }
    
private:
    constexpr BitConstReference(storage_pointer p, storage_type m) noexcept : pointer_(p), mask_(m) {}
    
    storage_pointer pointer_;
    storage_type mask_;
    
};


class BitVector {
    using Self = BitVector;
    using storage_type = id_type;
    using storage_pointer = storage_type*;
    using const_storage_pointer = const storage_type*;
    
    using s_block_type = uint8_t;
    
    static constexpr uint8_t kBitsPerWord = sizeof(id_type) * 8; // 64
    static constexpr uint8_t kSmallBlockBits = kBitsPerWord;
    static constexpr size_t kLargeBlockBits = 0x100;
    static constexpr uint8_t kNumSPerL = kLargeBlockBits / kSmallBlockBits; // (256) / 64 = 4
    static constexpr size_t kBitsPerSelectTips = 0x200;
    
    std::vector<storage_type> bits_;
    FitVector l_blocks_;
    std::vector<s_block_type> s_blocks_;
    FitVector select_tips_;
    size_t size_ = 0;
    
    friend class BitReference<BitVector>;
    friend class BitConstReference<BitVector>;
    
    using Reference = BitReference<BitVector>;
    using ConstReference = BitConstReference<BitVector>;
    
public:
    template <typename BitSet>
    BitVector(const BitSet& bools, bool use_rank, bool use_select = false) {
        resize(bools.size());
        for (auto i = 0; i < bools.size(); i++)
            operator[](i) = bools[i];
        if (use_rank)
            Build(use_select);
    }
    
    Reference operator[](size_t index) {
        assert(index < size());
        return Reference(&bits_[abs_(index)], bit_tools::OffsetMask(rel_(index)));
    }
    
    ConstReference operator[](size_t index) const {
        assert(index < size());
        return ConstReference(&(bits_[abs_(index)]), bit_tools::OffsetMask(rel_(index)));
    }
    
    Reference front() {
        return operator[](0);
    }
    
    ConstReference front() const {
        return operator[](0);
    }
    
    Reference back() {
        return operator[](size() - 1);
    }
    
    ConstReference back() const {
        return operator[](size() - 1);
    }
    
    void resize(size_t size) {
        size_t word_size = std::ceil(double(size) / kSmallBlockBits);
        bits_.resize(word_size);
        size_ = size;
    }
    
    void reserve(size_t size) {
        size_t word_size = std::ceil(double(size) / kSmallBlockBits);
        bits_.reserve(word_size);
    }
    
    void shrink_to_fit() {
        bits_.shrink_to_fit();
    }
    
    size_t size() const {
        return size_;
    }
    
    void push_back(bool bit) {
        resize(size() + 1);
        back() = bit;
    }
    
    void BuildRank();
    
    void BuildSelect();
    
    void Build(bool use_select) {
        if (bits_.empty())
            return;
        BuildRank();
        if (use_select)
            BuildSelect();
    }
    
    size_t rank(size_t index) const {
        auto rel = rel_(index);
        return tip_l_(index) + tip_s_(index) + (rel > 0 ? bit_tools::popcnt(bits_[abs_(index)] & bit_tools::WidthMask(rel)) : 0);
    }
    
    size_t rank_1(size_t index) const {
        return rank(index);
    }
    
    size_t rank_0(size_t index) const {
        return index - rank(index);
    }
    
    size_t select(size_t index) const;
    
    size_t size_in_bytes() const {
        auto size = sizeof(size_t);
        size +=  size_vec(bits_);
        size += l_blocks_.size_in_bytes();
        size += size_vec(s_blocks_);
        size += select_tips_.size_in_bytes();
        return size;
    }
    
    void Read(std::istream& is) {
        size_ = read_val<size_t>(is);
        bits_ = read_vec<storage_type>(is);
        l_blocks_ = FitVector(is);
        s_blocks_ = read_vec<s_block_type>(is);
        select_tips_ = FitVector(is);
    }
    
    void Write(std::ostream& os) const {
        write_val(size_, os);
        write_vec(bits_, os);
        l_blocks_.Write(os);
        write_vec(s_blocks_, os);
        select_tips_.Write(os);
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
    
    size_t tip_l_(size_t index) const {
        return l_blocks_[block_(index)];
    }
    
    size_t tip_s_(size_t index) const {
        return s_blocks_[abs_(index)];
    }
    
public:
    
    BitVector() = default;
    ~BitVector() = default;

    BitVector(const BitVector&) = default;
    BitVector& operator=(const BitVector&) = default;
    
    BitVector(BitVector&&) = default;
    BitVector& operator=(BitVector&&) = default;
    
};


void BitVector::BuildRank() {
    l_blocks_ = FitVector(calc::SizeFitsInBits(size_), std::ceil(double(size() + 1) / kLargeBlockBits));
    s_blocks_.resize(std::ceil(double(size() + 1) / kSmallBlockBits));
    
    size_t count = 0;
    for (auto i = 0; i < l_blocks_.size(); i++) {
        l_blocks_[i] = count;
        
        for (auto offset = 0; offset < kNumSPerL; offset++) {
            auto index = i * kNumSPerL + offset;
            if (index > s_blocks_.size() - 1)
                break;
            s_blocks_[index] = count - l_blocks_[i];
            if (index < bits_.size()) {
                count += bit_tools::popcnt(bits_[index]);
            }
        }
    }
}


void BitVector::BuildSelect() {
    select_tips_ = FitVector(calc::SizeFitsInBits(l_blocks_.size()));
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
    
    size_t offset = 1;
    for (size_t index = left * kNumSPerL + offset;
         offset < kNumSPerL && index < s_blocks_.size() && i > s_blocks_[index];
         offset++, index = left * kNumSPerL + offset) continue;
    i -= s_blocks_[left * kNumSPerL + --offset];
    
    auto ret = (left * kLargeBlockBits) + (offset * kSmallBlockBits);
    auto bits = bits_[ret / kSmallBlockBits];
    
    auto Compress = [&bits, &ret, &i](const id_type block) {
        auto count = bit_tools::popcnt(bits % block);
        auto shiftBlock = calc::SizeFitsInBytes(block - 1) * 8;
        if (count < i) {
            bits >>= shiftBlock;
            ret += shiftBlock;
            i -= count;
        }
    };
#ifndef USE_X86
    Compress(0x100000000);
#endif
    Compress(0x10000);
    Compress(0x100);
    
    ret += bit_tools::SelectTable(bits % 0x100, i);
    return ret - 1;
}


template <class BitSequence, bool UseSelect = false>
class SuccinctBitVector : MultipleVector {
    using sequence_type = BitSequence;
    using Base = MultipleVector;
    
    static constexpr uint8_t kSmallTipBits = BitSequence::kSmallBlockBits;
    static constexpr size_t kLargeTipBits = BitSequence::kLargeBlockBits;
    static constexpr size_t kNumSPerL = BitSequence::kNumSPerL;
    static constexpr size_t kBitsPerSelectTips = BitSequence::kBitsPerSelectTips;
    static constexpr size_t kLargeTipId = kNumSPerL * 1;
    static constexpr size_t kSelectTipId = kLargeTipId + 1;
    static constexpr size_t kLargeTipPos = kNumSPerL * 1;
    
    sequence_type bits_;
    FitVector select_tips_;
    
    friend typename BitSequence::Self;
    
public:
    constexpr size_t rank(size_t index) const {
        auto block_offset = block_offset_(index);
        auto l_count = Base::get_(block_offset + kLargeTipPos, Base::element_table_[kLargeTipId].size);
        auto s_count = Base::bytes_[block_offset + index / kSmallTipBits % kNumSPerL];
        auto rel = relative_offset_(index);
        return (l_count + s_count +
                (rel > 0 ? bit_tools::popcnt(bits_[index / kBitsPerWord]) & bit_tools::WidthMask(rel) : 0));
    }
    
    constexpr size_t select(size_t index) const;
    
    size_t num_blocks() const {
        return Base::size();
    }
    
    size_t size() const {
        return bits_.size();
    }
    
private:
    explicit SuccinctBitVector(const sequence_type bits);
    
    void BuildRank();
    void BuildSelect();
    
    constexpr size_t relative_offset_(size_t index) const {
        return index % kBitsPerWord;
    }
    
    constexpr size_t block_offset_(size_t index) const {
        return index / kLargeTipBits * block_size();
    }
    
    constexpr size_t l_tip_at_block(size_t block) const {
        return Base::get_(block * block_size() + kLargeTipPos, element_table_[kLargeTipId].size);
    }
    
    constexpr size_t s_tip_at_count(size_t count) const {
        return Base::bytes_[count / kNumSPerL * block_size() + count % kNumSPerL];
    }
    
};

template <class BitSequence, bool UseSelect>
constexpr size_t SuccinctBitVector<BitSequence, UseSelect>::select(size_t index) const {
    id_type left = 0, right = num_blocks();
    auto i = index;
    
    if (select_tips_.size() != 0) {
        auto selectTipId = i / kBitsPerSelectTips;
        left = select_tips_[selectTipId];
        right = select_tips_[selectTipId + 1] + 1;
    }
    
    while (left + 1 < right) {
        const auto center = (left + right) / 2;
        if (i < l_tip_at_block(i)) {
            right = center;
        } else {
            left = center;
        }
    }
    
    i += 1; // for i+1 th
    i -= l_tip_at_block(left);
    
    size_t offset = 1;
    for (size_t index = left * kNumSPerL + offset;
         offset < kNumSPerL && index < num_blocks() * kNumSPerL && i > s_tip_at_count(index);
         offset++, index = left * kNumSPerL + offset) continue;
    i -= s_tip_at_count(left * kNumSPerL + --offset);
    
    auto ret = (left * kLargeTipBits) + (offset * kSmallTipBits);
    auto bits = bits_[ret / kSmallTipBits];
    
    auto Compress = [&bits, &ret, &i](const id_type block) {
        auto count = bit_tools::popcnt(bits % block);
        auto shiftBlock = calc::SizeFitsInBytes(block - 1) * 8;
        if (count < i) {
            bits >>= shiftBlock;
            ret += shiftBlock;
            i -= count;
        }
    };
#ifndef USE_X86
    Compress(0x100000000);
#endif
    Compress(0x10000);
    Compress(0x100);
    
    ret += bit_tools::SelectTable(bits % 0x100, i);
    return ret - 1;
}

template <class BitSequence, bool UseSelect>
SuccinctBitVector<BitSequence, UseSelect>::SuccinctBitVector(const sequence_type bits) : bits_(bits) {
    auto num_blocks = std::ceil(double(bits.size() + 1) / kLargeTipBits);
    std::vector<size_t> element_sizes(kNumSPerL, 1);
    auto l_tip_size = calc::SizeFitsInBytes(bits.size());
    element_sizes.push_back(l_tip_size);
    if constexpr (UseSelect) {
        select_tips_ = FitVector(calc::SizeFitsInBits(num_blocks));
    }
    Base::set_element_sizes(element_sizes);
    Base::resize(num_blocks);
    
    BuildRank();
    if constexpr (UseSelect) {
        BuildSelect();
    }
}

template <class BitSequence, bool UseSelect>
void SuccinctBitVector<BitSequence, UseSelect>::BuildRank() {
    size_t count = 0;
    for (auto i = 0; i < num_blocks(); i++) {
        Base::block(i).set<kLargeTipId>(count);
        
        for (auto offset = 0; offset < kNumSPerL; offset++) {
            auto index = i * kNumSPerL + offset;
            if (index > kNumSPerL * num_blocks())
                break;
            Base::bytes_[i * block_size() + offset] = count - Base::block(i).template get<kLargeTipId>();
            if (index < bits_.size()) {
                count += bit_tools::popcnt(bits_[index]);
            }
        }
    }
}

template <class BitSequence, bool UseSelect>
void SuccinctBitVector<BitSequence, UseSelect>::BuildSelect() {
    auto count = kBitsPerSelectTips;
    select_tips_.push_back(0);
    for (id_type i = 0; i < num_blocks(); i++) {
        if (count < Base::block(i).template get<kLargeTipId>()) {
            select_tips_.push_back(i - 1);
            count += kBitsPerSelectTips;
        }
    }
    
    select_tips_.push_back(num_blocks() - 1);
}


} // namespace sim_ds

#endif /* VitVector_hpp */
