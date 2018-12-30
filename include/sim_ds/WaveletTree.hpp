//
//  WaveletTree.hpp
//  bench
//
//  Created by 松本拓真 on 2018/06/07.
//

#ifndef WaveletTree_hpp
#define WaveletTree_hpp

#include "basic.hpp"
#include "SuccinctBitVector.hpp"
#include "calc.hpp"

namespace sim_ds {
    
class WaveletTree {
public:
    using bv_type = SuccinctBitVector<false>;
    
    static constexpr uint8_t kMaxChar = 8;
    
private:
    size_t height_ = 0;
    size_t leafs_ = 0;
    size_t size_ = 0;
    std::vector<bv_type> bv_list_;
    
public:
    template <typename T>
    WaveletTree(const std::vector<T>& vec) {
        size_t max_char = *std::max_element(vec.begin(), vec.end());
        leafs_ = max_char + 1;
        
        auto height = calc::SizeFitsInBits(max_char);
        std::vector<BitVector> bv_list_src_;
        bv_list_src_.resize((1U << height) - 1);
        height_ = height;
        
        for (auto i = 0; i < vec.size(); i++) {
            auto id = 1;
            auto value = vec[i];
            for (auto depth = 0; depth < height_; depth++) {
                auto bit = (value >> (height_ - 1 - depth)) & 1U;
                bv_list_src_[id - 1].push_back(bit);
                id = (id << 1) | bit;
            }
        }
        
        bv_list_.resize(bv_list_src_.size());
        for (auto i = 0; i < bv_list_src_.size(); i++)
            bv_list_[i] = bv_type(bv_list_src_[i]);
    }
    
    uint8_t operator[](size_t index) const {
        uint8_t value = 0;
        auto id = 1;
        auto depth = 0;
        size_t idx = index;
        for (; depth < height_ - 1; depth++) {
            auto& cbv = bv_list_[id - 1];
            auto bit = cbv[idx];
            value |= bit << (height_ - 1 - depth);
            idx = !bit ? cbv.rank_0(idx) : cbv.rank_1(idx);
            id = (id << 1) | bit;
        }
        value |= bv_list_[id - 1][idx] << (height_ - 1 - depth);
        
        return value;
    }
    
    size_t rank(size_t index) const {
        size_t idx = index;
        for (auto depth = 0, id = 1; depth < height_; depth++) {
            auto& cbv = bv_list_[id - 1];
            auto bit = cbv[idx];
            idx = !bit ? cbv.rank_0(idx) : cbv.rank_1(idx);
            id = (id << 1) | bit;
        }
        return idx;
    }
    
    size_t rank(uint8_t c, size_t index) const {
        size_t idx = index;
        for (auto depth = 0, id = 1; depth < height_; depth++) {
            auto& cbv = bv_list_[id - 1];
            auto bit = (c >> (height_ - 1 - depth)) & 1;
            assert(bit == cbv[idx]);
            idx = !bit ? cbv.rank_0(idx) : cbv.rank_1(idx);
            id = (id << 1) | bit;
        }
        return idx;
    }
    
    std::pair<uint8_t, unsigned long long> AccessAndRank(size_t index) const {
        auto value = 0;
        size_t idx = index;
        for (auto depth = 0, id = 1; depth < height_; depth++) {
            auto& cbv = bv_list_[id - 1];
            auto bit = cbv[idx];
            value |= bit << (height_ - 1 - depth);
            idx = !bit ? cbv.rank_0(idx) : cbv.rank_1(idx);
            id = (id << 1) | bit;
        }
        
        return {value, idx};
    }
    
    size_t size() const {
        return bv_list_[0].size();
    }
    
    size_t node_diff(size_t node, size_t diff_height) {
        auto node_depth = calc::SizeFitsInBits(node);
        auto ti = bit_util::WidthMask(node_depth);
        auto fi = bit_util::WidthMask(node_depth - diff_height);
        return ti - fi;
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
    
};
    
} // namespace sim_ds

#endif /* WaveletTree_hpp */
