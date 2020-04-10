//
//  Samc.hpp
//
//  Created by 松本拓真 on 2019/05/24.
//

#ifndef Samc_hpp
#define Samc_hpp

#include <iterator>

#include "basic.hpp"
#include "graph_util.hpp"
#include "bit_util.hpp"
#include "BitVector.hpp"
#include "SuccinctBitVector.hpp"
#include "log.hpp"

namespace sim_ds {

template <typename Iter>
class string_array_explorer {
public:
    using iterator_traits = std::iterator_traits<Iter>;
    static_assert(std::is_convertible_v<typename iterator_traits::value_type, std::string>);
    static constexpr size_t kInvalidDepth = std::numeric_limits<size_t>::max();
private:
    Iter begin_;
    Iter end_;
    size_t depth_ = kInvalidDepth;

public:
    string_array_explorer() = default;
    explicit string_array_explorer(Iter begin, Iter end, size_t depth) : begin_(begin), end_(end), depth_(depth) {}

    template <typename Action>
    void for_each_edge(Action action) const {
      if (depth_ == kInvalidDepth)
        throw std::logic_error("class is not initialized");
      auto it = begin_;
      if (begin_->size() == depth_) {
        action('\0', string_array_explorer(end_, end_, depth_));
        while (++it != end_ and it->size() == depth_) {}
      }
      while (it != end_) {
        auto c = (*it)[depth_];
        auto endit = it;
        while (endit != end_ and (*endit)[depth_] == c)
          ++endit;
        action(c, string_array_explorer(it, endit, depth_+1));
        it = endit;
      }
    }

};


// SAMC (Single Array with Multi Code)
// reference:
//    M. Fuketa, H. Kitagawa, T. Ogawa, K. Morita, J. Aoe.
//    Compression of double array structure for fixed length keywords.
//    Information Processing and Management Vol 50 p796-806. 2014.
//

// MARK: - Samc

template <typename CodeType>
class _SamcImpl {
public:
    using code_type = CodeType;
    using position_type = long long;
    using char_type = uint8_t;
    static constexpr size_t kAlphabetSize = 0x100;
    static constexpr char_type kEmptyChar = kAlphabetSize-1;
    static constexpr char_type kLeafChar = 0;

protected:
    std::vector<char_type> storage_;
    std::vector<std::array<code_type, kAlphabetSize>> code_table_;
    std::vector<code_type> head_;

public:
    _SamcImpl() = default;

    char_type check(size_t index) const {return storage_[index];}

    code_type code(size_t depth, char_type c) const {return code_table_[depth][c];}

    size_t head(size_t depth) const {return head_[depth];}

    size_t size_in_bytes() const {
      size_t size = size_vec(storage_);
      size += size_vec(code_table_);
      size += size_vec(head_);
      return size;
    }

    void Write(std::ostream& os) const {
      write_vec(storage_, os);
      write_vec(code_table_, os);
      write_vec(head_, os);
    }

    void Read(std::istream& is) {
      read_vec(is, storage_);
      read_vec(is, code_table_);
      read_vec(is, head_);
    }

protected:
    template <typename Iter>
    explicit _SamcImpl(const string_array_explorer<Iter>& explorer);

    template <typename StrIter>
    explicit _SamcImpl(StrIter begin, StrIter end) : _SamcImpl(string_array_explorer(begin, end, 0)) {}

    template <typename T, typename S>
    [[deprecated]]
    explicit _SamcImpl(const graph_util::Trie<T, S>& trie);

private:
    using mask_type = uint64_t;
    static constexpr size_t kMaskWidth = 64;

    position_type y_check_(const std::vector<size_t>& indices, const BitVector& empties) const;

    position_type y_check_legacy_(const std::vector<size_t>& indices, const BitVector& empties) const;

    size_t shifts_of_conflicts_(mask_type fields, mask_type mask, mask_type conflicts) const;

};

template <typename CodeType>
template <typename Iter>
_SamcImpl<CodeType>::_SamcImpl(const string_array_explorer<Iter>& explorer) {
  std::map<size_t, char> storage_map;
  std::queue<std::pair<size_t, string_array_explorer<Iter>>> node_queue;
  // set root
  storage_map[0] = '^';
  node_queue.emplace(0, explorer);
  head_ = {0,1};

  for (size_t depth = 0; ; ++depth) {
    if (node_queue.empty())
      break;
#ifndef NDEBUG
    std::cerr << "depth: " << depth
              << ", block_height: " << head_[depth+1]-head_[depth] << std::endl;
#endif
    std::array<std::vector<size_t>, kAlphabetSize> indices_table;
    std::array<std::vector<string_array_explorer<Iter>>, kAlphabetSize> node_table;
    while (not node_queue.empty()) {
      auto node_pair = node_queue.front(); node_queue.pop();
      auto index = node_pair.first;
      auto node = node_pair.second;
      assert(storage_map.count(index) > 0);
      assert(storage_map[index] != kLeafChar);
      node.for_each_edge([&](uint8_t c, auto target_node) {
          indices_table[c].push_back(index - head_[depth]);
          node_table[c].push_back(target_node);
      });
    }
    BitVector empty_bv;
    size_t max_height = 0;
    code_table_.emplace_back();
#ifndef NDEBUG
    std::cerr << "ycheck for each char..." << std::endl;;
#endif
    for (size_t c = 0; c < kAlphabetSize; c++) {
      auto& indices = indices_table[c];
      if (indices.empty())
        continue;
#ifndef NDEBUG
      std::cerr << c << ':' << uint8_t(c) << ", indices: " << indices.size() << std::endl;
#endif
      auto& nodes = node_table[c];
      auto indices_height = 1 + *max_element(indices.begin(), indices.end()) - *min_element(indices.begin(), indices.end());
      if (empty_bv.size() < max_height + indices_height)
        empty_bv.resize(max_height + indices_height, true);
      auto y_front = y_check_(indices, empty_bv);
      const size_t prev_height = head_[depth+1]-head_[depth];
      auto code = prev_height + y_front;
      code_table_[depth][c] = code;
      for (size_t i = 0; i < indices.size(); i++) {
        size_t index = head_[depth] + indices[i] + code;
//#ifndef NDEBUG
//        std::cerr<<index<<"] "<<c<<std::endl;
//#endif
        assert(storage_map.count(index) == 0);
        storage_map[index] = c;
        size_t inset = index - head_[depth+1];
        max_height = std::max(max_height, inset+1);
        empty_bv[inset] = false;
        if (c != kLeafChar) {
          node_queue.emplace(index, nodes[i]);
        }
      }
    }
    head_.push_back(head_[depth+1] + max_height);
#ifndef NDEBUG
    auto used = SuccinctBitVector<false>(empty_bv).rank_0(empty_bv.size());
    double per_used = double(used) / empty_bv.size() * 100;
    std::cerr << "used: " << std::fixed << std::setprecision(2) << " %" << per_used << std::endl << std::endl;
#endif
  }

  // Store character to storage_
  storage_.resize(head_.back(), kEmptyChar);
  for (auto i_c : storage_map)
    storage_[i_c.first] = i_c.second;

}

template <typename CodeType>
template <typename T, typename S>
_SamcImpl<CodeType>::_SamcImpl(const graph_util::Trie<T, S>& trie) {
  std::vector<size_t> node_indexes = {graph_util::kRootIndex};
  storage_.emplace_back('^'); // root
  head_.emplace_back(0);
  position_type max_index = 0;
  while (max_index >= 0) {
#ifndef NDEBUG
    std::cerr << "depth: " << head_.size()
              << ", block_height: " << max_index << std::endl;
#endif
    std::array<std::vector<size_t>, kAlphabetSize> indices_list;
    size_t height = max_index + 1;
    for (size_t i = 0; i < height; i++) {
      auto index = storage_.size() - height + i;
      if (storage_[index] == kEmptyChar)
        continue;
      trie.node(node_indexes[index]).for_each_edge([&](uint8_t c, auto) {
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
      assert(height + y_front <= std::numeric_limits<code_type>::max());
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
    head_.push_back(storage_.size() - 1);
#ifndef NDEBUG
    auto used = SuccinctBitVector<false>(empties).rank_0(empties.size());
    double per_used = double(used) / empties.size() * 100;
    std::cerr << "used: " << std::fixed << std::setprecision(2) << " %" << per_used << std::endl << std::endl;
#endif
  }
}

template <typename CodeType>
typename _SamcImpl<CodeType>::position_type
_SamcImpl<CodeType>::y_check_(const std::vector<size_t>& indices, const BitVector& empties) const {
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

template <typename CodeType>
typename _SamcImpl<CodeType>::position_type
_SamcImpl<CodeType>::y_check_legacy_(const std::vector<size_t>& indices, const BitVector& empties) const {
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

template <typename CodeType>
size_t
_SamcImpl<CodeType>::shifts_of_conflicts_(mask_type fields, mask_type mask, mask_type conflicts) const {
  assert((fields & mask) == conflicts);
  using bit_util::ctz, bit_util::clz;
  auto conflict_front = ctz(conflicts);
  auto field_continuouses = ctz(~(fields >> conflict_front));
  auto mask_followings = clz(~(mask << (kMaskWidth-1 - conflict_front))) - 1;
  return field_continuouses + mask_followings;
}


template <typename CodeType>
class Samc : _SamcImpl<CodeType> {
public:
    using code_type = CodeType;
    using _base = _SamcImpl<code_type>;
    template <typename T, typename S>
    using input_trie = graph_util::Trie<T, S>;

    static constexpr uint8_t kLeafChar = '\0';

public:
    Samc() = default;

    template <typename StrIter>
    explicit Samc(StrIter begin, StrIter end) : _base(begin, end) {}

    template <typename T, typename S>
    [[deprecated("Maybe consume much memory. You should construct from string-iterator like (begin end)")]]
    explicit Samc(const input_trie<T, S>& trie) : _base(trie) {}

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
      return _base::head(depth) <= index and index < _base::head(depth+1);
    }

    bool empty(size_t index) const {return _base::check(index) == _base::kEmptyChar;}

};

template <typename CodeType>
bool
Samc<CodeType>::accept(std::string_view key) const {
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

template <typename CodeType>
class _SamcDictImpl : protected _SamcImpl<CodeType> {
    using code_type = CodeType;
    using _base = _SamcImpl<code_type>;

public:
    static constexpr uint8_t kLeafChar = graph_util::kLeafChar;

protected:
    using succinct_bv_type = SuccinctBitVector<true>;
    succinct_bv_type leaves_;

public:
    _SamcDictImpl() : _base() {
      _setup_leaves();
    }

    template <typename StrIter>
    explicit _SamcDictImpl(StrIter begin, StrIter end) : _base(begin, end) {
      _setup_leaves();
    }

    template <typename T, typename S>
    explicit _SamcDictImpl(const graph_util::Trie<T, S>& trie) : _base(trie) {
      _setup_leaves();
    }

    void _setup_leaves() {
      BitVector leaves_src(_base::storage_.size());
      for (size_t i = 1; i < _base::storage_.size(); i++) {
        if (_base::check(i) == kLeafChar)
          leaves_src[i] = true;
      }
      leaves_ = succinct_bv_type(leaves_src);
    }

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


template <typename CodeType>
class SamcDict : _SamcDictImpl<CodeType> {
public:
    using code_type = CodeType;
    using _base = _SamcDictImpl<code_type>;
    template <typename T, typename S>
    using input_trie = graph_util::Trie<T, S>;

    static constexpr size_t kSearchError = -1;

    SamcDict() = default;

    template <typename StrIter>
    explicit SamcDict(StrIter begin, StrIter end) : _base(begin, end) {}

    template <typename T, typename S>
    explicit SamcDict(const input_trie<T, S>& trie) : _base(trie) {}

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
      return _base::head(depth) <= index and index < _base::head(depth+1);
    }

    bool empty(size_t index) const {return _base::check(index) == _base::kEmptyChar;}

};

template <typename CodeType>
size_t
SamcDict<CodeType>::lookup(std::string_view key) const {
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

template <typename CodeType>
std::string
SamcDict<CodeType>::access(size_t value) const {
  std::string text;
  size_t node = _base::leaf(value);
  size_t depth = std::upper_bound(_base::head_.begin(), _base::head_.end(), node) - _base::head_.begin() - 1;
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
