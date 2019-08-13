//
// Created by 松本拓真 on 2019-08-10.
//

#ifndef DOUBLEARRAYPATRICIATRIE_HPP_
#define DOUBLEARRAYPATRICIATRIE_HPP_

#include "DoubleArrayMpTrie.hpp"

namespace sim_ds {


template <typename ValueType, typename IndexType, bool LetterCheck>
class _DoubleArrayBcPatriciaTrieBehavior : public _DoubleArrayMpTrieBehavior<ValueType, IndexType, LetterCheck> {
  using _base = _DoubleArrayMpTrieBehavior<ValueType, IndexType, LetterCheck>;
 public:
  using typename _base::_value_type;
  using typename _base::_value_pointer;
  using typename _base::_const_value_pointer;
  using typename _base::_index_type;
  using typename _base::_char_type;

  static constexpr bool kLetterCheck = LetterCheck;
  static constexpr bool kRootIndex = _base::kRootIndex;
  static constexpr bool kLeafChar = _base::kLeafChar;

  using _base::kIndexSize;

 public:
  _DoubleArrayBcPatriciaTrieBehavior() : _base() {}

  _DoubleArrayBcPatriciaTrieBehavior(_DoubleArrayBcPatriciaTrieBehavior&&) noexcept = default;
  _DoubleArrayBcPatriciaTrieBehavior& operator=(_DoubleArrayBcPatriciaTrieBehavior&&) noexcept = default;

  virtual ~_DoubleArrayBcPatriciaTrieBehavior() = default;

  template <class SuccessAction, class FailedInBcAction, class FailedInInternalLabelAction, class FailedInSuffixAction>
  std::pair<_value_pointer, bool>
  Traverse(std::string_view key,
           SuccessAction success,
           FailedInBcAction failed_in_bc,
           FailedInInternalLabelAction failed_in_internal_label,
           FailedInSuffixAction failed_in_suffix) {
    if (_base::unit_at(kRootIndex).is_leaf()) {
      // First insertion
      return {failed_in_bc(kRootIndex, 0), false};
    }
    _index_type node = kRootIndex;
    _index_type base = _base::base_at(node);
    for (size_t key_pos = 0; key_pos < key.size(); key_pos++) {
      if (not TransitionBc(node, base, key[key_pos])) {
        return {failed_in_bc(node, key_pos), false};
      }
      auto unit = _base::unit_at(node);
      if (unit.has_label()) {
        if (not unit.label_is_suffix()) {
          auto [nbase, ptr, res] = TransitionInternalLabel(node, key, ++key_pos, failed_in_internal_label);
          if (not res)
            return {ptr, false};
          base = nbase;
        } else {
          auto [ptr, res] =  _base::TransitionSuffix(node, key, ++key_pos, failed_in_suffix);
          if (not res)
            return {ptr, false};
          success(node);
          return {ptr, true};
        }
      } else {
        base = unit.base();
      }
    }
    if (not TransitionBc(node, base, kLeafChar)) {
      return {failed_in_bc(node, key.size()), false};
    }
    success(node);
    return {_base::value_ptr_in_pool_at(_base::unit_at(node).pool_index()), true};
  }

  template <class SuccessAction, class FailedInBcAction, class FailedInInternalLabelAction, class FailedInSuffixAction>
  std::pair<_const_value_pointer, bool>
  Traverse(std::string_view key,
           SuccessAction success,
           FailedInBcAction failed_in_bc,
           FailedInInternalLabelAction failed_in_internal_label,
           FailedInSuffixAction failed_in_suffix) const {
    if (_base::unit_at(kRootIndex).is_leaf()) {
      // First insertion
      return {failed_in_bc(kRootIndex, 0), false};
    }
    _index_type node = kRootIndex;
    _index_type base = _base::base_at(node);
    for (size_t key_pos = 0; key_pos < key.size(); key_pos++) {
      if (not TransitionBc(node, base, key[key_pos])) {
        return {failed_in_bc(node, key_pos), false};
      }
      auto unit = _base::unit_at(node);
      if (unit.has_label()) {
        if (not unit.label_is_suffix()) {
          auto [nbase, ptr, res] = TransitionInternalLabel(node, key, ++key_pos, failed_in_internal_label);
          if (not res)
            return {ptr, false};
          base = nbase;
        } else {
          auto [ptr, res] =  _base::TransitionSuffix(node, key, ++key_pos, failed_in_suffix);
          if (not res)
            return {ptr, false};
          success(node);
          return {ptr, true};
        }
      } else {
        base = unit.base();
      }
    }
    if (not TransitionBc(node, base, kLeafChar)) {
      return {failed_in_bc(node, key.size()), false};
    }
    success(node);
    return {_base::value_ptr_in_pool_at(_base::unit_at(node).pool_index()), true};
  }

  bool TransitionBc(_index_type &node, _index_type base, _char_type c) const {
    auto next = base xor c;
    auto unit = _base::unit_at(next);
    if (unit.check_empty() or
        unit.check() != (kLetterCheck ?  c : node))
      return false;
    node = next;
    return true;
  }

  template <class FailedAction>
  std::tuple<_index_type, _value_pointer, bool>
  TransitionInternalLabel(_index_type node,
                          std::string_view key,
                          size_t &key_pos,
                          FailedAction failed) {
    auto pool_index = _base::unit_at(node).pool_index();
    auto base = _base::target_in_pool_at(pool_index);
    auto label_index = pool_index + kIndexSize;
    auto pool_ptr = (_char_type*)_base::pool_ptr_at(label_index);
    assert(*pool_ptr != kLeafChar);
    size_t i = 0;
    while (key_pos < key.size()) {
      _char_type char_in_label = *pool_ptr;
      if (char_in_label == kLeafChar) {
        --key_pos;
        return {base, nullptr, true};
      }
      if (char_in_label != (_char_type)key[key_pos]) {
        return {base, failed(node, label_index+i, key_pos), false};
      }
      ++pool_ptr;
      i++;
      key_pos++;
    }
    if (*pool_ptr != kLeafChar) {
      return {base, failed(node, label_index+i, key_pos), false};
    }
    --key_pos;
    return {base, nullptr, true};
  }

  template <class FailedAction>
  std::tuple<_index_type, _const_value_pointer, bool>
  TransitionInternalLabel(_index_type node,
                          std::string_view key,
                          size_t &key_pos,
                          FailedAction failed) const {
    auto pool_index = _base::unit_at(node).pool_index();
    auto base = _base::target_in_pool_at(pool_index);
    auto label_index = pool_index + kIndexSize;
    auto pool_ptr = (const _char_type*)_base::pool_ptr_at(label_index);
    assert(*pool_ptr != kLeafChar);
    size_t i = 0;
    while (key_pos < key.size()) {
      _char_type char_in_label = *pool_ptr;
      if (char_in_label == kLeafChar) {
        --key_pos;
        return {base, nullptr, true};
      }
      if (char_in_label != (_char_type)key[key_pos]) {
        return {base, failed(node, label_index+i, key_pos), false};
      }
      ++pool_ptr;
      i++;
      key_pos++;
    }
    if (*pool_ptr != kLeafChar) {
      return {base, failed(node, label_index+i, key_pos), false};
    }
    --key_pos;
    return {base, nullptr, true};
  }

//  template <class FailedAction>
//  std::pair<_value_pointer, bool>
//  TransitionSuffix(_index_type node,
//                   std::string_view key,
//                   size_t &key_pos,
//                   FailedAction failed) {
//    auto label_index = _base::unit_at(node).pool_index();
//    auto pool_ptr = (_char_type*)_base::pool_ptr_at(label_index);
//    size_t i = 0;
//    while (key_pos < key.size()) {
//      _char_type char_in_label = *pool_ptr;
//      if (char_in_label == kLeafChar or
//          char_in_label != (_char_type)key[key_pos]) {
//        return {failed(node, label_index+i, key_pos), false};
//      }
//      ++pool_ptr;
//      i++;
//      key_pos++;
//    }
//    if (*pool_ptr != kLeafChar) {
//      return {failed(node, label_index+i, key_pos), false};
//    }
//    --key_pos;
//    return {reinterpret_cast<_value_pointer>(pool_ptr+1), true};
//  }
//
//  template <class FailedAction>
//  std::pair<_const_value_pointer, bool>
//  TransitionSuffix(_index_type node,
//                   std::string_view key,
//                   size_t &key_pos,
//                   FailedAction failed) const {
//    auto label_index = _base::unit_at(node).pool_index();
//    auto pool_ptr = (const _char_type*)_base::pool_ptr_at(label_index);
//    size_t i = 0;
//    while (key_pos < key.size()) {
//      _char_type char_in_label = *pool_ptr;
//      if (char_in_label == kLeafChar or
//          char_in_label != (_char_type)key[key_pos]) {
//        return {failed(node, label_index+i, key_pos), false};
//      }
//      ++pool_ptr;
//      i++;
//      key_pos++;
//    }
//    if (*pool_ptr != kLeafChar) {
//      return {failed(node, label_index+i, key_pos), false};
//    }
//    --key_pos;
//    return {reinterpret_cast<_const_value_pointer>(pool_ptr+1), true};
//  }

};



template <typename ValueType, typename IndexType, bool LetterCheck, bool Ordered, size_t MaxTrial, bool BitOperationalFind>
class _DynamicDoubleArrayPatriciaTrieConstructor : public _DoubleArrayMpTrieConstructor<ValueType, IndexType, LetterCheck, Ordered, MaxTrial, BitOperationalFind> {
 public:
  using _da_trie = _DoubleArrayBcPatriciaTrieBehavior<ValueType, IndexType, LetterCheck>;
  using _base = _DoubleArrayMpTrieConstructor<ValueType, IndexType, LetterCheck, Ordered, MaxTrial, BitOperationalFind>;
  using typename _base::_value_type;
  using typename _base::_value_pointer;
  using typename _base::_const_value_pointer;
  using typename _base::_index_type;
  using typename _base::_char_type;
  static constexpr bool kLetterCheck = _da_trie::kLetterCheck;
  using _base::kUseUniqueBase;

  using _base::kValueBytes;
  using _base::kIndexBytes;
  using _base::kLeafChar;
  using _base::kEmptyFlag;
  using _base::kEmptyChar;

  using _base::da_;
  using typename _base::Index2;

 public:
  explicit _DynamicDoubleArrayPatriciaTrieConstructor(_da_trie& da) : _base(da) {}

  virtual ~_DynamicDoubleArrayPatriciaTrieConstructor() = default;

  _value_type* InsertInSuffix(_index_type node, _index_type label_pos, std::string_view additional_suffix) {
    assert(da_.unit_at(node).has_label() and
        da_.unit_at(node).label_is_suffix());
    auto forked_base = _base::base_finder_.get({da_.char_in_pool_at(label_pos), (_char_type) additional_suffix.front()});
    _ForkInSuffix(node, label_pos, forked_base);
    assert(not da_.unit_at(node).is_leaf());
    return _base::InsertInBc(node, additional_suffix);
  }

  _value_type* InsertInInternalLabel(_index_type node, _index_type label_pos, std::string_view additional_suffix) {
    assert(da_.unit_at(node).has_label() and
        not da_.unit_at(node).label_is_suffix());
    auto forked_base = _base::base_finder_.get({da_.char_in_pool_at(label_pos), (_char_type) additional_suffix.front()});
    _ForkInInternalLabel(node, label_pos, forked_base);
    return _base::InsertInBc(node, additional_suffix);
  }

  void InsertNodes(_index_type node, std::vector<typename _base::_InternalLabelContainer>& label_datas, _index_type base) override {
    da_.ExpandIfNeeded(base);
    _base::_SetNewEdge(node, base, label_datas.front().label.empty() ? kLeafChar : label_datas.front().label.front());
    for (size_t i = 0; i < label_datas.size(); i++) {
      auto cur_label = label_datas[i].label;
      _char_type c = cur_label.empty() ? kLeafChar : cur_label.front();
      auto next = base xor c;
      assert(da_.unit_empty_at(next));
      auto sibling = i+1 < label_datas.size() ? label_datas[i+1].label.front() : kEmptyChar;
      if (label_datas[i].suffix) {
        if (cur_label.empty()) {
          if constexpr (kLetterCheck) {
            _base::_SetNewNodeWithLabel(next, c, sibling, da_.pool_size(), true);
          } else {
            _base::_SetNewNodeWithLabel(next, node, sibling, da_.pool_size(), true);
          }
          da_.AppendEmptyValue();
        } else {
          auto pool_index = da_.AppendLabelInPool(cur_label.substr(1));
          da_.AppendEmptyValue();
          if constexpr (kLetterCheck) {
            _base::_SetNewNodeWithLabel(next, c, sibling, pool_index, true);
          } else {
            _base::_SetNewNodeWithLabel(next, node, sibling, pool_index, true);
          }
        }
      } else {
        if (cur_label.size() == 1) {
          if constexpr (kLetterCheck) {
            _base::_SetNewNode(next, c, sibling);
          } else {
            _base::_SetNewNode(next, node, sibling);
          }
        } else {
          auto pool_index = _AppendInternalLabelInPool(cur_label.substr(1), kEmptyFlag);
          if constexpr (kLetterCheck) {
            _base::_SetNewNodeWithLabel(next, c, sibling, pool_index, false);
          } else {
            _base::_SetNewNodeWithLabel(next, node, sibling, pool_index, false);
          }
        }
      }
    }
    _base::_ConsumeBlock(Index2(base).block_index, label_datas.size());
  }

  template <class SrcDa>
  void ArrangeDa(const SrcDa &src_da, const _index_type node, const _index_type self_node) {
    std::vector<typename _base::_InternalLabelContainer> label_datas;
    std::vector<_char_type> children;
    src_da.for_each_children(node, [&](_index_type index, _char_type child) {
      children.push_back(child);
      std::string label;
      label.push_back(child);
      auto unit = src_da.unit_at(index);
      if (unit.has_label())
        label += src_da.label_in_pool(unit.pool_index()+(unit.label_is_suffix()?0:_da_trie::kIndexSize));
      while (src_da.single_node_at(index)) {
        auto base = src_da.base_at(index);
        auto child = src_da.unit_at(base).child();
        label += child;
        src_da.TransitionBc(index, base, child);
        unit = src_da.unit_at(index);
        if (unit.has_label())
          label += src_da.label_in_pool(unit.pool_index()+(unit.label_is_suffix()?0:_da_trie::kIndexSize));
      }
      label_datas.push_back({label, unit.label_is_suffix()});
    });

    auto new_base = _base::base_finder_.get(children);
    InsertNodes(self_node, label_datas, new_base);
    for (auto label_data : label_datas) {
      auto target = node;
      uint8_t c = label_data.label.front();
      auto res = src_da.TransitionBc(target, src_da.base_at(target), c);
      assert(res);
      if (not label_data.suffix)
        ArrangeDa(src_da, target, new_base xor c);
    }
  }

  template <typename StrIter,
      typename Traits = std::iterator_traits<StrIter>>
  void ArrangeKeysets(StrIter begin, StrIter end, size_t depth, _index_type co_node) {
    if (begin >= end)
      return;

    std::vector<typename _base::_InternalLabelContainer> label_datas;
    std::vector<_char_type> children;
    if ((*begin).size() == depth) {
      label_datas.push_back({"", true});
      children.push_back(kLeafChar);
      ++begin;
    }
    if (begin == end) {
      InsertNodes(co_node, label_datas, _base::base_finder_.get(children));
    } else {
      assert(begin < end);
      assert(begin->size() > depth);
      std::vector<StrIter> iters = {begin};
      std::string_view front_label(begin->data() + depth);
      _char_type prev_key = begin->size() <= depth ? kLeafChar : (*begin)[depth];
      auto append = [&](auto it) {
        size_t label_length = 1;
        std::string_view back_label(it->data() + depth);
        while (label_length < front_label.size() and label_length < back_label.size() and
            back_label[label_length] == front_label[label_length])
          label_length++;
        label_datas.push_back({std::string(front_label.substr(0, label_length)), label_length == back_label.size()});
        children.push_back(prev_key);
        iters.push_back(it+1);
      };
      for (auto it = begin+1; it < end; ++it) {
        _char_type c = (*it)[depth];
        if (c != prev_key) {
          append(it-1);
          front_label = std::string_view(it->data() + depth);
          prev_key = c;
        }
      }
      append(end-1);
      auto new_base = _base::base_finder_.get(children);
      InsertNodes(co_node, label_datas, new_base);
      for (size_t i = 0; i < iters.size()-1; i++) {
        auto& ld = label_datas[i+(children.front()==kLeafChar?1:0)];
        auto label = ld.label;
        if (not ld.suffix)
          ArrangeKeysets(iters[i], iters[i + 1], depth + label.size(), new_base xor (_char_type) label.front());
      }
    }

  }

 private:
  _index_type _AppendInternalLabelInPool(std::string_view label, _index_type new_base) {
    auto index = da_.AppendTarget(new_base);
    da_.AppendLabelInPool(label);
    return index;
  }

  void _ForkInSuffix(_index_type node, _index_type label_pos, _index_type forked_base) {
    assert(da_.unit_at(node).has_label() and
        da_.unit_at(node).label_is_suffix());
    _char_type char_at_confliction = da_.char_in_pool_at(label_pos);

    auto label_index = da_.unit_at(node).pool_index();
    auto left_label_length = label_pos - label_index;
    std::vector<_char_type> cs;
    for (int i = 0; i < left_label_length; i++)
      cs.push_back(da_.char_in_pool_at(label_index+i));
    if (left_label_length == 0) {
      da_.unit_at(node).set_base(forked_base);
    } else {
      std::string left_label((char*)da_.pool_ptr_at(label_index), left_label_length);
      auto relay_pool_index = _AppendInternalLabelInPool(left_label, forked_base);
      da_.unit_at(node).set_pool_index(relay_pool_index, false);
    }
    assert(not da_.unit_at(node).is_leaf());
    if constexpr (kLetterCheck) {
      _base::_UseBaseAt(forked_base, char_at_confliction);
    } else {
      da_.unit_at(node).set_child(char_at_confliction);
    }
    auto relay_next = forked_base xor char_at_confliction;
    if constexpr (kLetterCheck) {
      _base::_SetNewNodeWithLabel(relay_next, char_at_confliction, kEmptyChar, label_pos + 1, true);
    } else {
      _base::_SetNewNodeWithLabel(relay_next, node, kEmptyChar, label_pos + 1, true);
    }
    _base::_ConsumeBlock(Index2(relay_next).block_index, 1);
  }

  void _ForkInInternalLabel(_index_type node, _index_type label_pos, _index_type forked_base) {
    assert(da_.unit_at(node).has_label() and
        not da_.unit_at(node).label_is_suffix());
    _char_type char_at_confliction = da_.char_in_pool_at(label_pos);
    assert(char_at_confliction != kLeafChar);
    const auto pool_index = da_.unit_at(node).pool_index();
    const auto node_base = da_.target_in_pool_at(pool_index);
    const auto node_child = da_.unit_at(node).child(); // Use at last.

    auto label_index = pool_index + kIndexBytes;
    auto left_label_length = label_pos - label_index;
    auto right_label_length = 0;
    bool small_prefix = false;
    while (not small_prefix and da_.char_in_pool_at(label_pos + right_label_length) != kLeafChar) {
      small_prefix = left_label_length < ++right_label_length;
    }
    if constexpr (kUseUniqueBase) {
      _base::_UseBaseAt(forked_base, char_at_confliction);
    } else {
      da_.unit_at(node).set_child(char_at_confliction);
    }
    auto relay_next = forked_base xor char_at_confliction;
    if (small_prefix) {
      // ||*|*|*||*|*|*|$||
      //  |→   ←| conflict between this area
      if (left_label_length == 0) {
        da_.unit_at(node).set_base(forked_base);
      } else {
        std::string left_label((char*)da_.pool_ptr_at(label_index), left_label_length);
        auto relay_pool_index = _AppendInternalLabelInPool(left_label, forked_base);
        da_.unit_at(node).set_pool_index(relay_pool_index, false);
      }
      if (da_.char_in_pool_at(label_pos + 1) == kLeafChar) { // Length of right-label is 1.
        if constexpr (kLetterCheck) {
          _base::_SetNewNode(relay_next, char_at_confliction);
        } else {
          _base::_SetNewNode(relay_next, node);
        }
        da_.unit_at(relay_next).set_base(node_base);
      } else {
        if constexpr (kLetterCheck) {
          _base::_SetNewNodeWithLabel(relay_next, char_at_confliction, kEmptyChar, label_pos + 1 - kIndexBytes, false);
        } else {
          _base::_SetNewNodeWithLabel(relay_next, node, kEmptyChar, label_pos + 1 - kIndexBytes, false);
        }
        da_.set_target_in_pool_at(label_pos+1-kIndexBytes, node_base);
      }
    } else {
      // ||*|*|*||*|*|*|$||
      //         |→   ←| conflict between this area
      da_.set_char_in_pool_at(label_pos, kLeafChar);
      da_.set_target_in_pool_at(pool_index, forked_base);
      if (right_label_length == 1) {
        if constexpr (kLetterCheck) {
          _base::_SetNewNode(relay_next, char_at_confliction, kEmptyChar);
        } else {
          _base::_SetNewNode(relay_next, node, kEmptyChar);
        }
        da_.unit_at(relay_next).set_base(node_base);
      } else {
        std::string right_label((char*)da_.pool_ptr_at(label_pos + 1));
        auto relay_pool_index = _AppendInternalLabelInPool(right_label, node_base);
        if constexpr (kLetterCheck) {
          _base::_SetNewNodeWithLabel(relay_next, char_at_confliction, kEmptyChar, relay_pool_index, false);
        } else {
          _base::_SetNewNodeWithLabel(relay_next, node, kEmptyChar, relay_pool_index, false);
        }
      }
    }
    if constexpr (not kLetterCheck) {
      da_.unit_at(relay_next).set_child(node_child);
      _base::_UpdateCheck(node_base, node_child, relay_next);
    }
    _base::_ConsumeBlock(Index2(relay_next).block_index, 1);
  }

};


template <typename ValueType, typename IndexType, bool LetterCheck, bool Ordered, size_t MaxTrial, bool LegacyBuild>
class DoubleArrayPatriciaTrie : public _DoubleArrayBcPatriciaTrieBehavior<ValueType, IndexType, LetterCheck> {
  using _self = DoubleArrayPatriciaTrie<ValueType, IndexType, LetterCheck, Ordered, MaxTrial, LegacyBuild>;
  using _behavior = _DoubleArrayBcPatriciaTrieBehavior<ValueType, IndexType, LetterCheck>;

  using _constructor = _DynamicDoubleArrayPatriciaTrieConstructor<ValueType, IndexType, LetterCheck, Ordered, MaxTrial, not LegacyBuild>;

 private:
  _constructor constructor_;

 public:
  using value_type = typename _behavior::_value_type;
  using value_pointer = typename _behavior::_value_pointer;
  using const_value_pointer = typename _behavior::_const_value_pointer;
  using index_type = typename _behavior::_index_type;
  using char_type = typename _behavior::_char_type;

  static constexpr index_type kRootIndex = _behavior::kRootIndex;
  static constexpr char_type kLeafChar = _behavior::kLeafChar;

  static constexpr size_t kBlockSize = _behavior::kNumUnitsPerBlock;

  DoubleArrayPatriciaTrie() : _behavior(), constructor_(*this) {
    constructor_.init();
  }

  explicit DoubleArrayPatriciaTrie(std::ifstream& ifs) : _behavior(ifs), _constructor(*this) {}

  template <typename StrIter,
      typename Traits = std::iterator_traits<StrIter>>
  DoubleArrayPatriciaTrie(StrIter begin, StrIter end) : DoubleArrayPatriciaTrie() {
    constructor_.ArrangeKeysets(begin, end, 0, kRootIndex);
  }

  explicit DoubleArrayPatriciaTrie(const std::vector<std::string>& key_set) : DoubleArrayPatriciaTrie(key_set.begin(), key_set.end()) {}

  ~DoubleArrayPatriciaTrie() = default;

  template <bool _Ordered, size_t _MaxTrial, bool _LegacyBuild>
  void build_from(const DoubleArrayPatriciaTrie<ValueType, IndexType, LetterCheck, _Ordered, _MaxTrial, _LegacyBuild>& da) {
    constructor_.ArrangeDa(da, kRootIndex, kRootIndex);
  }

  void rebuild() {
    DoubleArrayPatriciaTrie<ValueType, IndexType, LetterCheck, Ordered, 32, LegacyBuild> new_da;
    new_da.build_from(*this);
    (_behavior&)*this = std::move((_behavior&)new_da);
  }

  void shrink_to_fit() {
    rebuild();
  }

  value_pointer find(std::string_view key) {
    return _behavior::Traverse(key, [](auto) {},
                               [](auto, auto) { return nullptr; },
                               [](auto, auto, auto) { return nullptr; },
                               [](auto, auto, auto) { return nullptr; }).first;
  }

  const_value_pointer find(std::string_view key) const {
    return _behavior::Traverse(key, [](auto) {},
                               [](auto, auto) { return nullptr; },
                               [](auto, auto, auto) { return nullptr; },
                               [](auto, auto, auto) { return nullptr; }).first;
  }

  std::pair<value_type*, bool> insert(std::string_view key) {
    auto [ptr, res] = _behavior::Traverse(key, [](auto) {},
                                          [&](index_type node, size_t key_pos) {
                                            return constructor_.InsertInBc(node, key.substr(key_pos));
                                          },
                                          [&](index_type node, size_t label_pos, size_t key_pos) {
                                            return constructor_.InsertInInternalLabel(node, label_pos, key.substr(key_pos));
                                          },
                                          [&](index_type node, size_t label_pos, size_t key_pos) {
                                            return constructor_.InsertInSuffix(node, label_pos, key.substr(key_pos));
                                          });
    return {ptr, not res};
  }

  std::pair<value_type*, bool> insert(std::string_view key, value_type value) {
    auto pair = insert(key);
    if (pair.second)
      *(pair.first) = value;
    return pair;
  }

  bool erase(std::string_view key) {
    return _behavior::Traverse(key, [&](index_type node) {
                                 constructor_.DeleteLeaf(node);
                               },
                               [](auto, auto) { return nullptr; },
                               [](auto, auto, auto) { return nullptr; },
                               [](auto, auto, auto) { return nullptr; }).second;
  }

};

}

#endif //DOUBLEARRAYPATRICIATRIE_HPP_
