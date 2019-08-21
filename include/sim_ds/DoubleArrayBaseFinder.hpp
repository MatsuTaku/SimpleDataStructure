//
// Created by 松本拓真 on 2019-08-10.
//

#ifndef DOUBLEARRAYBASEFINDER_H_
#define DOUBLEARRAYBASEFINDER_H_

#include "da_util.hpp"

namespace sim_ds {


template <class _Da, class _DaConstructor, bool BitOperationalBuild>
class _DoubleArrayBaseFinder {
public:
  using _self = _DoubleArrayBaseFinder<_Da, _DaConstructor, BitOperationalBuild>;
  using _da = _Da;
  using _da_constructor = _DaConstructor;
  using _char_type = typename _da_constructor::_char_type;
  
  static constexpr bool kUniqueBase = _da_constructor::kUseUniqueBase;
  static constexpr auto kDisabledBlockLinkHead = _da::kDisabledBlockLinkHead;
  static constexpr size_t kNumUnitsPerBlock = _da::kNumUnitsPerBlock;
  
  static constexpr int kGeneralLinkId = 0;
  static constexpr int kPersonalLinkId = 1;

 private:
  _da& da_;
  _da_constructor& constructor_;
  
  template <class Action, class Failure>
  size_t _get_for_blocks(Action action, Failure failure) const {
    for (size_t block_index = da_.block_link_head_of(kGeneralLinkId); ; ) {
      auto succ_block_index = da_.block_at(block_index).succ();
      bool is_end = succ_block_index == da_.block_link_head_of(kGeneralLinkId);
      
      auto [index, result] = action(block_index);
      if (result)
        return index;
      
      if (is_end)
        break;
      block_index = succ_block_index;
    }
    return failure();
  }
  
  bool _num_of_children_less_than(size_t index, size_t x) const {
    assert(not da_.empty_unit_at(index));
    auto parent = da_.unit_at(index).check();
    auto base = da_.base_at(parent);
    size_t cnt = 0;
    for (size_t target = base xor parent.child(); ; ) {
      if (cnt >= x)
        return false;
      cnt++;
      auto unit = da_.unit_at(target);
      auto sibling = unit.sibling();
      if (sibling == _da::kEmptyChar)
        break;
      target = base xor sibling;
    }
    return cnt < x;
  }

 public:
  explicit _DoubleArrayBaseFinder(_da& da_impl, _da_constructor& constructor) : da_(da_impl), constructor_(constructor) {}

  size_t new_base(const std::vector<_char_type>& label_set) const {
    return da_.unit_size() xor label_set.front();
  }

  size_t get(const std::vector<_char_type> &label_set) {
    assert(not label_set.empty());
    if constexpr (not kUniqueBase) {
      if (label_set.size() == 1 and da_.block_link_head_of(kPersonalLinkId) != kDisabledBlockLinkHead) {
        auto block_index = da_.block_link_head_of(kPersonalLinkId);
        auto block = da_.block_at(block_index);
        assert(block.link_enabled());
        auto empty_index = block_index * kNumUnitsPerBlock + block.empty_head();
        return empty_index xor label_set.front();
      }
    }

    if (da_.block_link_head_of(kGeneralLinkId) == kDisabledBlockLinkHead) {
      return new_base(label_set);
    }
    
    return _get_for_blocks([&](size_t block_index) -> std::pair<size_t, bool> {

      auto block = da_.block_at(block_index);
      auto offset = block_index * kNumUnitsPerBlock;
      if constexpr (not BitOperationalBuild) {
        // Basic algorithm which using empty-element link.
        for (auto inset = block.empty_head(); ; ) {
          auto base = inset xor label_set.front();
          bool is_candidate = not kUniqueBase;
          if constexpr (kUniqueBase) {
            is_candidate |= block.empty_base_at(base);
          }
          if (is_candidate) {
            size_t i = 1;
            for (; i < label_set.size(); i++) {
              if (not block.empty_element_at(base xor label_set[i]))
                break;
            }
            if (i == label_set.size()) {
              return {block_index * kNumUnitsPerBlock + base, true};
            }
          }
          inset = da_.unit_at(offset+inset).succ();
          if (inset == block.empty_head())
            break;
        }
        
      } else {
        // Proposal algorithm which using bit operations.
        auto ctz = da_util::xcheck_in_da_block(block.unit_field_ptr(), label_set, block.base_field_ptr());
        if (ctz < kNumUnitsPerBlock) {
          return {block_index * kNumUnitsPerBlock + ctz, true};
        }
        
      }
      
      constructor_._ErrorBlock(block_index);
      return {0, false};
    }, [&] {
      return new_base(label_set);
    });
  }

  size_t get_compression_target(std::vector<_char_type> &label_set) const {
    assert(not kUniqueBase);
    if (label_set.size() == 1 and da_.block_link_head_of[kPersonalLinkId] != kDisabledBlockLinkHead) {
      size_t pbh = da_.block_link_head_of[kPersonalLinkId];
      return (kNumUnitsPerBlock * pbh + da_.block_at(pbh).empty_head()) xor label_set.front();
    }
    if (da_.block_link_head_of[kGeneralLinkId] == kDisabledBlockLinkHead) {
      return da_.unit_size();
    }
    
    return _get_for_blocks([&](size_t block_index) -> std::pair<size_t, bool> {
      auto block = da_.block_at(block_index);
      if (block_index != da_.block_size()-1) {
        auto offset = block_index * kNumUnitsPerBlock;
        
        for (auto inset = block.empty_head(); ; ) {
          auto base = inset xor label_set.front();
          size_t i = 1;
          for (; i < label_set.size(); i++) {
            auto index = base xor label_set[i];
            if (not block.empty_element_at(index))
              break;
            if (not _num_of_children_less_than(index, label_set.size()))
              break;
          }
          if (i == label_set.size()) {
            return {offset + base, true};
          }
          inset = da_.unit_at(offset+inset).succ();
          if (inset == block.empty_head())
            break;
        }
      }
      return {0, false};
    }, [&] {
      return da_.unit_size();
    });
  }

};

template <class _DaTrie, class _DaConstructor>
using BasicDoubleArrayBaseFinder = _DoubleArrayBaseFinder<_DaTrie, _DaConstructor, false>;

template <class _DaTrie, class _DaConstructor>
using BitOperationalDoubleArrayBaseFinder = _DoubleArrayBaseFinder<_DaTrie, _DaConstructor, true>;

}

#endif //DOUBLEARRAYBASEFINDER_H_
