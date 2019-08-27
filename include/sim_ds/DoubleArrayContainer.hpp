//
// Created by 松本拓真 on 2019-08-09.
//

#ifndef DoubleArrayContainer_hpp
#define DoubleArrayContainer_hpp

#include "DoubleArrayUnit.hpp"
#include "DoubleArrayBlock.hpp"

#include "DoubleArrayException.h"

namespace sim_ds {


template <typename IndexType, bool LetterCheck>
class _DoubleArrayContainer {
 public:
  using _self = _DoubleArrayContainer;
  using _index_type = IndexType;
  using _char_type = uint8_t;
  using _inset_type = uint8_t;

  using check_type = std::conditional_t<LetterCheck, _char_type, _index_type>;
  using target_type = _index_type;

  static constexpr int kLinkLayers = LetterCheck ? 1 : 2;

  using _unit_container = _DoubleArrayUnitContainer<_index_type, _char_type, _inset_type, LetterCheck>;
  using _unit_reference = typename _unit_container::_unit_reference;
  using _const_unit_reference = typename _unit_container::_const_unit_reference;
  static constexpr size_t kUnitSize = _unit_reference::kUnitSize;
  static constexpr _index_type kEmptyFlag = _unit_reference::kEmptyFlag;
  static constexpr _index_type kIndexMax = _unit_reference::kIndexMax;

  static constexpr size_t kNumUnitsPerBlock = 0x100;
  static constexpr bool kUseUniqueBase = LetterCheck;
  using _block_container = _DoubleArrayBlockContainer<_index_type, _inset_type, kNumUnitsPerBlock, kUseUniqueBase>;
  using _block_reference = typename _block_container::_block_reference;
  using _const_block_reference = typename _block_container::_const_block_reference;

  static constexpr size_t kDisabledBlockLinkHead = std::numeric_limits<size_t>::max();
  static constexpr int kLinkLayerIdGeneral = 0;
  static constexpr int kLinkLayerIdPersonal = 1;

  struct Index2 {
    const size_t block_index;
    const size_t unit_insets;
    explicit Index2(size_t index) : block_index(index/kNumUnitsPerBlock),
                                    unit_insets(index%kNumUnitsPerBlock) {}
  };

 private:
  std::array<size_t, kLinkLayers> block_link_head_;
  _block_container block_;
  _unit_container unit_;

 public:
  _DoubleArrayContainer() {
    for (size_t i = 0; i < kLinkLayers; i++)
      block_link_head_[i] = kDisabledBlockLinkHead;
  }

  virtual size_t size_in_bytes() const {
    return sizeof(size_t)*kLinkLayers + block_.size_in_bytes() + unit_.size_in_bytes();
  }

  size_t num_nodes() const {
    size_t cnt = 0;
    for (size_t i = 0; i < block_size(); i++)
      cnt += 256 - block_at(i).num_empties();
    return cnt;
  }

  virtual float load_factor() const {return float(num_nodes()) / unit_size();}

  size_t block_link_head_of(int id) const {return block_link_head_[id];}

  void set_block_link_head_of(int id, size_t new_value) {
    block_link_head_[id] = new_value;
  }

  _block_reference block_at(size_t index) {return block_[index];}

  _const_block_reference block_at(size_t index) const {return block_[index];}

  size_t block_size() const {return block_.size();}

  void PushBlockTo(size_t index, int link_layer_id) {
    auto link_head = block_link_head_of(link_layer_id);
    if (link_head == kDisabledBlockLinkHead) {
      auto block = block_[index];
      block.set_pred(index);
      block.set_succ(index);
      set_block_link_head_of(link_layer_id, index);
    } else {
      auto head = block_[link_head];
      auto pred_index = head.pred();
      head.set_pred(index);
      block_[pred_index].set_succ(index);
      auto block = block_[index];
      block.set_pred(pred_index);
      block.set_succ(link_head);
    }
  }

  void PopBlockFrom(size_t index, const int link_layer_id) {
    auto link_head = block_link_head_of(link_layer_id);
    assert(link_head != kDisabledBlockLinkHead);
    auto block = block_[index];
    auto succ_index = block.succ();
    if (succ_index == index) {
      assert(link_head == index);
      set_block_link_head_of(link_layer_id, kDisabledBlockLinkHead);
    } else {
      auto pred_index = block.pred();
      block_[succ_index].set_pred(pred_index);
      block_[pred_index].set_succ(succ_index);
      if (index == link_head)
        set_block_link_head_of(link_layer_id, succ_index);
    }
  }

  _unit_reference unit_at(size_t index) {return unit_[index];}

  _const_unit_reference unit_at(size_t index) const {return unit_[index];}

  size_t unit_size() const {return unit_.size();}

  void PushEmptyUnit(size_t index) {
    Index2 id(index);
    auto block = block_[id.block_index];
    block.thaw_element_at(id.unit_insets);
    if (not block.link_enabled()) {
      unit_[index].init_disabled_unit(id.unit_insets, id.unit_insets);
      block.set_empty_head(id.unit_insets);
    } else {
      auto offset = id.block_index * kNumUnitsPerBlock;
      auto head_insets = block.empty_head();
      auto pred_insets = unit_[offset+head_insets].pred();
      unit_[offset+head_insets].set_pred(id.unit_insets);
      unit_[offset+pred_insets].set_succ(id.unit_insets);
      unit_[index].init_disabled_unit(pred_insets, head_insets);
    }
  }

  void PopEmptyUnit(size_t index) {
    Index2 id(index);
    auto block = block_[id.block_index];
    block.freeze_element_at(id.unit_insets);
    assert(block.link_enabled());
    auto unit = unit_[index];
    auto succ_insets = unit.succ();
    if (succ_insets == id.unit_insets) {
      assert(block.empty_head() == id.unit_insets);
      block.disable_link();
    } else {
      auto offset = id.block_index * kNumUnitsPerBlock;
      auto pred_insets = unit.pred();
      unit_[offset+pred_insets].set_succ(succ_insets);
      unit_[offset+succ_insets].set_pred(pred_insets);
      if (block.empty_head() == id.unit_insets)
        block.set_empty_head(succ_insets);
    }
  }

  void Expand() {
    if (unit_.size() + kNumUnitsPerBlock - 1 > kIndexMax) {
      throw _DoubleArrayExceptionSizeOver<_self>();
    }

    size_t block_index = block_.size();
    block_.resize(block_index+1);
    block_[block_index].init();
    PushBlockTo(block_index, kLinkLayerIdGeneral);

    auto offset = unit_.size();
    unit_.resize(unit_.size() + kNumUnitsPerBlock);
    unit_[offset].clean_with_link(kNumUnitsPerBlock-1, 1);
    for (size_t i = 0; i < kNumUnitsPerBlock-1; i++) {
      unit_[offset+i].clean_with_link(i-1, i+1);
    }
    unit_[offset+kNumUnitsPerBlock-1].clean_with_link(kNumUnitsPerBlock-2, 0);
  }

  void ExpandIfNeeded(size_t index) {
    if (index >= unit_size())
      Expand();
  }

  void Shrink() {
    if (block_at(block_size()-1).num_empties() != 0)
      return;
    block_.resize(block_size()-1);
    unit_.resize(unit_size()-kNumUnitsPerBlock);
  }

};

}

#endif //DoubleArrayContainer_hpp
