//
//  Samc.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/05/24.
//

#ifndef Samc_hpp
#define Samc_hpp

#include "basic.hpp"
#include "graph_util.hpp"
#include "EmptyLinkedVector.hpp"
#include "SuccinctBitVector.hpp"
#include "log.hpp"

namespace sim_ds {


template <typename ValueType> class _SamcDictImpl;

template <typename ValueType>
class _SamcImpl {
public:
    using value_type = ValueType;
    using position_type = long long;
    static constexpr size_t kAlphabetSize = 0x100;
    static constexpr uint8_t kEmptyChar = 0xFF;
    
protected:
    std::vector<uint8_t> storage_;
    using code_type = std::array<value_type, kAlphabetSize>;
    std::vector<code_type> code_table_;
    std::vector<value_type> max_;
    
    friend class _SamcDictImpl<ValueType>;
    
public:
    _SamcImpl() = default;
    
    uint8_t check(size_t index) const {return storage_[index];}
    
    size_t code(size_t depth, uint8_t c) const {return code_table_[depth][c];}
    
    size_t max(size_t depth) const {return max_[depth];}
    
    size_t size_in_bytes() const {
        size_t size = size_vec(storage_);
        size += size_vec(code_table_);
        size += size_vec(max_);
        return size;
    }
    
    void Write(std::ostream& os) const {
        write_vec(storage_, os);
        write_vec(code_table_, os);
        write_vec(max_, os);
    }
    
    void Read(std::istream& is) {
        storage_ = read_vec<uint8_t>(is);
        code_table_ = read_vec<code_type>(is);
        max_ = read_vec<value_type>(is);
    }
    
protected:
    template <typename T, typename S>
    _SamcImpl(const graph_util::Trie<T, S>& trie) {
        std::cerr << "Get trie" << std::endl;
        assert(trie.size() < std::numeric_limits<value_type>::max());
        
        std::vector<size_t> node_indexes = {graph_util::kRootIndex};
        storage_.emplace_back('^'); // root
        max_.emplace_back(0);
        long long max_index = 0;
        while (max_index >= 0) {
            std::cerr << "block_height: " << max_index << std::endl;
            std::array<std::vector<value_type>, kAlphabetSize> indices_list;
            auto height = max_index + 1;
            for (size_t i = 0; i < height; i++) {
                auto index = storage_.size() - height + i;
                if (storage_[index] == kEmptyChar)
                    continue;
                trie.node(node_indexes[index]).for_each_edge([&](uint8_t c, auto e) {
                    indices_list[c].push_back(i);
                });
            }
            
            std::cerr << "ycheck for ";
            EmptyLinkedVector<uint8_t> exists;
            max_index = -1;
            code_table_.emplace_back();
            for (size_t i = 0; i < kAlphabetSize; i++) {
                auto& indices = indices_list[i];
                if (indices.empty()) {
                    code_table_.back()[i] = 0;
                    continue;
                }
                std::cerr << i << ' ';
                
                exists.resize(max_index + 1 + height + 64);
                auto y_front = y_check_(indices, exists);
                for (auto id : indices) {
                    exists.set_value(y_front + id, i);
                }
                max_index = std::max(max_index, y_front + indices.back());
                code_table_.back()[i] = height + y_front;
            }
            if (max_index == -1) {
                code_table_.erase(code_table_.end()-1);
                break;
            }
            
            std::cerr << std::endl << "expand" << std::endl;
            auto old_size = storage_.size();
            storage_.resize(old_size + max_index + 1, kEmptyChar);
            node_indexes.resize(old_size + max_index + 1);
            for (size_t i = 0; i <= max_index; i++) {
                if (exists.empty_at(i))
                    continue;
                auto index = old_size + i;
                auto c = exists[i].value();
                storage_[index] = c;
                auto parent_index = index - code_table_.back()[c];
                auto parent_node = trie.node(node_indexes[parent_index]);
                node_indexes[index] = parent_node.target(c);
            }
            
            max_.push_back(storage_.size() - 1);
        }
    }
    
private:
    position_type y_check_(const std::vector<value_type>& indices, const EmptyLinkedVector<uint8_t>& exists) const {
        BitVector indice_mask((size_t)indices.back()+1);
        for (auto id : indices)
            indice_mask[id] = true;
        
        SuccinctBitVector<false> sbv(exists.bit_vector());
        auto num_empties = [&](size_t begin, size_t end) {
            return sbv.rank_0(end) - sbv.rank_0(begin);
        };
        
        auto idfront = indices.front();
        auto idback = indices.back();
        auto check = [&](position_type offset) -> size_t {
            auto empties = num_empties(offset + idfront, offset + idback+1);
            if (idback+1 - idfront == empties) // All of values are corresponding to expanded area.
                return 0;
            if (empties < indices.size()) { // Not enough to number of empty areas.
                auto shift = indices.size() - empties;
                shift += sbv.rank(offset + idback+1+shift) - sbv.rank(offset + idback+1);
                return shift;
            }
            
            auto mask_data = indice_mask.data();
            auto exists_data = exists.bit_vector().data();
            const auto insets = offset & 0x3F;
            size_t i = idfront/64;
            position_type p = offset + 64ll*i;
            position_type pi_end = (exists.size()-1)/64+1;
            for (; i <= idback/64; i++, p+=64) {
                uint64_t mask = mask_data[i];
                if (mask == 0)
                    continue;
                uint64_t hits = 0;
                if (p < 0) {
                    hits = exists_data[(p/64)+1] << (64-insets);
                } else {
                    auto pi = p/64;
                    hits = exists_data[pi] >> insets;
                    if (pi+1 < pi_end)
                        hits |= exists_data[pi+1] << (64-insets);
                }
                auto conflicts = mask & hits;
                if (conflicts) {
                    auto shift = shifts_of_conflicts_(conflicts, hits);
                    while (shift < 64 and (conflicts = (mask << shift) & hits)) {
                        shift += shifts_of_conflicts_(conflicts, hits);
                    }
                    return shift;
                }
            }
            return 0;
        };
        
        const position_type gap = -(position_type)idfront;
        position_type empty_front = exists.empty_front_index();
        size_t shift;
        while ((shift = check(gap + empty_front)) > 0) {
            empty_front += shift;
        }
        
        return gap + empty_front;
    };
                        
    size_t shifts_of_conflicts_(uint64_t conflicts, uint64_t fields) const {
        return std::__ctz(~fields >> std::__ctz(conflicts));
    }
    
};

// SAMC (Single Array with Multi CODE)
// reference:
//     M. Fuketa, et.al. Compression of double array structure for fixed length keywords.
template <typename ValueType>
class Samc : _SamcImpl<ValueType> {
public:
    using value_type = ValueType;
    using Base = _SamcImpl<value_type>;
    
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;
    
public:
    Samc() = default;
    
    template <typename T, typename S>
    Samc(const graph_util::Trie<T, S>& trie) : Base(trie) {}
    
    bool accept(std::string_view key) const {
        size_t node = 0;
        size_t depth = 0;
        for (; depth < key.size(); depth++) {
            uint8_t c = key[depth];
            auto target = node + Base::code(depth, c);
            if (not in_range(target, depth+1) or
                Base::check(target) != c) {
                return false;
            }
            node = target;
        }
        auto terminal = node + Base::code(depth, kLeafChar);
        return (in_range(terminal, depth+1) and
                Base::check(terminal) == kLeafChar);
    }
    
    size_t size_in_bytes() const {return Base::size_in_bytes();}
    
    void Write(std::ostream& os) const {
        Base::Write(os);
    }
    
    void Read(std::istream& is) {
        Base::Read(is);
    }
    
private:
    bool in_range(size_t index, size_t depth) const {
        assert(depth > 0);
        return index > Base::max(depth-1) and index <= Base::max(depth);
    }
    
    bool empty(size_t index) const {return Base::check(index) == Base::kEmptyChar;}
    
};

    
template <typename ValueType>
class _SamcDictImpl : protected _SamcImpl<ValueType> {
    using value_type = ValueType;
    using Base = _SamcImpl<value_type>;
    
public:
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;
    
protected:
    using succinct_bv_type = SuccinctBitVector<true>;
    succinct_bv_type leaves_;
    
public:
    _SamcDictImpl() = default;
    
    template <typename T, typename S>
    _SamcDictImpl(const graph_util::Trie<T, S>& trie) : Base(trie) {
        std::cerr << "Build dict" << std::endl;
        BitVector leaves_src(Base::storage_.size());
        size_t depth = 1;
        for (size_t i = 1; i < Base::storage_.size(); i++) {
            if (Base::max(depth) < i)
                depth++;
            if (Base::check(i) == kLeafChar)
                leaves_src[i] = true;
        }
        leaves_ = succinct_bv_type(leaves_src);
    }
    
    size_t id(size_t index) const {return leaves_.rank(index);}
    
    size_t leaf(size_t index) const {return leaves_.select(index);}
    
    size_t size_in_bytes() const {return Base::size_in_bytes() + leaves_.size_in_bytes();}
    
    void Write(std::ostream& os) const {
        Base::Write(os);
        leaves_.Write(os);
    }
    
    void Read(std::istream& is) {
        Base::Read(is);
        leaves_.Read(is);
    }
    
};


template <typename ValueType>
class SamcDict : _SamcDictImpl<ValueType> {
public:
    using value_type = ValueType;
    using Base = _SamcDictImpl<value_type>;
    
    SamcDict() = default;
    
    template <typename T, typename S>
    SamcDict(const graph_util::Trie<T, S>& trie) : Base(trie) {}
    
    size_t lookup(std::string_view key) const {
        size_t node = 0;
        size_t depth = 0;
        for (; depth < key.size(); depth++) {
            uint8_t c = key[depth];
            auto target = node + Base::code(depth, c);
            if (not in_range(target, depth+1) or
                Base::check(target) != c) {
                return false;
            }
            node = target;
        }
        auto terminal = node + Base::code(depth, Base::kLeafChar);
        if (not in_range(terminal, depth+1) or
            Base::check(terminal) != Base::kLeafChar) {
            return -1;
        }
        return Base::id(terminal);
    }
    
    std::string access(size_t value) const {
        std::string text;
        size_t node = Base::leaf(value);
        size_t depth = std::lower_bound(Base::max_.begin(), Base::max_.end(), node) - Base::max_.begin();
        while (depth > 0) {
            auto c = Base::check(node);
            node -= Base::code(--depth, c);
            text.push_back(c);
        }
        assert(node == 0);
        return std::string(text.rbegin(), text.rend()-1);
    }
    
    size_t size_in_bytes() const {return Base::size_in_bytes();}
    
    void Write(std::ostream& os) const {
        Base::Write(os);
    }
    
    void Read(std::istream& is) {
        Base::Read(is);
    }
    
private:
    bool in_range(size_t index, size_t depth) const {
        assert(depth > 0);
        return index > Base::max(depth-1) and index <= Base::max(depth);
    }
    
    bool empty(size_t index) const {return Base::check(index) == Base::kEmptyChar;}
    
};

}

#endif /* Samc_hpp */
