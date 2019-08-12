//
// Created by 松本拓真 on 2019-08-10.
//

#ifndef DOUBLEARRAYBASEFINDER_H_
#define DOUBLEARRAYBASEFINDER_H_

#include "da_util.hpp"

namespace sim_ds {


template <class _DaTrie, class _DaConstructor, bool BitOperationalBuild>
class DoubleArrayBaseFinder {
 public:
  using _self = DoubleArrayBaseFinder<_DaTrie, _DaConstructor, BitOperationalBuild>;
  using _da_impl = _DaTrie;
  using _da_constructor = _DaConstructor;
  using _char_type = typename _da_constructor::_char_type;
  static constexpr bool kUniqueBase = _da_constructor::kUseUniqueBase;

  static constexpr int kGeneralLinkId = 0;
  static constexpr int kPersonalLinkId = 1;

  static constexpr auto kDisabledBlockLinkHead = _da_impl::kDisabledBlockLinkHead;
  static constexpr size_t kNumUnitsPerBlock = _da_impl::kNumUnitsPerBlock;

 private:
  _da_impl& impl_;
  _da_constructor& constructor_;

 public:
  explicit DoubleArrayBaseFinder(_da_impl& da_impl, _da_constructor& constructor) : impl_(da_impl), constructor_(constructor) {}

  size_t new_base(const std::vector<_char_type>& label_set) const {
    return impl_.unit_size() xor label_set.front();
  }

  size_t get(const std::vector<_char_type> &label_set) {
    assert(not label_set.empty());
    if constexpr (not kUniqueBase) {
      if (label_set.size() == 1 and impl_.block_link_head_of(kPersonalLinkId) != kDisabledBlockLinkHead) {
        auto block_index = impl_.block_link_head_of(kPersonalLinkId);
        auto block = impl_.block_at(block_index);
        assert(block.link_enabled());
        auto empty_index = block_index * kNumUnitsPerBlock + block.empty_head();
        return empty_index xor label_set.front();
      }
    }

    if (impl_.block_link_head_of(kGeneralLinkId) == kDisabledBlockLinkHead) {
      return new_base(label_set);
    }
    for (size_t block_index = impl_.block_link_head_of(kGeneralLinkId); ; ) {
      auto block = impl_.block_at(block_index);
      auto offset = block_index * kNumUnitsPerBlock;

      if constexpr (not BitOperationalBuild) {
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
              return block_index * kNumUnitsPerBlock + base;
            }
          }
          inset = impl_.unit_at(offset+inset).succ();
          if (inset == block.empty_head())
            break;
        }
      } else {
        auto ctz = da_util::xcheck_in_da_block(block.unit_field_ptr(), label_set, block.base_field_ptr());
        if (ctz < kNumUnitsPerBlock) {
          return block_index * kNumUnitsPerBlock + ctz;
        }
      }

      auto next_block_index = block.succ();
      bool finish = next_block_index == impl_.block_link_head_of(kGeneralLinkId);
      constructor_.ErrorBlock(block_index);
      if (finish)
        break;
      block_index = next_block_index;
    }

    return new_base(label_set);
  }

  bool num_of_children_less_than(size_t index, size_t x) const {
    assert(not impl_.empty_unit_at(index));
    auto parent = impl_.unit_at(index).check();
    auto base = impl_.base_at(parent);
    size_t cnt = 0;
    for (size_t target = base xor parent.child(); ; ) {
      if (cnt >= x)
        return false;
      cnt++;
      auto unit = impl_.unit_at(target);
      auto sibling = unit.sibling();
      if (sibling == _da_impl::kEmptyChar)
        break;
      target = base xor sibling;
    }
    return cnt < x;
  }

  size_t get_compression_target(std::vector<_char_type> &label_set) const {
    assert(not kUniqueBase);
    if (label_set.size() == 1 and impl_.block_link_head_of[kPersonalLinkId] != kDisabledBlockLinkHead) {
      size_t pbh = impl_.block_link_head_of[kPersonalLinkId];
      return (kNumUnitsPerBlock * pbh + impl_.block_at(pbh).empty_head()) xor label_set.front();
    }
    if (impl_.block_link_head_of[kGeneralLinkId] == kDisabledBlockLinkHead) {
      return impl_.unit_size();
    }
    for (size_t block_index = impl_.block_link_head_of(kGeneralLinkId); ; ) {
        auto block = impl_.block_at(block_index);
        if (block_index != impl_.block_size()-1) {
            auto offset = block_index * kNumUnitsPerBlock;
            
            for (auto inset = block.empty_head(); ; ) {
                auto base = inset xor label_set.front();
                size_t i = 1;
                for (; i < label_set.size(); i++) {
                    auto index = base xor label_set[i];
                    if (not block.empty_element_at(index))
                        break;
                    if (not num_of_children_less_than(index, label_set.size()))
                        break;
                }
                if (i == label_set.size()) {
                    return block_index * kNumUnitsPerBlock + base;
                }
                inset = impl_.unit_at(offset+inset).succ();
                if (inset == block.empty_head())
                    break;
            }
        }

      auto next_block_index = block.succ();
      if (next_block_index == impl_.block_link_head_of(kGeneralLinkId))
        break;
      block_index = next_block_index;
    }

    return impl_.unit_size();
  }

};

template <class _DaTrie, class _DaConstructor>
using DoubleArrayBaseFinderBasic = DoubleArrayBaseFinder<_DaTrie, _DaConstructor, false>;

template <class _DaTrie, class _DaConstructor>
using DoubleArrayBaseFinderBitOperational = DoubleArrayBaseFinder<_DaTrie, _DaConstructor, true>;

}

#endif //DOUBLEARRAYBASEFINDER_H_
