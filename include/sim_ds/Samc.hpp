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
#include "bit_util.hpp"
#include "SuccinctBitVector.hpp"
#include "log.hpp"

namespace sim_ds {

// SAMC (Single Array with Multi Code)
// reference:
//     M. Fuketa, H. Kitagawa, T. Ogawa, K. Morita, J. Aoe. Compression of double array structure for fixed length keywords. Information Processing and Management Vol 50 p796-806. 2014.
//

// MARK: - Samc

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
        read_vec(is, storage_);
        read_vec(is, code_table_);
        read_vec(is, max_);
    }
    
protected:
    template <typename T, typename S>
    _SamcImpl(const graph_util::Trie<T, S>& trie);
    
private:
    using mask_type = uint64_t;
    static constexpr size_t kMaskWidth = 64;
    
    position_type y_check_(const std::vector<value_type>& indices, const BitVector& empties) const;
    
    position_type y_check_legacy_(const std::vector<value_type>& indices, const BitVector& empties) const;
    
    size_t shifts_of_conflicts_(mask_type fields, mask_type mask, mask_type conflicts) const;
    
};

template <typename ValueType>
template <typename T, typename S>
_SamcImpl<ValueType>::_SamcImpl(const graph_util::Trie<T, S>& trie) {
    std::vector<size_t> node_indexes = {graph_util::kRootIndex};
    storage_.emplace_back('^'); // root
    max_.emplace_back(0);
    position_type max_index = 0;
    while (max_index >= 0) {
#ifndef NDEBUG
        std::cerr << "depth: " << max_.size()
        << ", block_height: " << max_index << std::endl;
#endif
        std::array<std::vector<value_type>, kAlphabetSize> indices_list;
        size_t height = max_index + 1;
        for (size_t i = 0; i < height; i++) {
            auto index = storage_.size() - height + i;
            if (storage_[index] == kEmptyChar)
                continue;
            trie.node(node_indexes[index]).for_each_edge([&](uint8_t c, auto e) {
                indices_list[c].push_back(i);
            });
        }
#ifndef NDEBUG
        std::cerr << "ycheck for each char..." << std::endl;;
#endif
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
#ifndef NDEBUG
            std::cerr << i << ':' << uint8_t(i) << ", indices: " << indices.size() << std::endl;
#endif
            empties.resize(max_index + 1 + height, true);
            auto y_front = y_check_(indices, empties);
            max_index = std::max(max_index, y_front + position_type(indices.back()));
            assert(height + y_front <= std::numeric_limits<value_type>::max());
            code_table_.back()[i] = height + y_front;
            storage_.resize(old_size + max_index + 1, kEmptyChar);
            node_indexes.resize(old_size + max_index + 1);
            for (auto id : indices) {
                auto index = y_front + id;
                auto abs_index = old_size + index;
                assert(storage_[abs_index] == kEmptyChar and
                       empties[index]);
                storage_[abs_index] = i;
                empties[index] = false;
                auto parent_index = abs_index - (height + y_front);
                auto parent_node = trie.node(node_indexes[parent_index]);
                node_indexes[abs_index] = parent_node.target(i);
            }
        }
        if (max_index == -1) {
            code_table_.erase(code_table_.end()-1);
            break;
        }
        max_.push_back(storage_.size() - 1);
#ifndef NDEBUG
        auto used = SuccinctBitVector<false>(empties).rank_0(empties.size());
        double per_used = double(used) / empties.size() * 100;
        std::cerr << "used: " << std::fixed << std::setprecision(2) << " %" << per_used << std::endl << std::endl;
#endif
    }
}

template <typename ValueType>
typename _SamcImpl<ValueType>::position_type
_SamcImpl<ValueType>::y_check_(const std::vector<value_type>& indices, const BitVector& empties) const {
    const auto word_size = (empties.size()-1)/kMaskWidth+1;
    auto field_bits = [&](size_t i) {
        return i < word_size ? empties.data()[i] : 0;
    };
    assert(position_type(empties.size()) + indices.front() - indices.back() >= 0);
    auto field_size = (empties.size() + indices.front() - indices.back())/kMaskWidth+1;
    position_type heads = indices.front();
    for (size_t i = 0; i < field_size; i++) {
        mask_type candidates = -1ull;
        for (auto id : indices) {
            auto p = (id-heads) / kMaskWidth;
            auto insets = (id-heads) % kMaskWidth;
            if (insets == 0) {
                candidates &= field_bits(p+i);
            } else {
                candidates &= (field_bits(p+i) >> insets) | (field_bits(p+i+1) << (kMaskWidth-insets));
            }
            if (candidates == 0)
                break;
        }
        if (candidates)
            return position_type(kMaskWidth) * i + bit_util::ctz(candidates) - heads;
    }
    return empties.size() + indices.front() - indices.back(); // ERROR
}

template <typename ValueType>
typename _SamcImpl<ValueType>::position_type
_SamcImpl<ValueType>::y_check_legacy_(const std::vector<value_type>& indices, const BitVector& empties) const {
    std::vector<mask_type> indice_mask(indices.back()/kMaskWidth+1, 0);
    for (auto id : indices) {
        indice_mask[id/kMaskWidth] |= 1ull << (id%kMaskWidth);
    }
    auto field_bits_at = [&](long long index) -> mask_type {
        return index < (empties.size()-1)/kMaskWidth+1 ? ~empties.data()[index] : 0;
    };
    
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
        
        for (auto id : indices) {
            if (not empties[offset + id]) {
                const mask_type mask = indice_mask[id/kMaskWidth];
                mask_type field = 0;
                const position_type p = offset + kMaskWidth*(id/kMaskWidth);
                const auto insets = offset & (kMaskWidth-1);
                if (p < 0) {
                    field = field_bits_at(0) << (kMaskWidth-insets);
                } else {
                    const auto pi = p/kMaskWidth;
                    if (insets == 0)
                        field = field_bits_at(pi);
                    else
                        field = (field_bits_at(pi) >> insets) | (field_bits_at(pi+1) << (kMaskWidth-insets));
                }
                size_t shifts = 0;
                mask_type conflicts = mask & field;
                while ((shifts += shifts_of_conflicts_(field, mask << shifts, conflicts)) < kMaskWidth and
                       (conflicts = (mask << shifts) & field)) continue;
                return shifts;
            }
        }
        
        return 0;
    };
    
    const position_type gap = - position_type(idfront);
    position_type empty_front = sbv.select(0);
    size_t shifts;
    while ((shifts = check(gap + empty_front)) > 0) {
        empty_front += shifts;
    }
    return gap + empty_front;
}

template <typename ValueType>
size_t
_SamcImpl<ValueType>::shifts_of_conflicts_(mask_type fields, mask_type mask, mask_type conflicts) const {
    assert((fields & mask) == conflicts);
    using bit_util::ctz, bit_util::clz;
    auto conflict_front = ctz(conflicts);
    auto field_continuouses = ctz(~(fields >> conflict_front));
    auto mask_followings = clz(~(mask << (kMaskWidth-1 - conflict_front))) - 1;
    return field_continuouses + mask_followings;
}


template <typename ValueType>
class Samc : _SamcImpl<ValueType> {
public:
    using value_type = ValueType;
    using _base = _SamcImpl<value_type>;
    template <typename T, typename S>
    using input_trie = graph_util::Trie<T, S>;
    
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;
    
public:
    Samc() = default;
    
    template <typename T, typename S>
    Samc(const input_trie<T, S>& trie) : _base(trie) {}
    
    bool accept(std::string_view key) const;
    
    size_t size_in_bytes() const {return _base::size_in_bytes();}
    
    void Write(std::ostream& os) const {
        _base::Write(os);
    }
    
    void Read(std::istream& is) {
        _base::Read(is);
    }
    
private:
    bool in_range(size_t index, size_t depth) const {
        assert(depth > 0);
        return index > _base::max(depth-1) and index <= _base::max(depth);
    }
    
    bool empty(size_t index) const {return _base::check(index) == _base::kEmptyChar;}
    
};

template <typename ValueType>
bool
Samc<ValueType>::accept(std::string_view key) const {
    size_t node = 0;
    size_t depth = 0;
    for (; depth < key.size(); depth++) {
        uint8_t c = key[depth];
        auto target = node + _base::code(depth, c);
        if (not in_range(target, depth+1) or
            _base::check(target) != c) {
            return false;
        }
        node = target;
    }
    auto terminal = node + _base::code(depth, kLeafChar);
    return (in_range(terminal, depth+1) and
            _base::check(terminal) == kLeafChar);
}

    
// MARK: SamcDict

template <typename ValueType>
class _SamcDictImpl : protected _SamcImpl<ValueType> {
    using value_type = ValueType;
    using _base = _SamcImpl<value_type>;
    
public:
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;
    
protected:
    using succinct_bv_type = SuccinctBitVector<true>;
    succinct_bv_type leaves_;
    
public:
    _SamcDictImpl() = default;
    
    template <typename T, typename S>
    _SamcDictImpl(const graph_util::Trie<T, S>& trie);
    
    size_t id(size_t index) const {
        assert(leaves_[index] == true);
        return leaves_.rank(index);
    }
    
    size_t leaf(size_t index) const {return leaves_.select(index);}
    
    size_t size_in_bytes() const {return _base::size_in_bytes() + leaves_.size_in_bytes();}
    
    void Write(std::ostream& os) const {
        _base::Write(os);
        leaves_.Write(os);
    }
    
    void Read(std::istream& is) {
        _base::Read(is);
        leaves_.Read(is);
    }
    
};

template <typename ValueType>
template <typename T, typename S>
_SamcDictImpl<ValueType>::_SamcDictImpl(const graph_util::Trie<T, S>& trie) : _base(trie) {
    std::cerr << "Build dict" << std::endl;
    BitVector leaves_src(_base::storage_.size());
    size_t depth = 1;
    for (size_t i = 1; i < _base::storage_.size(); i++) {
        if (_base::max(depth) < i)
            depth++;
        if (_base::check(i) == kLeafChar)
            leaves_src[i] = true;
    }
    leaves_ = succinct_bv_type(leaves_src);
}


template <typename ValueType>
class SamcDict : _SamcDictImpl<ValueType> {
public:
    using value_type = ValueType;
    using _base = _SamcDictImpl<value_type>;
    template <typename T, typename S>
    using input_trie = graph_util::Trie<T, S>;
    
    static constexpr size_t kSearchError = -1;
    
    SamcDict() = default;
    
    template <typename T, typename S>
    SamcDict(const input_trie<T, S>& trie) : _base(trie) {}
    
    size_t lookup(std::string_view key) const;
    
    std::string access(size_t value) const;
    
    size_t size_in_bytes() const {return _base::size_in_bytes();}
    
    void Write(std::ostream& os) const {
        _base::Write(os);
    }
    
    void Read(std::istream& is) {
        _base::Read(is);
    }
    
private:
    bool in_range(size_t index, size_t depth) const {
        assert(depth > 0);
        return index > _base::max(depth-1) and index <= _base::max(depth);
    }
    
    bool empty(size_t index) const {return _base::check(index) == _base::kEmptyChar;}
    
};

template <typename ValueType>
size_t
SamcDict<ValueType>::lookup(std::string_view key) const {
    size_t node = 0;
    size_t depth = 0;
    for (; depth < key.size(); depth++) {
        uint8_t c = key[depth];
        auto target = node + _base::code(depth, c);
        if (not in_range(target, depth+1) or
            _base::check(target) != c) {
            return kSearchError;
        }
        node = target;
    }
    auto terminal = node + _base::code(depth, _base::kLeafChar);
    if (not in_range(terminal, depth+1) or
        _base::check(terminal) != _base::kLeafChar) {
        return kSearchError;
    }
    return _base::id(terminal);
}

template <typename ValueType>
std::string
SamcDict<ValueType>::access(size_t value) const {
    std::string text;
    size_t node = _base::leaf(value);
    size_t depth = std::lower_bound(_base::max_.begin(), _base::max_.end(), node) - _base::max_.begin();
    while (depth > 0) {
        auto c = _base::check(node);
        assert(c != _base::kEmptyChar);
        node -= _base::code(--depth, c);
        text.push_back(c);
    }
    assert(node == 0);
    return std::string(text.rbegin(), text.rend()-1);
}

}

#endif /* Samc_hpp */
