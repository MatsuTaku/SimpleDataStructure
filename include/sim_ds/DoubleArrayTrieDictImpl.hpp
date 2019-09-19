//
// Created by 松本拓真 on 2019-08-10.
//

#ifndef DOUBLEARRAYTRIEDICTIMPL_HPP
#define DOUBLEARRAYTRIEDICTIMPL_HPP

#include "DoubleArrayContainer.hpp"

namespace sim_ds {


template <typename ValueType, typename IndexType, bool LetterCheck>
class _DoubleArrayTrieDictImpl : public _DoubleArrayContainer<IndexType, LetterCheck> {
 public:
  using _base = _DoubleArrayContainer<IndexType, LetterCheck>;
  using _value_type = ValueType;
  using _value_pointer = _value_type*;
  using _const_value_pointer = const _value_type*;
  using _index_type = IndexType;
  using _char_type = uint8_t;
  using _storage_type = uint8_t;

  static constexpr bool kLetterCheck = LetterCheck;

  static constexpr size_t kValueSize = std::is_void_v<ValueType> ? 0 : sizeof(ValueType);
  static constexpr size_t kIndexSize = sizeof(_index_type);

  static constexpr _char_type kEmptyChar = 0xff;
  static constexpr _char_type kEndChar = '\0';

  using typename _base::Index2;

 private:
  std::vector<_storage_type> label_pool_;

 public:
  _DoubleArrayTrieDictImpl() : _base() {}

  // MARK: Basic information

  size_t size_in_bytes() const override {
    return _base::size_in_bytes() + size_vec(label_pool_);
  }

  size_t bc_size_in_bytes() const {
    return _base::kUnitSize * _base::unit_size();
  }

  size_t bc_blank_size_in_bytes() const {
    return bc_size_in_bytes() - _base::kUnitSize * _base::num_nodes();
  }

  size_t pool_size_in_bytes() const {return label_pool_.size();}

  size_t pool_blank_size_in_bytes() const {
    size_t blank_size = label_pool_.size();
    for (size_t i = 0; i < _base::unit_size(); i++) {
      auto unit = _base::unit_at(i);
      if (unit_empty_at(i) or not unit.has_label())
        continue;
      if (unit.label_is_suffix()) {
        blank_size -= label_in_pool(unit.pool_index()).size() + kValueSize;
        if (not unit.has_label())
            blank_size--;
      } else {
        blank_size -= kIndexSize + label_in_pool(unit.pool_index() + kIndexSize).size();
      }
    }
    return blank_size;
  }

  size_t succinct_size_in_bytes() const {
    return size_in_bytes() - bc_size_in_bytes() - pool_size();
  }

  // Load factor of double-array.
  float load_factor_alpha() const {
    return _base::load_factor();
  }

  // Load factor of label pool.
  float load_factor_beta() const {
    return float(pool_blank_size_in_bytes()) / label_pool_.size();
  }

  float load_factor() const override {
    return float(bc_size_in_bytes() + pool_size_in_bytes() - bc_blank_size_in_bytes() - pool_blank_size_in_bytes()) / float(bc_size_in_bytes() + pool_size_in_bytes());
  }

  // MARK: Double-array Trie dict behavior

  bool unit_empty_at(size_t index) const {
    Index2 id(index);
    return _base::block_at(id.block_index).empty_element_at(id.unit_insets);
  }

  _index_type base_at(_index_type index) const {
      auto unit = _base::unit_at(index);
      assert(not unit.is_leaf());
      return not unit.has_label() ? unit.base() : target_in_pool_at(unit.pool_index());
  }

  void set_base_at(_index_type index, _index_type new_value) {
      auto unit = _base::unit_at(index);
      if (not unit.has_label()) {
          unit.set_base(new_value);
      } else {
          set_target_in_pool_at(unit.pool_index(), new_value);
      }
  }

  size_t pool_size() const {return label_pool_.size();}

  _char_type* pool_ptr_at(size_t index) {return label_pool_.data() + index;}

  const _char_type* pool_ptr_at(size_t index) const {return label_pool_.data() + index;}

  _char_type char_in_pool_at(size_t index) const {return label_pool_[index];}

  void set_char_in_pool_at(size_t index, _char_type new_value) {
    label_pool_[index] = new_value;
  }

  std::string_view label_in_pool(size_t index) const {
    return std::string_view((const char*)label_pool_.data() + index);
  }

  size_t AppendLabelInPool(std::string_view label) {
    auto index = label_pool_.size();
    label_pool_.resize(label_pool_.size() + label.size() + 1);
    for (size_t i = 0; i < label.size(); i++)
      label_pool_[index+i] = label[i];
    label_pool_[index+label.size()] = kEndChar;
    return index;
  }

  _value_pointer value_ptr_in_pool_at(size_t index) {
    return reinterpret_cast<_value_pointer>(label_pool_.data() + index);
  }

  _const_value_pointer value_ptr_in_pool_at(size_t index) const {
    return reinterpret_cast<_const_value_pointer>(label_pool_.data() + index);
  }

  _value_pointer AppendEmptyValue() {
    auto index = label_pool_.size();
    label_pool_.resize(label_pool_.size() + kValueSize);
    return value_ptr_in_pool_at(index);
  }

  size_t target_in_pool_at(size_t index) const {
    return *reinterpret_cast<const _index_type*>(label_pool_.data() + index);
  }

  void set_target_in_pool_at(size_t index, _index_type new_value) {
    *reinterpret_cast<_index_type*>(label_pool_.data() + index) = new_value;
  }

  size_t AppendTarget(_index_type value) {
    auto index = label_pool_.size();
    label_pool_.resize(label_pool_.size() + kIndexSize);
    set_target_in_pool_at(index, value);
    return index;
  }

};

}

#endif //DOUBLEARRAYTRIEDICTIMPL_HPP
