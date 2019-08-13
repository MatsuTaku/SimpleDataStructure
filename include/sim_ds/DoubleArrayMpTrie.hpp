//
// Created by 松本拓真 on 2019-08-10.
//

#ifndef DOUBLEARRAYMPTRIE_HPP_
#define DOUBLEARRAYMPTRIE_HPP_

#include "DoubleArrayTrieDictImpl.hpp"
#include "DoubleArrayBaseFinder.hpp"

namespace sim_ds {


// MARK: - MPTrie


template <typename ValueType, typename IndexType, bool LetterCheck>
class _DoubleArrayMpTrieBehavior : public _DoubleArrayTrieDictImpl<ValueType, IndexType, LetterCheck> {
 public:
  using _self = _DoubleArrayMpTrieBehavior<ValueType, IndexType, LetterCheck>;
  using _base = _DoubleArrayTrieDictImpl<ValueType, IndexType, LetterCheck>;
  using _value_type = ValueType;
  using _value_pointer = typename _base::_value_pointer;
  using _const_value_pointer = typename _base::_const_value_pointer;
  using _index_type = IndexType;
  using _char_type = uint8_t;
  using _inset_type = uint8_t;
  static constexpr bool kLetterCheck = LetterCheck;

  static constexpr _index_type kRootIndex = 0;
  static constexpr _char_type kLeafChar = _base::kEndChar;
  static constexpr _char_type kEmptyChar = _base::kEmptyChar;

  static constexpr _index_type kEmptyFlag = _base::kEmptyFlag;

  static constexpr unsigned kNumUnitsPerBlock = _base::kNumUnitsPerBlock;

  using Index2 = typename _base::Index2;

 protected:
  _DoubleArrayMpTrieBehavior() : _base() {}

 public:
  _DoubleArrayMpTrieBehavior(_DoubleArrayMpTrieBehavior&&) noexcept = default;
  _DoubleArrayMpTrieBehavior& operator=(_DoubleArrayMpTrieBehavior&&) noexcept = default;

  virtual ~_DoubleArrayMpTrieBehavior() = default;

  // MARK: Basic information

  void print_for_debug() const {
    std::cout << "------------ Double-array implementation ------------" << std::endl;
    std::cout << "\tindex] \texists, \tcheck, \tsibling, \tbase, \tchild"  << std::endl;
    for (size_t i = 0; i < std::min(_base::unit_size(), (size_t)0x10000); i++) {
      if (i % 0x100 == 0)
        std::cout << std::endl;
      auto unit = _base::unit_at(i);
      std::cout << "\t\t"<<i<<"] \t";
      if constexpr (kLetterCheck) std::cout<< unit.child()<<"| \t";
      if (not _base::unit_empty_at(i)) {
        std::cout<<unit.check()<<", \t"<<unit.sibling();
        if (not unit.is_leaf()) {
          std::cout<<", \t"<< _base::base_at(i);
          if constexpr (not kLetterCheck) std::cout<<"| \t"<< unit.child();
        }
      } else {
//        std::cout<<(size_t)unit.pred()<<", \t"<<(size_t)unit.succ();
      }
      std::cout << std::endl;
    }
  }

  void make_table(_index_type node, std::array<size_t, 257>& table) const {
    if (_base::unit_at(node).is_leaf())
      return;
    auto cnt = 0;
    for_each_children(node, [&](_index_type index, auto) {
      make_table(index, table);
      ++cnt;
    });
    table[cnt]++;
  }

  std::array<size_t, 257> get_num_of_children_table() const {
    std::array<size_t, 257> table = {};
    make_table(kRootIndex, table);
    return table;
  }

  void print_num_of_children_table() const {
    auto table = get_num_of_children_table();
    std::cout << "num of children | counts" << std::endl;
    for (int i = 0; i <= 256; i++)
      std::cout << table[i] << std::endl;
  }

  // MARK: For construction

  template <class Action>
  void for_each_children(_index_type node, Action action) const {
    assert(not _base::unit_at(node).is_leaf());
    auto base = _base::base_at(node);
    _char_type child;
    if constexpr (LetterCheck) {
      child = _base::unit_at(base).child();
    } else {
      child = _base::unit_at(node).child();
    }
    while (child != kEmptyChar) {
      auto next = base xor child;
      assert(not _base::unit_empty_at(next));
      assert(_base::unit_at(next).check() == (kLetterCheck ? child : node));
      auto sibling = _base::unit_at(next).sibling();
      action(next, child);
      child = sibling;
    }
  }

  size_t num_of_children_at(_index_type node) const {
    if (_base::unit_at(node).is_leaf())
      return 0;
    size_t cnt = 0;
    for_each_children(node, [&cnt](auto, auto) { ++cnt; });
    return cnt;
  }

  bool single_node_at(_index_type node) const {
    return num_of_children_at(node) == 1;
  }

  template <class SuccessAction, class FailedInBcAction, class FailedInSuffixAction>
  std::pair<_value_pointer, bool>
  Traverse(std::string_view     key,
           SuccessAction        success,
           FailedInBcAction     failed_in_bc,
           FailedInSuffixAction failed_in_suffix) {
    _index_type node = kRootIndex;
    size_t key_pos = 0;
    for (; key_pos < key.size(); key_pos++) {
      if (not TransitionBc(node, key[key_pos])) {
        return {failed_in_bc(node, key_pos), false};
      }
      if (_base::unit_at(node).has_label()) {
        auto [ptr, res] = TransitionSuffix(node, key, ++key_pos, failed_in_suffix);
        if (not res) {
          return {ptr, false};
        }
        success(node);
        return {ptr, true};
      }
    }
    if (not TransitionBc(node, kLeafChar)) {
      return {failed_in_bc(node, key_pos), false};
    }
    success(node);
    return {_base::value_ptr_in_pool_at(_base::unit_at(node).pool_index()), true};
  }

  template <class SuccessAction, class FailedInBcAction, class FailedInSuffixAction>
  std::pair<_const_value_pointer, bool>
  Traverse(std::string_view key,
           SuccessAction success,
           FailedInBcAction failed_in_bc,
           FailedInSuffixAction failed_in_suffix) const {
    _index_type node = kRootIndex;
    size_t key_pos = 0;
    for (; key_pos < key.size(); key_pos++) {
      if (not TransitionBc(node, key[key_pos])) {
        return {failed_in_bc(node, key_pos), false};
      }
      if (_base::unit_at(node).has_label()) {
        auto [ptr, res] = TransitionSuffix(node, key, ++key_pos, failed_in_suffix);
        if (not res) {
          return {ptr, false};
        }
        success(node);
        return {ptr, true};
      }
    }
    if (not TransitionBc(node, kLeafChar)) {
      return {failed_in_bc(node, key_pos), false};
    }
    success(node);
    return {_base::value_ptr_in_pool_at(_base::unit_at(node).pool_index()), true};
  }

  bool TransitionBc(_index_type &node, _char_type c) const {
    if (_base::unit_at(node).is_leaf())
      return false;
    auto next = _base::base_at(node) xor c;
    if (_base::unit_at(next).check_empty() or
        _base::unit_at(next).check() != (kLetterCheck ? c : node))
      return false;
    node = next;
    return true;
  }

  template <class FailedAction>
  std::pair<_value_pointer, bool>
  TransitionSuffix(_index_type node,
                   std::string_view key,
                   size_t &key_pos,
                   FailedAction failed) {
    assert(_base::unit_at(node).has_label());
    auto pool_index = _base::unit_at(node).pool_index();
    auto pool_ptr = (_char_type*)_base::pool_ptr_at(pool_index);
    size_t i = 0;
    while (key_pos < key.size()) {
      _char_type char_in_tail = *pool_ptr;
      if (char_in_tail == kLeafChar or
          char_in_tail != (_char_type)key[key_pos]) {
        return {failed(node, pool_index+i, key_pos), false};
      }
      ++pool_ptr;
      i++;
      key_pos++;
    }
    if (*pool_ptr != kLeafChar) {
      return {failed(node, pool_index+i, key_pos), false};
    }
    key_pos--;
    return {reinterpret_cast<_value_pointer>(pool_ptr + 1), true};
  }

  template <class FailedAction>
  std::pair<_const_value_pointer, bool>
  TransitionSuffix(_index_type node,
                   std::string_view key,
                   size_t &key_pos,
                   FailedAction failed) const {
    assert(_base::unit_at(node).has_label());
    auto pool_index = _base::unit_at(node).pool_index();
    auto pool_ptr = (const _char_type*)_base::pool_ptr_at(pool_index);
    size_t i = 0;
    while (key_pos < key.size()) {
      _char_type char_in_tail = *pool_ptr;
      if (char_in_tail == kLeafChar or
          char_in_tail != (_char_type)key[key_pos]) {
        return {failed(node, pool_index+i, key_pos), false};
      }
      ++pool_ptr;
      i++;
      key_pos++;
    }
    if (*pool_ptr != kLeafChar) {
      return {failed(node, pool_index+i, key_pos), false};
    }
    key_pos--;
    return {reinterpret_cast<_const_value_pointer>(pool_ptr + 1), true};
  }

};


template <typename ValueType, typename IndexType, bool LetterCheck, bool Ordered, size_t MaxTrial, bool BitOperationalFind>
class _DoubleArrayMpTrieConstructor {
 public:
  using _self = _DoubleArrayMpTrieConstructor<ValueType, IndexType, LetterCheck, Ordered, MaxTrial, BitOperationalFind>;
  using _da_trie = _DoubleArrayMpTrieBehavior<ValueType, IndexType, LetterCheck>;
  using _value_type = typename _da_trie::_value_type;
  using _value_pointer = typename _da_trie::_value_pointer;
  using _const_value_pointer = typename _da_trie::_const_value_pointer;
  using _index_type = typename _da_trie::_index_type;
  using _char_type = typename _da_trie::_char_type;
  using _inset_type = typename _da_trie::_inset_type;
  using Index2 = typename _da_trie::Index2;
  static constexpr bool kLetterCheck = _da_trie::kLetterCheck;
  using check_type = std::conditional_t<kLetterCheck, _char_type, _index_type>;

  static constexpr bool kUseUniqueBase = _da_trie::kUseUniqueBase;
  static constexpr bool kUsePersonalLink = not kUseUniqueBase;
  static constexpr bool kLinkLayerIdGeneral = _da_trie::kLinkLayerIdGeneral;
  static constexpr bool kLinkLayerIdPersonal = _da_trie::kLinkLayerIdPersonal;

  static constexpr size_t kValueBytes = _da_trie::kValueBytes;
  static constexpr size_t kIndexBytes = _da_trie::kIndexSize;
  static constexpr _index_type kRootIndex = _da_trie::kRootIndex;
  static constexpr _index_type kEmptyFlag = _da_trie::kEmptyFlag;
  static constexpr _char_type kLeafChar = _da_trie::kLeafChar;
  static constexpr _char_type kEmptyChar = _da_trie::kEmptyChar;

  using _base_finder = std::conditional_t<not BitOperationalFind,
                                          DoubleArrayBaseFinderBasic<_da_trie, _self>,
                                          DoubleArrayBaseFinderBitOperational<_da_trie, _self>>;

 protected:
  _da_trie& da_;
  _base_finder base_finder_;
  
  friend typename _base_finder::_self;

  struct _MovingLuggage {
    bool is_leaf:1;
    bool has_label:1;
    bool label_is_suffix:1;
    _index_type target;
    _char_type child;
    _MovingLuggage(bool is_leaf, bool has_label, bool label_is_suffix, _index_type base, _char_type child) : is_leaf(is_leaf), has_label(has_label), label_is_suffix(label_is_suffix), target(base), child(child) {}
  };

  struct _Shelter {
    std::vector<_char_type> children;
    std::vector<_MovingLuggage> luggages;
  };

  struct _InternalLabelContainer {
    std::string label;
    bool suffix:1;
  };

 public:
  explicit _DoubleArrayMpTrieConstructor(_da_trie& da_impl) : da_(da_impl), base_finder_(da_, *this) {}

  virtual ~_DoubleArrayMpTrieConstructor() = default;

  void init() {
    if constexpr (kLetterCheck) {
      _SetNewNode(kRootIndex, kEmptyChar);
    } else {
      _SetNewNode(kRootIndex, kEmptyFlag);
    }
    _ConsumeBlock(Index2(kRootIndex).block_index, 1);
  }

  _index_type Grow(_index_type node, _index_type base, _char_type c) {
    assert(da_.unit_at(node).is_leaf());
    if constexpr (kUseUniqueBase) {
      _UseBaseAt(base, c);
    } else {
      da_.unit_at(node).set_child(c);
    }
    da_.unit_at(node).set_base(base);
    auto next = base xor c;
    if constexpr (kLetterCheck) {
      _SetNewNode(next, c);
    } else {
      _SetNewNode(next, node);
    }
    _ConsumeBlock(Index2(next).block_index, 1);
    return next;
  }

  _value_pointer InsertInBc(_index_type node, std::string_view additional_suffix) {
    if (not additional_suffix.empty()) {
      node = _InsertTrans(node, additional_suffix.front());
      auto pool_index = da_.AppendLabelInPool(additional_suffix.substr(1));
      da_.unit_at(node).set_pool_index(pool_index, true);
    } else {
      node = _InsertTrans(node, kLeafChar);
      da_.unit_at(node).set_pool_index(da_.pool_size(), true);
    }
    return da_.AppendEmptyValue();
  }

  _value_pointer InsertInTail(_index_type node, _index_type pool_pos, std::string_view additional_suffix) {
    assert(da_.unit_at(node).label_is_suffix());
    auto pool_index = da_.unit_at(node).pool_index();
    while (pool_index < pool_pos) {
      _char_type c = da_.char_in_pool_at(pool_index++);
      node = Grow(node, base_finder_.get({c}), c);
    }
    _char_type char_at_confliction = da_.char_in_pool_at(pool_pos);
    auto new_base = base_finder_.get({char_at_confliction,
      additional_suffix.empty() ? kLeafChar : (_char_type) additional_suffix.front()});
    auto next = Grow(node,
                     new_base,
                     char_at_confliction);
    assert(da_.base_at(node) == new_base);

    auto unit = da_.unit_at(next);
    unit.set_pool_index(pool_pos+1, true);
    return InsertInBc(node, additional_suffix);
  }

  virtual void InsertNodes(_index_type node, std::vector<_InternalLabelContainer> &label_datas, _index_type base) {
    _SetNewEdge(node, base,
                label_datas.front().label.empty() ? kLeafChar : label_datas.front().label.front());
    for (size_t i = 0; i < label_datas.size(); i++) {
      auto label = label_datas[i].label;
      _char_type c = label.empty() ? kLeafChar : label.front();
      auto next = base xor c;
      auto sibling = i+1<label_datas.size() ? label_datas[i+1].label.front() : kEmptyChar;
      if (label.empty()) {
        if constexpr (kLetterCheck) {
          _SetNewNodeWithLabel(next, c, sibling, da_.pool_size(), true);
        } else {
          _SetNewNodeWithLabel(next, node, sibling, da_.pool_size(), true);
        }
        da_.AppendEmptyValue();
      } else if (label_datas[i].suffix) {
        auto pool_index = da_.AppendLabelInPool(label.substr(1));
        da_.AppendEmptyValue();
        if constexpr (kLetterCheck) {
          _SetNewNodeWithLabel(next, c, sibling, pool_index, true);
        } else {
          _SetNewNodeWithLabel(next, node, sibling, pool_index, true);
        }
      } else {
        if constexpr (kLetterCheck) {
          _SetNewNode(next, c, sibling);
        } else {
          _SetNewNode(next, node, sibling);
        }
      }
    }
    _ConsumeBlock(Index2(base).block_index, label_datas.size());
  }

  void DeleteLeaf(_index_type node) {
    assert(da_.unit_at(node).is_leaf());
    auto cur_node = node;
    while (cur_node != kRootIndex and da_.unit_at(cur_node).is_leaf()) {
      if constexpr (kLetterCheck) {
        auto c = da_.unit_at(cur_node).check();
        auto base = cur_node xor c;
        _PopSibling(cur_node, 0, cur_node xor c, da_.unit_at(base).child());
      } else {
        auto parent = da_.unit_at(cur_node).check();
        _PopSibling(cur_node, parent, da_.base_at(parent), da_.unit_at(parent).child());
      }
      _EraseUnitAt(cur_node);
      _RefillBlock(Index2(cur_node).block_index, 1);
      if constexpr (not kLetterCheck) {
        cur_node = da_.unit_at(cur_node).check();
      } else {
        break;
      }
    }
    if constexpr (not kLetterCheck) {
      Reduce();
    }
  }

  void Reduce() {
    assert(not kLetterCheck);
    using bit_util::ctz256;
    for (_index_type b = da_.block_size()-1; b > 0; b--) {
      auto block = da_.block_at(b);
      for (size_t ctz = ctz256(block.field_ptr());
           ctz < 256; ctz = ctz256(block.field_ptr())) {
        auto parent = da_.unit_at(_da_trie::kNumUnitsPerBlock * b + ctz).check();
        auto base = da_.base_at(parent);
        _Shelter shelter;
        shelter.node = parent;
        da_.for_each_children(parent, [&](auto, _char_type child) {
          shelter.children.push_back(child);
        });
        auto compression_target = base_finder_.get_compression_target(shelter.children);
        if (Index2(compression_target).block_index >= b)
          return;
        da_.for_each_children(parent, [&](_index_type next, auto) {
          auto unit = da_.unit_at(next);
          shelter.luggages.emplace_back(unit.is_leaf(), unit.has_label(),
                                        unit.label_is_suffix(), unit.base(), unit.child());
          _EraseUnitAt(next);
        });
        _RefillBlock(Index2(base).block_index, shelter.children.size());
        _CompressNodes(shelter, compression_target);
      }
      da_.PopBlockFrom(b, kLinkLayerIdGeneral);
      da_.Shrink();
    }
  }

  template <class CoImpl>
  void ArrangeDa(const CoImpl &da, const _index_type node, const _index_type co_node) {
    std::vector<_InternalLabelContainer> label_datas;
    std::vector<_char_type> children;
    da.for_each_children(node, [&](_index_type index, _char_type child) {
      children.push_back(child);
      std::string label;
      label.push_back(child);
      auto unit = da.unit_at(index);
      if (unit.has_label())
        label += da._suffix_in_pool(unit.pool_index());
      while (da._is_single_node(index)) {
        _char_type child;
        if constexpr (kLetterCheck) {
          child = da.unit_at(da.base_at(index)).child();
        } else {
          child = da.unit_at(index).child();
        }
        label += child;
        index = da.base_at(index) xor child;
        unit = da.unit_at(index);
        if (unit.has_label())
          label += da._suffix_in_pool(unit.pool_index());
      }
      label_datas.push_back({label, unit.label_is_suffix()});
    });

    auto new_base = base_finder_.get(children);
    InsertNodes(co_node, label_datas, new_base);
    for (auto label_data : label_datas) {
      auto target = node;
      auto c = label_data.label.front();
      da._transition_bc(target, c);
      if (not label_data.suffix)
        ArrangeDa(da, target, new_base xor c);
    }
  }

  template <typename StrIter,
      typename Traits = std::iterator_traits<StrIter>>
  void ArrangeKeysets(StrIter begin, StrIter end, size_t depth, _index_type co_node) {
    if (begin >= end)
      return;

    std::vector<_InternalLabelContainer> label_datas;
    std::vector<_char_type> children;
    while (begin < end and ((*begin).size() == depth)) {
      label_datas.push_back({"", true});
      children.push_back(kLeafChar);
      ++begin;
    }
    std::vector<StrIter> iters = {begin};
    std::string_view front_label((*begin).data() + depth);
    _char_type prev_key = (*begin)[depth];
    auto append = [&](auto it) {
      assert(not front_label.empty());
      if (it == iters.back()) {
        label_datas.push_back({std::string(front_label), true});
      } else {
        label_datas.push_back({std::string(front_label.substr(0,1)), false});
      }
      children.push_back(prev_key);
      iters.push_back(it+1);
    };
    for (auto it = begin+1; it != end; ++it) {
      _char_type c = (*it)[depth];
      if (c != prev_key) {
        append(it-1);
        front_label = std::string_view((*it).data() + depth);
        prev_key = c;
      }
    }
    append(end-1);

    auto new_base = base_finder_.get(children);
    InsertNodes(co_node, label_datas, new_base);
    for (size_t i = 0; i < label_datas.size(); i++) {
      auto label = label_datas[i].label;
      if (not label_datas[i].suffix)
        ArrangeKeysets(iters[i], iters[i + 1], depth + label.size(), new_base xor (_char_type) label.front());
    }
  }

 protected:
  void _PrepareToUseUnitAt(size_t index) {
    da_.ExpandIfNeeded(index);
    da_.PopEmptyUnit(index);
    da_.unit_at(index).init_unit();
  }

  void _EraseUnitAt(size_t index) {
    da_.PushEmptyUnit(index);
  }

  void _UseBaseAt(_index_type base, _char_type child) {
    assert(LetterCheck);
    da_.ExpandIfNeeded(base);
    Index2 id(base);
    da_.block_at(id.block_index).freeze_base_at(id.unit_insets);
    da_.unit_at(base).set_child(child);
  }

  void _DontUseBaseAt(_index_type base) {
    assert(LetterCheck);
    Index2 id(base);
    da_.block_at(id.block_index).thaw_base_at(id.unit_insets);
    da_.unit_at(base).set_child(kEmptyChar);
  }

  void _SetNewEdge(_index_type node, _index_type new_base, _char_type new_child) {
    if constexpr (kUseUniqueBase) {
      _UseBaseAt(new_base, new_child);
    } else {
      da_.unit_at(node).set_child(new_child);
    }
    da_.set_base_at(node, new_base);
  }

  void _SetNewNode(_index_type index, check_type new_check, _char_type new_sibling = kEmptyChar) {
    _PrepareToUseUnitAt(index);
    da_.unit_at(index).init_unit(new_check, new_sibling);
  }

  void _SetNewNodeWithLabel(_index_type index,
                            check_type  new_check,
                            _char_type  new_sibling,
                            _index_type new_pool_index,
                            bool        label_is_suffix) {
    _SetNewNode(index, new_check, new_sibling);
    da_.unit_at(index).set_pool_index(new_pool_index, label_is_suffix);
  }

  _index_type _InsertTrans(_index_type node, _char_type c) {
    if (da_.unit_at(node).is_leaf()) {
      return Grow(node, base_finder_.get({c}), c);
    }
    auto base = da_.base_at(node);
    auto next = base xor c;
    if (not da_.unit_empty_at(next)) {
      node = _ResolveCollision(node, c);
      base = da_.base_at(node);
      next = base xor c;
    }
    _PrepareToUseUnitAt(next);
    auto next_unit = da_.unit_at(next);
    if constexpr (kLetterCheck) {
      next_unit.set_check(c);
    } else {
      next_unit.set_check(node);
    }
    if constexpr (kUseUniqueBase) {
      _PushSibling(next, c, node, base, da_.unit_at(base).child());
    } else {
      _PushSibling(next, c, node, base, da_.unit_at(node).child());
    }
    _ConsumeBlock(Index2(base).block_index, 1);

    return next;
  }

  void _Evacuate(_index_type node, _Shelter &shelter) {
    auto base = da_.base_at(node);
    da_.for_each_children(node, [&](_index_type index, _char_type child) {
      shelter.children.push_back(child);
      auto index_unit = da_.unit_at(index);
      shelter.luggages.emplace_back(index_unit.is_leaf(), index_unit.has_label(), index_unit.label_is_suffix(), index_unit.target(), index_unit.child());
      _EraseUnitAt(index);
    });
    if constexpr (kLetterCheck) {
      _DontUseBaseAt(base);
    }
    _RefillBlock(Index2(base).block_index, shelter.children.size());
  }

  void _UpdateNode(_index_type index, check_type new_check, _index_type sibling, const _MovingLuggage &luggage) {
    if (not luggage.has_label) {
      _SetNewNode(index, new_check, sibling);
      if (not luggage.is_leaf) {
        da_.unit_at(index).set_base(luggage.target);
      }
    } else {
      _SetNewNodeWithLabel(index, new_check, sibling, luggage.target, luggage.label_is_suffix);
    }
    if constexpr (not kLetterCheck) {
      if (not luggage.is_leaf) {
        da_.unit_at(index).set_child(luggage.child);
      }
    }
  }

  _index_type _MoveNodes(_index_type node, _Shelter &shelter, _index_type new_base,
                         _index_type monitoring_node = kRootIndex) {
    auto old_base = da_.base_at(node);
    _SetNewEdge(node, new_base, shelter.children.front());
    for (size_t i = 0; i < shelter.children.size(); i++) {
      auto child = shelter.children[i];
      auto sibling = i+1<shelter.children.size() ? shelter.children[i+1] : kEmptyChar;
      auto new_next = new_base xor child;
      auto& luggage = shelter.luggages[i];
      if constexpr (kLetterCheck) {
        _UpdateNode(new_next, child, sibling, luggage);
      } else {
        _UpdateNode(new_next, node, sibling, luggage);
        if (not luggage.is_leaf)
          _UpdateCheck(da_.base_at(new_next), luggage.child, new_next);
      }
      if ((old_base xor child) == monitoring_node) {
        monitoring_node = new_next;
      }
    }
    _ConsumeBlock(Index2(new_base).block_index, shelter.children.size());

    return monitoring_node;
  }

  // Compare number of children of index (x,y).
  // This function returns true if "num of x" greater than "num of y", otherwise return false.
  bool _CompareNumOfChildrenGreater(_index_type x, _index_type y) const {
    assert(not kLetterCheck);
    auto x_base = da_.base_at(x), y_base = da_.base_at(y);
    for (auto x_c = da_.unit_at(x).child(), y_c = da_.unit_at(y).child(); ;) {
      if ((x_c = da_.unit_at(x_base xor x_c).sibling()) == kEmptyChar)
        return false;
      if ((y_c = da_.unit_at(y_base xor y_c).sibling()) == kEmptyChar)
        return true;
    }

  }

  _index_type _ResolveCollision(_index_type node, _char_type c) {
    if constexpr (kLetterCheck) {

      _Shelter shelter;
      _Evacuate(node, shelter);
      shelter.children.push_back(c);
      auto new_base = base_finder_.get(shelter.children);
      shelter.children.pop_back();
      _MoveNodes(node, shelter, new_base);
      return node;

    } else {

      auto base = da_.base_at(node);
      auto conflicting_index = base xor c;
      auto competitor = da_.unit_at(conflicting_index).check();
      if (conflicting_index != kRootIndex and
          not _CompareNumOfChildrenGreater(competitor, node)) {
        // Move competitor node
        _Shelter shelter;
        _Evacuate(competitor, shelter);
        da_.PopEmptyUnit(conflicting_index);
        Index2 conflicted_id(conflicting_index);
        _ConsumeBlock(conflicted_id.block_index, 1);
        auto new_base = base_finder_.get(shelter.children);
        da_.PushEmptyUnit(conflicting_index);
        _RefillBlock(conflicted_id.block_index, 1);
        node = _MoveNodes(competitor, shelter, new_base, node);
      } else {
        // Move self node
        _Shelter shelter;
        _Evacuate(node, shelter);
        shelter.children.push_back(c);
        auto new_base = base_finder_.get(shelter.children);
        shelter.children.pop_back();
        _MoveNodes(node, shelter, new_base);
      }
      return node;

    }
  }

  void _PushSibling(_index_type index, _char_type c, _index_type parent, _index_type base, _char_type child) {
    assert(base == (index xor c));
    auto base_unit = da_.unit_at(base);
    assert(child != c);
    if (not Ordered or c < child) {
      if constexpr (kLetterCheck) {
        base_unit.set_child(c);
      } else {
        da_.unit_at(parent).set_child(c);
      }
      da_.unit_at(index).set_sibling(child);
    } else {
      auto index = base xor child;
      auto index_unit = da_.unit_at(index);
      _char_type sibling;
      while ((sibling = index_unit.sibling()) < c) {
        index_unit = da_.unit_at(base xor sibling);
      }
      index_unit.set_sibling(c);
      da_.unit_at(index).set_sibling(sibling);
    }
  }

  void _PopSibling(_index_type index, _index_type parent, _index_type base, _char_type child) {
    auto node_unit = da_.unit_at(index);
    auto succ_sibling = node_unit.sibling();
    auto base_unit = da_.unit_at(base);
    if ((base xor child) == index) {
      if constexpr (kLetterCheck) {
        base_unit.set_child(succ_sibling);
      } else {
        da_.unit_at(parent).set_child(succ_sibling);
      }
    } else {
      auto index_unit = node_unit;
      _index_type next;
      while ((next = base xor index_unit.sibling()) != index) {
        index_unit = da_.unit_at(next);
      }
      index_unit.set_sibling(succ_sibling);
    }
  }

  bool _is_general_error_counts(size_t errors) const {return errors < MaxTrial;}
  
  // Called only by base_finder_
  void _ErrorBlock(_index_type block) {
    auto b = da_.block_at(block);
    if (not _is_general_error_counts(b.error_count() + 1)) {
      da_.PopBlockFrom(block, kLinkLayerIdGeneral);
      if constexpr (kUsePersonalLink) {
        da_.PushBlockTo(block, kLinkLayerIdPersonal);
      }
    }
    b.errored();
  }

  void _ConsumeBlock(_index_type block, size_t num) {
    auto b = da_.block_at(block);
    b.consume(num);
    if (b.filled()) {
      if constexpr (kUsePersonalLink) {
        auto link_id = _is_general_error_counts(b.error_count()) ? kLinkLayerIdGeneral : kLinkLayerIdPersonal;
        da_.PopBlockFrom(block, link_id);
      } else {
        if (_is_general_error_counts(b.error_count())) {
          da_.PopBlockFrom(block, kLinkLayerIdGeneral);
        }
      }
    }
  }

  void _RefillBlock(_index_type block, size_t num) {
    if (num == 0)
      return;
    auto b = da_.block_at(block);
    if (b.filled()) {
      da_.PushBlockTo(block, kLinkLayerIdGeneral);
    } else if (not _is_general_error_counts(b.error_count())) {
      if constexpr (kUsePersonalLink) {
        da_.PopBlockFrom(block, kLinkLayerIdPersonal);
      }
      da_.PushBlockTo(block, kLinkLayerIdGeneral);
    }
    b.error_reset();
    b.refill(num);
  }

  void _UpdateCheck(_index_type base, _char_type child, _index_type new_check) {
    assert(not kLetterCheck);
    for (_char_type c = child; c != kEmptyChar;) {
      auto unit = da_.unit_at(base xor c);
      unit.set_check(new_check);
      c = unit.sibling();
    }
  }

  void _CompressNodes(_Shelter &shelter, _index_type target_base) {
    assert(not kLetterCheck);
    da_.set_base_at(shelter.node, target_base);
    std::vector<_Shelter> shelters;
    for (size_t i = 0; i < shelter.children.size(); i++) {
      auto child = shelter.children[i];
      auto sibling = i + 1 < shelter.children.size() ? shelter.children[i + 1] : kEmptyChar;
      auto new_next = target_base xor child;
      if (not da_.empty_unit_at(new_next)) {
        // Evacuate already placed siblings membering element at conflicting index to shelter.
        shelters.emplace_back();
        auto conflicting_node = da_.unit_at(new_next).check();
        _evacuate(conflicting_node, shelters.back());
      }
      auto &luggage = shelter.luggages[i];
      _UpdateNode(new_next, shelter.node, sibling, luggage);
      if (not da_.unit_at(new_next).is_leaf())
        _UpdateCheck(da_.base_at(new_next), luggage.child, new_next);
    }
    _ConsumeBlock(Index2(target_base).block_index, shelter.children.size());

    // Compress sheltered siblings recursively.
    for (auto &sht : shelters) {
      _CompressNodes(sht, base_finder_.get_compression_target(sht.children));
    }

    return;
  }

};


template <typename ValueType, typename IndexType, bool LetterCheck, bool Ordered, size_t MaxTrial, bool LegacyBuild>
class DoubleArrayMpTrie : public _DoubleArrayMpTrieBehavior<ValueType, IndexType, LetterCheck> {
  using _behavior = _DoubleArrayMpTrieBehavior<ValueType, IndexType, LetterCheck>;
  using _constructor = _DoubleArrayMpTrieConstructor<ValueType, IndexType, LetterCheck, Ordered, MaxTrial, not LegacyBuild>;

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

  DoubleArrayMpTrie() : _behavior(), constructor_(*this) {
    constructor_.init();
  }

  explicit DoubleArrayMpTrie(std::ifstream& ifs) : _behavior(ifs), constructor_(*this) {}

  template <typename StrIter,
      typename Traits = std::iterator_traits<StrIter>>
  explicit DoubleArrayMpTrie(StrIter begin, StrIter end) : DoubleArrayMpTrie() {
    constructor_.ArrangeKeysets(begin, end, 0, kRootIndex);
  }

  explicit DoubleArrayMpTrie(const std::vector<std::string>& key_set) : DoubleArrayMpTrie(key_set.begin(), key_set.end()) {}

  ~DoubleArrayMpTrie() = default;

  template <bool _Ordered, size_t _MaxTrial, bool _LegacyBuild>
  void build_from(const DoubleArrayMpTrie<ValueType, IndexType, LetterCheck, _Ordered, _MaxTrial, _LegacyBuild>& da) {
    constructor_.ArrangeDa(da, kRootIndex, kRootIndex);
  }

  void Rebuild() {
    DoubleArrayMpTrie<ValueType, IndexType, LetterCheck, Ordered, 32, LegacyBuild> new_da;
    new_da.build_from(*this);
    (_behavior)*this = std::move((_behavior)new_da);
  }

  void shrink_to_fit() {
    Rebuild();
  }

  value_pointer find(std::string_view key) {
    return _behavior::Traverse(key, [](auto) {},
                               [](auto, auto) { return nullptr; },
                               [](auto, auto, auto) { return nullptr; }).first;
  }

  const_value_pointer find(std::string_view key) const {
    return _behavior::Traverse(key, [](auto) {},
                               [](auto, auto) { return nullptr; },
                               [](auto, auto, auto) { return nullptr; }).first;
  }

  std::pair<value_type*, bool> insert(std::string_view key) {
    auto [ptr, res] = _behavior::Traverse(key, [](auto) {},
                                          [&](index_type node, size_t key_pos) {
                                            return constructor_.InsertInBc(node, key.substr(key_pos));
                                          }, [&](index_type node, size_t pool_pos, size_t key_pos) {
          return constructor_.InsertInTail(node, pool_pos, key.substr(key_pos));
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
    return _behavior::Traverse(key,
                               [&](index_type node) {
                                 constructor_.DeleteLeaf(node);
                               },
                               [](auto, auto) { return nullptr; }, [](auto, auto, auto) { return nullptr; }).second;
  }

};

}

#endif //DOUBLEARRAYMPTRIE_HPP_
