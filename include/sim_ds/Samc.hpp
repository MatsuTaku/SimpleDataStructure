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
        assert(trie.size() < std::numeric_limits<value_type>::max());
        
        std::vector<size_t> node_indexes = {graph_util::kRootIndex};
        storage_.emplace_back('^'); // root
        max_.emplace_back(0);
        long long max_index = 0;
        while (max_index >= 0) {
            
            std::cout << "block_height: " << max_index << std::endl;
            
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
            
            std::cout << "ycheck for ";
            
            BitVector empties;
            auto old_size = storage_.size();
            max_index = -1;
            code_table_.emplace_back();
            for (size_t i = 0; i < kAlphabetSize; i++) {
                auto& indices = indices_list[i];
                if (indices.empty()) {
                    code_table_.back()[i] = 0;
                    continue;
                }
                std::cout << i << ':' << uint8_t(i) << ' ';
#ifndef NDEBUG
                std::cout << std::endl;
                if(max_index >= 0)
                    ShowAsBinary(~empties.data()[max_index/64]);
                std::cout << "expand from: " << empties.size();
#endif
                empties.resize(max_index + 1 + height, true);
#ifndef NDEBUG
                std::cout << ", to: " << empties.size() << std::endl;
                ShowAsBinary(~empties.data()[max_index/64]);
#endif
                auto y_front = y_check_(indices, empties);
                max_index = std::max(max_index, y_front + indices.back());
                code_table_.back()[i] = height + y_front;
                storage_.resize(old_size + max_index + 1, kEmptyChar);
                node_indexes.resize(old_size + max_index + 1);
                for (auto id : indices) {
                    auto index = y_front + id;
                    auto abs_index = old_size + index;
#ifndef NDEBUG
                    std::cout << std::endl << "insert" << std::endl;
                    ShowAsBinary(~empties.data()[index/64]);
                    ShowAsBinary(1ull<<(index%64));
#endif
                    assert(storage_[abs_index] == kEmptyChar and
                           empties[index]);
                    storage_[abs_index] = i;
                    empties[index] = false;
                    auto parent_index = abs_index - (height + y_front);
                    auto parent_node = trie.node(node_indexes[parent_index]);
                    node_indexes[abs_index] = parent_node.target(i);
                }
            }
            std::cout << std::endl;
            
            if (max_index == -1) {
                code_table_.erase(code_table_.end()-1);
                break;
            }
            
            max_.push_back(storage_.size() - 1);
            
#ifndef NDEBUG
            auto used = SuccinctBitVector<false>(empties).rank_0(empties.size());
            double per_used = double(used) / empties.size() * 100;
            std::cout << "used: " << std::fixed << std::setprecision(2) << " %" << per_used << std::endl << std::endl;
#endif
        }
    }
    
private:
    position_type y_check_(const std::vector<value_type>& indices, const BitVector& empties) const {
        BitVector indice_mask((size_t)indices.back()+1);
        for (auto id : indices)
            indice_mask[id] = true;
        
        SuccinctBitVector<true> sbv(empties);
        auto num_empties = [&](size_t begin, size_t end) {
            return sbv.rank(end) - sbv.rank(begin);
        };
        
        auto idfront = indices.front();
        auto idback = indices.back();
        auto check = [&](position_type offset) -> size_t {
            assert(offset + idback < empties.size());
            auto index_front = offset + idfront;
            auto index_end = offset + idback+1;
            auto n_empties = num_empties(index_front, index_end);
            if (index_end - index_front == n_empties) // All of values are corresponding to expanded area.
                return 0;
            if (n_empties < indices.size()) { // There are not enough number of empty areas.
                return indices.size() - n_empties;
            }
            
            auto mask_data = indice_mask.data();
            auto empties_data = empties.data();
            const auto pi_end = (empties.size()-1)/64+1;
            auto field_bits_at = [&](long long index) {
                return index < pi_end ? ~empties_data[index] : 0;
            };
            const auto insets = offset & 0x3F;
            size_t i = idfront/64;
            position_type p = offset + 64ll*i;
            for (; i <= idback/64; i++, p+=64) {
                uint64_t mask = mask_data[i];
                if (mask == 0)
                    continue;
                uint64_t field = 0;
                if (p < 0) {
                    field = field_bits_at(0) << (64-insets);
                } else {
                    auto pi = p/64;
                    if (insets == 0)
                        field = field_bits_at(pi);
                    else
                        field = (field_bits_at(pi) >> insets) | (field_bits_at(pi+1) << (64-insets));
                }
                auto conflicts = mask & field;
                if (conflicts) {
#ifndef NDEBUG
                    std::cout << "conflict at:" << p << std::endl;
                    ShowAsBinary(field);
                    ShowAsBinary(mask);
                    ShowAsBinary(conflicts);
#endif
                    auto shifts = shifts_of_conflicts_(field, mask, conflicts);
                    while (shifts < 64 and (conflicts = (mask << shifts) & field)) {
#ifndef NDEBUG
                        std::cout << "shift: " << shifts << std::endl;
                        ShowAsBinary(field);
                        ShowAsBinary(mask << shifts);
                        ShowAsBinary(conflicts);
#endif
                        shifts += shifts_of_conflicts_(field, mask << shifts, conflicts);
                    }
#ifndef NDEBUG
                    std::cout << "ok shift: " << shifts << std::endl;
                    ShowAsBinary(field);
                    ShowAsBinary(mask<<shifts);
                    ShowAsBinary(conflicts);
#endif
                    return shifts;
                }
            }
#ifndef NDEBUG
            std::cout << "check" << std::endl;
            for (i = idfront/64, p = offset + 64ll*i; i <= idback/64; i++, p+=64) {
                uint64_t mask = mask_data[i];
                uint64_t field = 0;
                if (p < 0) {
                    field = field_bits_at(0) << (64-insets);
                } else {
                    auto pi = p/64;
                    if (insets == 0)
                        field = field_bits_at(pi);
                    else
                        field = (field_bits_at(pi) >> insets) | (field_bits_at(pi+1) << (64-insets));
                }
                auto conflicts = mask & field;
                ShowAsBinary(field);
                ShowAsBinary(mask);
                ShowAsBinary(conflicts);
                std::cout << std::endl;
                assert(not conflicts);
            }
#endif
            return 0;
        };
        
        const position_type gap = -(position_type)idfront;
        position_type empty_front = sbv.select(0);
#ifndef NDEBUG
        ShowAsBinary(~empties.data()[empty_front/64]);
#endif
        size_t shifts;
        while ((shifts = check(gap + empty_front)) > 0) {
            empty_front += shifts;
        }
        
        return gap + empty_front;
    };
    
    size_t shifts_of_conflicts_(uint64_t fields, uint64_t mask, uint64_t conflicts) const {
        assert((fields & mask) == conflicts);
        using std::__ctz, std::__clz;
        auto conflict_front = __ctz(conflicts);
        auto field_continuouses = __ctz(~(fields >> conflict_front));
        auto mask_followings = __clz(~(mask << (63 - conflict_front))) - 1;
        return field_continuouses + mask_followings;
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
    
    static constexpr size_t kSearchError = -1;
    
    SamcDict() = default;
    
    template <typename T, typename S>
    SamcDict(const graph_util::Trie<T, S>& trie) : Base(trie) {}
    
    size_t lookup(std::string_view key) const {
        size_t node = 0;
        size_t depth = 0;
        for (; depth < key.size(); depth++) {
            uint8_t c = key[depth];
            auto target = node + Base::code(depth, c);
            auto ch = Base::check(target);
            if (not in_range(target, depth+1) or
                Base::check(target) != c) {
                return kSearchError;
            }
            node = target;
        }
        auto terminal = node + Base::code(depth, Base::kLeafChar);
        if (not in_range(terminal, depth+1) or
            Base::check(terminal) != Base::kLeafChar) {
            return kSearchError;
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
