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

namespace sim_ds {


template <typename ValueType> class _SamcDictImpl;

template <typename ValueType>
class _SamcImpl {
public:
    using value_type = ValueType;
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
    
protected:
    template <typename T>
    _SamcImpl(const graph_util::Trie<T>& trie) {
        assert(trie.size() < std::numeric_limits<value_type>::max());
        
        std::vector<size_t> node_indexes = {graph_util::kRootIndex};
        storage_.emplace_back('^');
        max_.emplace_back(0);
        std::vector<std::bitset<kAlphabetSize>> table(1);
        trie.root().for_each_edge([&](uint8_t c, auto e) {
            table.back()[c] = true;
        });
        while (not table.empty()) {
            EmptyLinkedVector<uint8_t> exists(table.size() * kAlphabetSize);
            auto y_check = [&](const std::vector<size_t>& indices) {
                long long gap = -indices.front();
                long long front = exists.empty_front_index();
                while (true) {
                    size_t j = 0;
                    for (; j < indices.size(); j++) {
                        auto index = gap + front + indices[j];
                        if (not exists[index].empty())
                            break;
                    }
                    if (j == indices.size())
                        break;
                    front = exists[front].next();
                }
                return gap + front;
            };
            
            long long max_index = -1;
            code_table_.emplace_back();
            for (size_t i = 0; i < kAlphabetSize; i++) {
                std::vector<size_t> indices;
                for (size_t j = 0; j < table.size(); j++) {
                    if (not table[j][i])
                        continue;
                    indices.push_back(j);
                }
                if (indices.empty()) {
                    code_table_.back()[i] = 0;
                    continue;
                }
                
                auto y_front = y_check(indices);
                for (auto id : indices) {
                    size_t index = y_front + id;
                    exists.set_value(index, i);
                    max_index = std::max(max_index, (long long)index);
                }
                auto block_gap = (max_.size() == 1 ? 1 :
                                  max_.back() - max_[max_.size() - 2]);
                code_table_.back()[i] = block_gap + y_front;
            }
            
            auto old_size = storage_.size();
            storage_.resize(old_size + max_index + 1, kEmptyChar);
            node_indexes.resize(old_size + max_index + 1);
            for (long long i = 0; i <= max_index; i++) {
                if (exists[i].empty())
                    continue;
                auto index = old_size + i;
                auto c = exists[i].value();
                storage_[index] = c;
                auto parent_index = index - code_table_.back()[c];
                auto parent_node = trie.node(node_indexes[parent_index]);
                node_indexes[index] = parent_node.target(c);
            }
            
            auto block_front = max_.back() + 1;
            max_.push_back(storage_.size() - 1);
            table.clear();
            for (size_t index = block_front; index < storage_.size(); index++) {
                table.emplace_back();
                trie.node(node_indexes[index]).for_each_edge([&](uint8_t c, auto e) {
                    table.back()[c] = true;
                });
            }
        }
    }
    
};

// SAMC (Single Array with Multi CODE)
// reference:
//     M. Fuketa, et.al. Compression of double array structure for fixed length keywords.
template <typename ValueType>
class Samc : _SamcImpl<ValueType> {
    using Base = _SamcImpl<ValueType>;
    
public:
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;
    
public:
    Samc() = default;
    
    template <typename T>
    Samc(const graph_util::Trie<T>& trie) : Base(trie) {}
    
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
    
private:
    bool in_range(size_t index, size_t depth) const {
        assert(depth > 0);
        return index > Base::max(depth-1) and index <= Base::max(depth);
    }
    
    bool empty(size_t index) const {return Base::check(index) == Base::kEmptyChar;}
    
};

    
template <typename ValueType>
class _SamcDictImpl : protected _SamcImpl<ValueType> {
    using Base = _SamcImpl<ValueType>;
    
public:
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;
    
protected:
    using succinct_bv_type = SuccinctBitVector<true>;
    succinct_bv_type leaves_;
    
public:
    _SamcDictImpl() = default;
    
    template <typename T>
    _SamcDictImpl(const graph_util::Trie<T>& trie) : Base(trie) {
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
    
};


template <typename ValueType>
class SamcDict : _SamcDictImpl<ValueType> {
    using Base = _SamcDictImpl<ValueType>;
    
public:
    SamcDict() = default;
    
    template <typename T>
    SamcDict(const graph_util::Trie<T>& trie) : Base(trie) {
        
    }
    
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
    
private:
    bool in_range(size_t index, size_t depth) const {
        assert(depth > 0);
        return index > Base::max(depth-1) and index <= Base::max(depth);
    }
    
    bool empty(size_t index) const {return Base::check(index) == Base::kEmptyChar;}
    
};

}

#endif /* Samc_hpp */
