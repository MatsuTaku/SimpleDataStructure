//
//  WaveletTree.hpp
//  bench
//
//  Created by 松本拓真 on 2018/06/07.
//

#ifndef WaveletTree_hpp
#define WaveletTree_hpp

#include "basic.hpp"
#include "BitVector.hpp"
#include "calc.hpp"

namespace sim_ds {
    
class WaveletTree {
    
    size_t height_ = 0;
    size_t leafs_ = 0;
    size_t size_ = 0;
    std::vector<BitVector> bv_list_;
    
    static constexpr uint8_t kMaxChar = 8;
    
public:
    WaveletTree() {
        expand(kMaxChar);
    }
    
    template <typename T>
    WaveletTree(std::vector<T>& vec) {
        size_t maxChar = *std::max_element(vec.begin(), vec.end());
        expand(maxChar + 1);
        
        for (auto i = 0; i < vec.size(); i++)
            set(i, vec[i]);
        
        build();
    }
    
    uint8_t operator[](size_t index) const {
        
        uint8_t value = 0;
        auto id = 1;
        auto depth = 0;
        
        for (; depth < height_ - 1; depth++) {
            auto &cbv = bv_list_[id - 1];
            
            auto bit = cbv[index];
            value |= bit << (height_ - 1 - depth);
            index = !bit ? cbv.rank_0(index) : cbv.rank_1(index);
            
            id = (id << 1) | bit;
        }
        value |= bv_list_[id - 1][index] << (height_ - 1 - depth);
        
        return value;
    }
    
    unsigned long long rank(size_t index) const {
        
        for (auto depth = 0, id = 1; depth < height_; depth++) {
            auto &cbv = bv_list_[id - 1];
            
            auto bit = cbv[index];
            index = !bit ? cbv.rank_0(index) : cbv.rank_1(index);
            
            id = (id << 1) | bit;
        }
        
        return index;
    }
    
    unsigned long long rank(uint8_t c, size_t index) const {
        
        for (auto depth = 0, id = 1; depth < height_; depth++) {
            auto &cbv = bv_list_[id - 1];
            
            auto bit = (c >> (height_ - 1 - depth)) & 1;
            assert(bit == cbv[index]);
            index = !bit ? cbv.rank_0(index) : cbv.rank_1(index);
            
            id = (id << 1) | bit;
        }
        
        return index;
    }
    
    std::pair<uint8_t, unsigned long long> accessAndRank(size_t index) const {
        auto value = 0;
        for (auto depth = 0, id = 1; depth < height_; depth++) {
            auto &cbv = bv_list_[id - 1];
            
            auto bit = cbv[index];
            value |= bit << (height_ - 1 - depth);
            index = !bit ? cbv.rank_0(index) : cbv.rank_1(index);
            
            id = (id << 1) | bit;
        }
        
        return std::make_pair<uint8_t, unsigned long long>(value, index);
    }
    
    size_t size() const {
        return size_;
    }
    
    // Require to call from src continuous.
    void set(size_t index, uint8_t value) {
        if (index > size_ - 1)
            size_ = index + 1;
//            if (value + 1 > leafs_)
//                expand(value + 1);
        
        auto id = 1;
        for (auto depth = 0; depth < height_; depth++) {
            auto bit = (value >> (height_ - 1 - depth)) & 1U;
            auto &cbv = bv_list_[id - 1];
            cbv.push_back(bit);
            id = (id << 1) | bit;
        }
    }
    
    void push_back(uint8_t c) {
        set(size_, c);
        size_ += 1;
    }
    
    void resize(size_t size) {
        if (size <= size_)
            return;
        size_ = size;
        set(size - 1, 0);
    }
    
    size_t nodeDiff(size_t node, size_t diffHeight) {
        auto nodeDepth = calc::SizeFitsInBits(node);
        auto ti = bit_tools::WidthMask(nodeDepth);
        auto fi = bit_tools::WidthMask(nodeDepth - diffHeight);
        return ti - fi;
    }
    
    void expand(size_t chars) {
        if (chars <= leafs_)
            return;
        leafs_ = chars;

        auto height = calc::SizeFitsInBits(chars - 1);
        if (height > height_) {
            bv_list_.resize((1U << height) - 1);
//                for (auto n = (1 << height_) - 1; n > 0; n--) {
//                    auto di = nodeDiff(n, height - height_);
//                    bv_list_[n + di - 1] = std::move(bv_list_[n - 1]);
//                    bv_list_[n - 1].resize(0);
//                }
//                if (height_ != 0) {
//                    auto pRootSize = bv_list_[(1 << (height - height_)) - 1].size();
//                    for (auto i = 0; i < (1 << (height - 1)) - 1; i++) {
//                        bv_list_[i].resize((i == 0 || (i & 1) == 1) ? pRootSize : 0);
//                    }
//                }

            height_ = height;
        }
    }
    
    void build() {
        for (auto &l : bv_list_)
            l.Build(false);
    }
    
    size_t size_in_bytes() const {
        auto size = sizeof(size_t) * 3;
        for (auto &l : bv_list_)
            size += l.size_in_bytes();
        return size;
    }
    
    void Read(std::istream &is) {
        height_ = read_val<size_t>(is);
        leafs_ = read_val<size_t>(is);
        size_ = read_val<size_t>(is);
        for (auto &l : bv_list_)
            l.Read(is);
    }
    
    void Write(std::ostream &os) const {
        write_val(height_, os);
        write_val(leafs_, os);
        write_val(size_, os);
        for (auto &l : bv_list_)
            l.Write(os);
    }
    
    ~WaveletTree() = default;
    
    WaveletTree(const WaveletTree&) = delete;
    WaveletTree& operator=(const WaveletTree&) = delete;
    
    WaveletTree(WaveletTree&&) noexcept = default;
    WaveletTree& operator=(WaveletTree&&) noexcept = default;
    
};
    
} // namespace sim_ds

#endif /* WaveletTree_hpp */
