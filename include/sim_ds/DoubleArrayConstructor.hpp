//
//  DoubleArrayConstructor.hpp
//  bench_string_map_dynamic
//
//  Created by 松本拓真 on 2019/08/07.
//

#ifndef DoubleArrayConstructor_hpp
#define DoubleArrayConstructor_hpp

#include "da_util.hpp"

namespace sim_ds {


template <bool Ordered, size_t MaxTrial, bool BitOperationalFind, class _Impl>
class _DynamicDoubleArrayPatriciaTrieConstructor;

template <bool Ordered, size_t MaxTrial, bool BitOperationalFind, class _Impl>
class _DynamicDoubleArrayMpTrieConstructor {
public:
    using _impl = _Impl;
    using _value_type = typename _impl::_value_type;
    using _index_type = typename _impl::_index_type;
    using _char_type = typename _impl::_char_type;
    using _inset_type = typename _impl::_inset_type;
    
    static constexpr size_t kValueBytes = _impl::kValueBytes;
    static constexpr size_t kIndexBytes = _impl::kIndexBytes;
    static constexpr size_t kBlockSize = _impl::kBlockSize;
    static constexpr _index_type kRootIndex = _impl::kRootIndex;
    static constexpr _index_type kInitialEmptyBlockHead = _impl::kInitialEmptyBlockHead;
    static constexpr _index_type kEmptyFlag = _impl::kEmptyFlag;
    static constexpr _char_type kLeafChar = _impl::kLeafChar;
    static constexpr _char_type kEmptyChar = _impl::kEmptyChar;
    
protected:
    _impl& da_impl_;
    
public:
    _DynamicDoubleArrayMpTrieConstructor(_impl& da_impl) : da_impl_(da_impl) {}
    
    virtual ~_DynamicDoubleArrayMpTrieConstructor() = default;
    
    void init() {
        da_impl_._set_new_node(kRootIndex, kEmptyChar);
        _consume_block(da_impl_._block_index_of(kRootIndex), 1);
    }
    
    _index_type _grow(_index_type node, _index_type base, _char_type c) {
        assert(da_impl_.unit_at(node).is_leaf());
        da_impl_._set_new_edge(node, base, c);
        auto next = base xor c;
        da_impl_._set_new_node(next, c);
        _consume_block(da_impl_._block_index_of(next), 1);
        return next;
    }
    
    virtual _value_type* _insert_in_bc(_index_type node, std::string_view additional_suffix) {
        if (additional_suffix.size() > 0) {
            node = _insert_trans(node, additional_suffix.front());
            auto pool_index = da_impl_._append_suffix_in_pool(additional_suffix.substr(1));
            da_impl_.unit_at(node).set_pool_index(pool_index, true);
            return reinterpret_cast<_value_type*>(&*(da_impl_.pool().end()-kValueBytes));
        } else {
            node = _insert_trans(node, kLeafChar);
            da_impl_.unit_at(node).set_pool_index(da_impl_.pool().size(), true);
            return da_impl_._append_value();
        }
    }
    
    _value_type* _insert_in_tail(_index_type node, _index_type pool_pos, std::string_view additional_suffix) {
        assert(da_impl_.unit_at(node).label_is_suffix());
        auto pool_index = da_impl_.unit_at(node).pool_index();
        while (pool_index < pool_pos) {
            _char_type c = da_impl_.pool()[pool_index++];
            node = _grow(node, _find_base({c}), c);
        }
        _char_type char_at_confliction = da_impl_.pool()[pool_pos];
        auto next = _grow(node,
                          _find_base({char_at_confliction, additional_suffix.empty() ? kLeafChar : (_char_type)additional_suffix.front()}),
                          char_at_confliction);
        
        auto unit = da_impl_.unit_at(next);
        unit.set_pool_index(pool_pos+1, true);
        return _insert_in_bc(node, additional_suffix);
    }
    
    struct _internal_label_container {
        std::string label;
        bool suffix:1;
    };
    
    virtual void _insert_nodes(_index_type node, std::vector<_internal_label_container>& label_datas, _index_type base) {
        da_impl_._set_new_edge(node, base,
                               label_datas.front().label.empty() ? kLeafChar : label_datas.front().label.front());
        for (size_t i = 0; i < label_datas.size(); i++) {
            auto label = label_datas[i].label;
            _char_type c = label.empty() ? kLeafChar : label.front();
            auto next = base xor c;
            auto sibling = i+1<label_datas.size() ? label_datas[i+1].label.front() : kEmptyChar;
            if (label.empty()) {
                da_impl_._set_new_node_by_label(next, c, sibling, da_impl_.pool().size(), true);
                da_impl_._append_value();
            } else if (label_datas[i].suffix) {
                auto pool_index = da_impl_._append_suffix_in_pool(label.substr(1));
                da_impl_._set_new_node_by_label(next, c, sibling, pool_index, true);
            } else {
                da_impl_._set_new_node(next, c, sibling);
            }
        }
        _consume_block(da_impl_._block_index_of(base), label_datas.size());
    }
    
    void _delete_leaf(_index_type node) {
        assert(da_impl_.unit_at(node).is_leaf());
        auto cur_node = node;
        while (cur_node != kRootIndex and da_impl_.unit_at(cur_node).is_leaf()) {
            auto parent = da_impl_.unit_at(cur_node).check();
            _pop_sibling(cur_node, parent);
            da_impl_._erase(cur_node);
            _refill_block(da_impl_._block_index_of(cur_node), 1);
            cur_node = parent;
        }
    }
    
    _index_type _find_base(const std::vector<_char_type>& children) {
        auto [index, res] = _find_base_around_blocks(children);
        if (res)
            return index;
        return _new_base(children.front());
    }
    
protected:
    struct _moving_luggage {
        bool is_leaf:1;
        bool has_label:1;
        bool label_is_suffix:1;
        _index_type target;
        _moving_luggage(bool is_leaf, bool has_label, bool label_is_suffix, _index_type base) : is_leaf(is_leaf), has_label(has_label), label_is_suffix(label_is_suffix), target(base) {}
    };
    
    struct _shelter {
        std::vector<_char_type> children;
        std::vector<_moving_luggage> luggages;
    };
    
    void _evacuate(_index_type node, _shelter& shelter) {
        auto base = da_impl_._base_at(node);
        da_impl_._for_each_children(node, [&](_index_type index, _char_type child) {
            shelter.children.push_back(child);
            auto index_unit = da_impl_.unit_at(index);
            shelter.luggages.emplace_back(index_unit.is_leaf(), index_unit.has_label(), index_unit.label_is_suffix(), index_unit.target());
            da_impl_._erase(index);
        });
        da_impl_._thaw_base_at(base);
        _refill_block(da_impl_._block_index_of(base), shelter.children.size());
    }
    
    void _update_node(_index_type index, _char_type check, _index_type sibling, const _moving_luggage& luggage) {
        if (not luggage.has_label) {
            da_impl_._set_new_node(index, check, sibling);
            if (not luggage.is_leaf)
                da_impl_.unit_at(index).set_base(luggage.target);
        } else {
            da_impl_._set_new_node_by_label(index, check, sibling, luggage.target, luggage.label_is_suffix);
        }
    }
    
    _index_type _move_nodes(_index_type node, _shelter& shelter, _index_type new_base,
                            _index_type monitoring_node = kRootIndex) {
        auto old_base = da_impl_._base_at(node);
        da_impl_._set_new_edge(node, new_base, shelter.children.front());
        for (size_t i = 0; i < shelter.children.size(); i++) {
            auto child = shelter.children[i];
            auto sibling = i+1<shelter.children.size() ? shelter.children[i+1] : kEmptyChar;
            auto new_next = new_base xor child;
            auto& luggage = shelter.luggages[i];
            _update_node(new_next, child, sibling, luggage);
            assert(not da_impl_._empty_at(new_next));
            if ((old_base xor child) == monitoring_node) {
                monitoring_node = new_next;
            }
        }
        _consume_block(da_impl_._block_index_of(new_base), shelter.children.size());
        
        return monitoring_node;
    }
    
    _index_type _resolve_collision(_index_type node, _char_type c) {
        _shelter shelter;
        _evacuate(node, shelter);
        shelter.children.push_back(c);
        auto new_base = _find_base(shelter.children);
        shelter.children.pop_back();
        _move_nodes(node, shelter, new_base);
        return node;
    }
    
    void _push_sibling(_index_type node, _char_type c, _index_type base) {
        assert(base == (node xor c));
        auto base_unit = da_impl_.unit_at(base);
        auto child = base_unit.child();
        assert(child != c);
        if (not Ordered or c < child) {
            base_unit.set_child(c);
            da_impl_.unit_at(node).set_sibling(child);
        } else {
            auto index = base xor child;
            auto index_unit = da_impl_.unit_at(index);
            _char_type sibling;
            while ((sibling = index_unit.sibling()) < c) {
                index_unit = da_impl_.unit_at(base xor sibling);
            }
            index_unit.set_sibling(c);
            da_impl_.unit_at(node).set_sibling(sibling);
        }
    }
    
    void _pop_sibling(_index_type node, _index_type base) {
        auto node_unit = da_impl_.unit_at(node);
        auto succ_sibling = node_unit.sibling();
        auto base_unit = da_impl_.unit_at(base);
        if ((base xor base_unit.child()) == node)
            base_unit.set_child(succ_sibling);
        auto index_unit = node_unit;
        _index_type next;
        while ((next = base xor index_unit.sibling()) != node) {
            index_unit = da_impl_.unit_at(next);
        }
        index_unit.set_sibling(succ_sibling);
    }
    
    bool _is_enabled_error_counts(size_t errors) const {return errors < MaxTrial;}
    
    void _error_block(_index_type block) {
        auto b = da_impl_.block_at(block);
        if (not _is_enabled_error_counts(b.error_count() + 1)) {
            da_impl_._pop_block(block);
        }
        b.errored();
    }
    
    void _consume_block(_index_type block, size_t num) {
        auto b = da_impl_.block_at(block);
        b.consume(num);
        if (b.filled() and _is_enabled_error_counts(b.error_count())) {
            da_impl_._pop_block(block);
        }
    }
    
    void _refill_block(_index_type block, size_t num) {
        if (num == 0)
            return;
        auto b = da_impl_.block_at(block);
        if (b.filled() or not _is_enabled_error_counts(b.error_count())) {
            da_impl_._push_block(block);
        }
        b.error_reset();
        b.refill(num);
    }
    
    _index_type _insert_trans(_index_type node, _char_type c) {
        if (da_impl_.unit_at(node).is_leaf()) {
            return _grow(node, _find_base({c}), c);
        }
        auto base = da_impl_._base_at(node);
        auto next = base xor c;
        if (not da_impl_._empty_at(next)) {
            node = _resolve_collision(node, c);
            base = da_impl_._base_at(node);
            next = base xor c;
        }
        da_impl_._setup(next);
        auto next_unit = da_impl_.unit_at(next);
        next_unit.set_check(c);
        _push_sibling(next, c, base);
        _consume_block(da_impl_._block_index_of(base), 1);
        
        return next;
    }
    
    _index_type _new_base(_char_type c) const {
        if constexpr (BitOperationalFind) {
            return da_impl_.bc_container_size();
        } else {
            return da_impl_.bc_container_size() xor c;
        }
    }
    
    std::pair<_index_type, bool> _find_base_around_blocks(const std::vector<_char_type>& children) {
        if (da_impl_.enabled_blocks_head() == kInitialEmptyBlockHead)
            return {0, false};
        std::vector<size_t> visited;
        for (_index_type block_i = da_impl_.enabled_blocks_head(); ; ) {
            auto [res, finish] = _find_base_in_block(block_i, children);
            if (finish)
                return {res, true};
            auto fin = res == da_impl_.enabled_blocks_head();
            _error_block(block_i);
            visited.push_back(block_i);
            block_i = res;
            if (fin)
                break;
        }
        return {0, false};
    }
    
    std::pair<_index_type, bool> _find_base_in_block(_index_type block_i, const std::vector<_char_type>& children) const {
        auto block = da_impl_.block_at(block_i);
        auto offset = kBlockSize * block_i;
        if constexpr (BitOperationalFind) {
            auto ctz = da_util::xcheck_in_da_block(block.base_field_ptr(), block.unit_field_ptr(), children);
            if (ctz < kBlockSize) {
                return {offset + ctz, true};
            }
        } else {
            auto begin = block.empty_head();
            for (_inset_type i = begin; ; ) {
                assert(block.empty_element_at(i));
                auto base = i xor children.front();
                if (block.empty_base_at(base)) {
                    size_t j = 1;
                    for (; j < children.size(); j++) {
                        if (not block.empty_element_at(base xor children[j]))
                            break;
                    }
                    if (j == children.size()) {
                        return {offset + base, true};
                    }
                }
                i = da_impl_.unit_at(offset + i).succ();
                if (i == begin)
                    break;
            }
        }
        return {block.succ(), false};
    }
    
};


template <bool Ordered, size_t MaxTrial, bool BitOperationalFind, class _Impl>
class _DynamicDoubleArrayPatriciaTrieConstructor : public _DynamicDoubleArrayMpTrieConstructor<Ordered, MaxTrial, BitOperationalFind, _Impl> {
public:
    using _impl = _Impl;
    using _base = _DynamicDoubleArrayMpTrieConstructor<Ordered, MaxTrial, BitOperationalFind, _Impl>;
    using typename _base::_value_type;
    using typename _base::_index_type;
    using typename _base::_char_type;
    
    using _base::kValueBytes;
    using _base::kIndexBytes;
    using _base::kLeafChar;
    using _base::kEmptyFlag;
    using _base::kEmptyChar;
    
    using _base::da_impl_;
    
public:
    _DynamicDoubleArrayPatriciaTrieConstructor(_impl& da_impl) : _base(da_impl) {}
    
    virtual ~_DynamicDoubleArrayPatriciaTrieConstructor() = default;
    
    _value_type* _insert_in_bc(_index_type node, std::string_view additional_suffix) override {
        if (not additional_suffix.empty()) {
            node = _base::_insert_trans(node, additional_suffix.front());
            auto pool_index = da_impl_._append_suffix_in_pool(additional_suffix.substr(1));
            da_impl_.unit_at(node).set_pool_index(pool_index, true);
            return reinterpret_cast<_value_type*>(&*(da_impl_.pool().end()-kValueBytes));
        } else {
            node = _base::_insert_trans(node, kLeafChar);
            da_impl_.unit_at(node).set_pool_index(da_impl_.pool().size(), true);
            return da_impl_._append_value();
        }
    }
    
    _value_type* _insert_in_suffix(_index_type node, _index_type label_pos, std::string_view additional_suffix) {
        assert(da_impl_.unit_at(node).has_label() and
               da_impl_.unit_at(node).label_is_suffix());
        auto forked_base = _base::_find_base({da_impl_.pool()[label_pos], (_char_type)additional_suffix.front()});
        _fork_in_suffix(node, label_pos, forked_base);
        assert(not da_impl_.unit_at(node).is_leaf());
        return _insert_in_bc(node, additional_suffix);
    }
    
    _value_type* _insert_in_internal_label(_index_type node, _index_type label_pos, std::string_view additional_suffix) {
        assert(da_impl_.unit_at(node).has_label() and
               not da_impl_.unit_at(node).label_is_suffix());
        auto forked_base = _base::_find_base({da_impl_.pool()[label_pos], (_char_type)additional_suffix.front()});
        _fork_in_internal_label(node, label_pos, forked_base);
        return _insert_in_bc(node, additional_suffix);
    }
    
    void _insert_nodes(_index_type node, std::vector<typename _base::_internal_label_container>& label_datas, _index_type base) override {
        da_impl_._set_new_edge(node, base, label_datas.front().label.empty() ? kLeafChar : label_datas.front().label.front());
        for (size_t i = 0; i < label_datas.size(); i++) {
            auto cur_label = label_datas[i].label;
            _char_type c = cur_label.empty() ? kLeafChar : cur_label.front();
            auto next = base xor c;
            assert(da_impl_._empty_at(next));
            auto sibling = i+1 < label_datas.size() ? label_datas[i+1].label.front() : kEmptyChar;
            if (label_datas[i].suffix) {
                if (cur_label.empty()) {
                    da_impl_._set_new_node_by_label(next, c, sibling, da_impl_.pool().size(), true);
                    da_impl_._append_value();
                } else {
                    auto pool_index = da_impl_._append_suffix_in_pool(cur_label.substr(1));
                    da_impl_._set_new_node_by_label(next, c, sibling, pool_index, true);
                }
            } else {
                if (cur_label.size() == 1) {
                    da_impl_._set_new_node(next, c, sibling);
                } else {
                    auto pool_index = da_impl_._append_internal_label_in_pool(cur_label.substr(1), kEmptyFlag);
                    da_impl_._set_new_node_by_label(next, c, sibling, pool_index, false);
                }
            }
        }
        _base::_consume_block(da_impl_._block_index_of(base), label_datas.size());
    }
    
private:
    void _fork_in_suffix(_index_type node, _index_type label_pos, _index_type forked_base) {
        assert(da_impl_.unit_at(node).has_label() and
               da_impl_.unit_at(node).label_is_suffix());
        _char_type char_at_confliction = da_impl_.pool()[label_pos];
        da_impl_._setup_base_at(forked_base, char_at_confliction);
        
        auto label_index = da_impl_.unit_at(node).pool_index();
        auto left_label_length = label_pos - label_index;
        std::vector<_char_type> cs;
        for (int i = 0; i < left_label_length; i++)
            cs.push_back(da_impl_.pool()[label_index+i]);
        auto node_unit = da_impl_.unit_at(node);
        if (left_label_length == 0) {
            node_unit.set_base(forked_base);
        } else {
            std::string left_label((char*)da_impl_.pool().data() + label_index, left_label_length);
            auto relay_pool_index = da_impl_._append_internal_label_in_pool(left_label, forked_base);
            node_unit.set_pool_index(relay_pool_index, false);
        }
        assert(not node_unit.is_leaf());
        auto relay_next = forked_base xor char_at_confliction;
        da_impl_._set_new_node_by_label(relay_next, char_at_confliction, kEmptyChar, label_pos+1, true);
        _base::_consume_block(da_impl_._block_index_of(relay_next), 1);
    }
    
    void _fork_in_internal_label(_index_type node, _index_type label_pos, _index_type forked_base) {
        assert(da_impl_.unit_at(node).has_label() and
               not da_impl_.unit_at(node).label_is_suffix());
        _char_type char_at_confliction = da_impl_.pool()[label_pos];
        assert(char_at_confliction != kLeafChar);
        da_impl_._setup_base_at(forked_base, char_at_confliction);
        const auto pool_index = da_impl_.unit_at(node).pool_index();
        const auto node_base = da_impl_._base_in_pool(pool_index);
        
        auto label_index = pool_index + kIndexBytes;
        auto left_label_length = label_pos - label_index;
        auto right_label_length = 0;
        bool small_prefix = false;
        while (not small_prefix and da_impl_.pool()[label_pos + right_label_length] != kLeafChar) {
            small_prefix = left_label_length < ++right_label_length;
        }
        auto relay_next = forked_base xor char_at_confliction;
        if (small_prefix) {
            // ||*|*|*||*|*|*|$||
            //  |→   ←| conflict between this area
            if (left_label_length == 0) {
                da_impl_.unit_at(node).set_base(forked_base);
            } else {
                std::string left_label((char*)da_impl_.pool().data() + label_index, left_label_length);
                auto relay_pool_index = da_impl_._append_internal_label_in_pool(left_label, forked_base);
                da_impl_.unit_at(node).set_pool_index(relay_pool_index, false);
            }
            if (da_impl_.pool()[label_pos + 1] == kLeafChar) { // Length of right-label is 1.
                da_impl_._set_new_node(relay_next, char_at_confliction);
                da_impl_.unit_at(relay_next).set_base(node_base);
            } else {
                da_impl_._set_new_node_by_label(relay_next, char_at_confliction, kEmptyChar, label_pos+1-kIndexBytes, false);
                da_impl_._set_base_in_pool(label_pos+1-kIndexBytes, node_base);
            }
        } else {
            // ||*|*|*||*|*|*|$||
            //         |→   ←| conflict between this area
            da_impl_.pool()[label_pos] = kLeafChar;
            da_impl_._set_base_in_pool(pool_index, forked_base);
            if (right_label_length == 1) {
                da_impl_._set_new_node(relay_next, char_at_confliction, kEmptyChar);
                da_impl_.unit_at(relay_next).set_base(node_base);
            } else {
                std::string right_label((char*)da_impl_.pool().data() + label_pos + 1);
                auto relay_pool_index = da_impl_._append_internal_label_in_pool(right_label, node_base);
                da_impl_._set_new_node_by_label(relay_next, char_at_confliction, kEmptyChar, relay_pool_index, false);
            }
        }
        _base::_consume_block(da_impl_._block_index_of(relay_next), 1);
    }
    
};

}

#endif /* DoubleArrayConstructor_hpp */
