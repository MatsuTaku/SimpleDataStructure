//
//  DoubleArrayTrie.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/06/18.
//

#ifndef DoubleArray_hpp
#define DoubleArray_hpp

#include "basic.hpp"
#include "CompactDoubleArrayUnit.hpp"
#include "CompactDoubleArrayBlock.hpp"
#include "CompactDoubleArrayConstructor.hpp"
#include "bit_util256.hpp"
#include "da_util.hpp"

namespace sim_ds {


// MARK: - MPTrie

template <typename ValueType, typename IndexType>
class _CompactDoubleArrayMpTrieImpl {
public:
    using _self = _CompactDoubleArrayMpTrieImpl<ValueType, IndexType>;
    using _value_type = ValueType;
    using _index_type = IndexType;
    using _char_type = uint8_t;
    using _inset_type = uint8_t;
    
    static constexpr _index_type kRootIndex = 0;
    static constexpr _char_type kLeafChar = '\0';
    static constexpr _char_type kEmptyChar = 0xFF;
    
    using _unit_container = _CompactDoubleArrayUnitContainer<_self, true>;
    using _unit_reference = typename _unit_container::_unit_reference;
    using _const_unit_reference = typename _unit_container::_const_unit_reference;
    static constexpr auto kUnitBytes = _unit_container::kUnitBytes;
    static constexpr auto kIndexMax = _unit_reference::kIndexMax;
    static constexpr auto kIndexMask = _unit_reference::kIndexMask;
    static constexpr auto kIndexBytes = _unit_reference::kIndexBytes;
    static constexpr auto kEmptyFlag = _unit_reference::kEmptyFlag;
    
    static constexpr size_t kValueBytes = std::is_void_v<ValueType> ? 0 : sizeof(ValueType);
    
    static constexpr unsigned kBlockSize = 0x100;
    using _block_container = _CompactDoubleArrayBlockContainer<_self>;
    using _block_reference = typename _block_container::_block_reference;
    using _const_block_reference = typename _block_container::_const_block_reference;
    static constexpr auto kBlockQBytes = _block_container::kBlockQBytes;
    static constexpr _index_type kInitialEmptyBlockHead = std::numeric_limits<_index_type>::max();
    
protected:
    _index_type enabled_blocks_head_;
    _block_container basic_block_;
    _unit_container container_;
    std::vector<_char_type> pool_;
    
    _CompactDoubleArrayMpTrieImpl() : enabled_blocks_head_(kInitialEmptyBlockHead) {}
    
    explicit _CompactDoubleArrayMpTrieImpl(std::ifstream& ifs) {
        enabled_blocks_head_ = read_val<_index_type>(ifs);
        basic_block_.read(ifs);
        container_.read(ifs);
        read_vec(ifs, pool_);
    }
    
    size_t _size_in_bytes() const {
        return (sizeof(enabled_blocks_head_) +
                basic_block_.size_in_bytes() +
                container_.size_in_bytes() +
                size_vec(pool_));
    }
    
    void _serialize(std::ofstream& ofs) const {
        write_val(enabled_blocks_head_, ofs);
        basic_block_.write(ofs);
        container_.write(ofs);
        write_vec(pool_, ofs);
    }
    
public:
    _CompactDoubleArrayMpTrieImpl(_CompactDoubleArrayMpTrieImpl&&) = default;
    _CompactDoubleArrayMpTrieImpl& operator=(_CompactDoubleArrayMpTrieImpl&&) = default;
    
    virtual ~_CompactDoubleArrayMpTrieImpl() = default;
    
    _index_type enabled_blocks_head() const {return enabled_blocks_head_;}
    
    _unit_reference unit_at(_index_type index) {return container_[index];}
    _const_unit_reference unit_at(_index_type index) const {return container_[index];}
    
    size_t bc_container_size() const {return container_.size();}
    
    _index_type _block_index_of(_index_type index) const {return index/kBlockSize;}
    
    _block_reference block_at(_index_type index) {return basic_block_[index];}
    _const_block_reference block_at(_index_type index) const {return basic_block_[index];}
    
    size_t block_container_size() const {return basic_block_.size();}
    
    std::vector<_char_type>& pool() {return pool_;}
    const std::vector<_char_type>& pool_at(_index_type index) const {return pool_;}
    
    void _expand() {
        if (container_.size() > kIndexMax) {
            throw "Index out-of-range! You should set large byte-size of template parameter.";
        }
        
        // Append blocks linking
        basic_block_.resize(basic_block_.size() + 1);
        basic_block_[basic_block_.size() - 1].init();
        _push_block(basic_block_.size() - 1);
        
        // Link elements in appended block
        container_.resize(container_.size() + kBlockSize);
        auto front = container_.size() - kBlockSize;
        auto inset_back = kBlockSize - 1;
        container_[front].clean_with_link(inset_back, 1);
        for (size_t i = 1; i < inset_back; i++) {
            container_[front+i].clean_with_link(i-1, i+1);
        }
        container_[front+inset_back].clean_with_link(inset_back-1, 0);
    }
    
    void _shrink() {
        if (basic_block_.size() == 1)
            return;
        assert(basic_block_[container_.size()-1].num_empties() == kBlockSize);
        if (_is_enabled_error_counts(basic_block_.back().error_count()))
            _pop_block(basic_block_.size()-1);
        container_.resize(container_.size() - kBlockSize);
        basic_block_.resize(basic_block_.size() - 1);
    }
    
    bool _empty_at(size_t index) const {return basic_block_[_block_index_of(index)].empty_element_at(index%kBlockSize);}
    
    void _expand_if_needed(size_t index) {
        if (index >= container_.size())
            _expand();
    }
    
    void _setup(size_t index) {
        _expand_if_needed(index);
        _freeze(index);
        container_[index].init_unit();
    }
    
    void _erase(size_t index) {
        _thaw(index);
    }
    
    virtual _index_type _base_at(_index_type node) const {
        assert(not container_[node].is_leaf());
        return container_[node].base();
    }
    
    virtual void _set_base_at(_index_type node, _index_type new_base) {
        container_[node].set_base(new_base);
    }
    
    void _setup_base_at(_index_type base, _char_type child) {
        _expand_if_needed(base);
        basic_block_[_block_index_of(base)].freeze_base_at(base%kBlockSize);
        container_[base].set_child(child);
    }
    
    void _thaw_base_at(_index_type base) {
        basic_block_[_block_index_of(base)].thaw_base_at(base%kBlockSize);
        container_[base].set_child(kEmptyChar);
    }
    
    void _set_new_edge(_index_type node, _index_type new_base, _char_type new_child) {
        _setup_base_at(new_base, new_child);
        _set_base_at(node, new_base);
    }
    
    void _set_new_node(_index_type index, _char_type new_check, _char_type new_sibling = kEmptyChar) {
        _setup(index);
        container_[index].init_unit(new_check, new_sibling);
    }
    
    void _set_new_node_by_label(_index_type index, _char_type new_check, _char_type new_sibling, _index_type new_pool_index, bool label_is_suffix) {
        _set_new_node(index, new_check, new_sibling);
        container_[index].set_pool_index(new_pool_index, label_is_suffix);
    }
    
    std::string_view _suffix_in_pool(_index_type pool_index) const {
        return std::string_view((char*)pool_.data() + pool_index);
    }
    
    template <class Action>
    void _for_each_children(_index_type node, Action action) const {
        assert(not container_[node].is_leaf());
        auto base = _base_at(node);
        auto child = container_[base].child();
        while (child != kEmptyChar) {
            auto next = base xor child;
            assert(not _empty_at(next));
            assert(container_[next].check() == child);
            auto sibling = container_[next].sibling();
            action(next, child);
            child = sibling;
        }
    }
    
    size_t _num_of_children(_index_type node) const {
        if (container_[node].is_leaf())
            return 0;
        size_t cnt = 0;
        _for_each_children(node, [&cnt](auto, auto) {++cnt;});
        return cnt;
    }
    
    bool _is_single_node(_index_type node) const {
        if (container_[node].is_leaf())
            return false;
        auto base = _base_at(node);
        auto child = container_[base].child();
        return container_[base xor child].sibling() == kEmptyChar;
    }
    
    _index_type _append_suffix_in_pool(std::string_view suffix) {
        _index_type index = pool_.size();
        pool_.resize(index + suffix.size() + 1 + kValueBytes);
        auto ptr = pool_.data()+index;
        for (_char_type c : suffix)
            *(ptr++) = c;
        *ptr = kLeafChar;
        return index;
    }
    
    _value_type* _append_value() {
        pool_.resize(pool_.size() + kValueBytes);
        return reinterpret_cast<_value_type*>(&*(pool_.end()-kValueBytes));
    }
    
    void _pop_block(_index_type block) {
        auto succ = basic_block_[block].succ();
        if (succ == block) {
            assert(enabled_blocks_head_ == block);
            enabled_blocks_head_ = kInitialEmptyBlockHead;
        } else {
            auto pred = basic_block_[block].pred();
            basic_block_[pred].set_succ(succ);
            basic_block_[succ].set_pred(pred);
            if (block == enabled_blocks_head_) {
                enabled_blocks_head_ = succ;
            }
        }
    }
    
    void _push_block(_index_type block) {
        if (enabled_blocks_head_ == kInitialEmptyBlockHead) {
            basic_block_[block].set_pred(block);
            basic_block_[block].set_succ(block);
            enabled_blocks_head_ = block;
        } else {
            auto tail = basic_block_[enabled_blocks_head_].pred();
            basic_block_[enabled_blocks_head_].set_pred(block);
            basic_block_[block].set_succ(enabled_blocks_head_);
            basic_block_[block].set_pred(tail);
            basic_block_[tail].set_succ(block);
        }
    }
    
private:
    void _freeze(size_t index) {
        assert(_empty_at(index));
        // Remove empty-elements linking
        size_t succ = container_[index].succ();
        size_t pred = container_[index].pred();
        auto block_index = _block_index_of(index);
        auto offset = kBlockSize * block_index;
        container_[offset+succ].set_pred(pred);
        container_[offset+pred].set_succ(succ);
        auto b = basic_block_[block_index];
        auto inset = index % kBlockSize;
        if (succ == inset) {
            assert(bit_util::popcnt256(b.unit_field_ptr()) == 1);
            b.disable_link();
        } else if (inset == b.empty_head()) {
            b.set_empty_head(succ);
        }
        b.freeze_element_at(index%kBlockSize);
    }
    
    void _thaw(size_t index) {
        assert(not _empty_at(index));
        auto block_index = _block_index_of(index);
        auto b = basic_block_[block_index];
        b.thaw_element_at(index%kBlockSize);
        
        // Append empty-elements linking
        auto inset = index % kBlockSize;
        if (not b.link_enabled()) {
            assert(bit_util::popcnt256(b.unit_field_ptr()) == 1);
            container_[index].init_disabled_unit(inset, inset);
            b.set_empty_head(inset);
        } else {
            auto offset = kBlockSize * (block_index);
            auto head = b.empty_head();
            auto head_unit = container_[offset+head];
            auto tail = head_unit.pred();
            head_unit.set_pred(inset);
            container_[offset+tail].set_succ(inset);
            container_[index].init_disabled_unit(tail, head);
        }
    }
    
};


// MARK: - Patricia Trie

template <typename ValueType, typename IndexType>
class _CompactDoubleArrayBcPatriciaTrieImpl : public _CompactDoubleArrayMpTrieImpl<ValueType, IndexType> {
    using _base = _CompactDoubleArrayMpTrieImpl<ValueType, IndexType>;
public:
    using typename _base::_value_type;
    using typename _base::_index_type;
    using typename _base::_char_type;
    
    using _base::kLeafChar;
    using _base::kEmptyChar;
    using _base::kIndexBytes;
    using _base::kEmptyFlag;
    using _base::kIndexMask;
    
public:
    _CompactDoubleArrayBcPatriciaTrieImpl() : _base() {}
    
    _CompactDoubleArrayBcPatriciaTrieImpl(std::ifstream& ifs) : _base(ifs) {}
    
    _CompactDoubleArrayBcPatriciaTrieImpl(_CompactDoubleArrayBcPatriciaTrieImpl&&) = default;
    _CompactDoubleArrayBcPatriciaTrieImpl& operator=(_CompactDoubleArrayBcPatriciaTrieImpl&&) = default;
    
    virtual ~_CompactDoubleArrayBcPatriciaTrieImpl() = default;
    
    _index_type _base_in_pool(_index_type pool_index) const {
        const _index_type* base_ptr = reinterpret_cast<const _index_type*>(_base::pool_.data() + pool_index);
        return *base_ptr bitand kIndexMask;
    }
    
    void _set_base_in_pool(_index_type pool_index, _index_type new_base) {
        _index_type* base_ptr = reinterpret_cast<_index_type*>(_base::pool_.data() + pool_index);
        *base_ptr = new_base bitand kIndexMask;
    }
    
    _index_type _base_at(_index_type node) const override {
        assert(not _base::container_[node].is_leaf());
        auto unit = _base::container_[node];
        return not unit.has_label() ? unit.base() : _base_in_pool(unit.pool_index());
    }
    
    void _set_base_at(_index_type node, _index_type base) override {
        auto unit = _base::container_[node];
        if (not unit.has_label()) {
            unit.set_base(base);
        } else {
            assert(not unit.label_is_suffix());
            _set_base_in_pool(unit.pool_index(), base);
        }
    }
    
    _index_type _append_internal_label_in_pool(std::string_view key, _index_type new_base) {
        assert(not key.empty());
        _index_type index = _base::pool_.size();
        _base::pool_.resize(_base::pool_.size() + kIndexBytes + key.size() + 1);
        _set_base_in_pool(index, new_base);
        auto ptr = _base::pool_.data()+index+kIndexBytes;
        for (_char_type c : key)
            *(ptr++) = c;
        *ptr = kLeafChar;
        return index;
    }
    
};


// MARK: - Dynamic Double-array Interface

template <typename ValueType,
          typename IndexType,
          bool     Ordered     = false,
          size_t   MaxTrial    = 1,
          bool     LegacyBuild = true,
          bool     Patricia    = true>
class CompactDoubleArrayTrie;


template <typename ValueType, typename IndexType, bool Ordered, size_t MaxTrial, bool LegacyBuild>
class CompactDoubleArrayTrie<ValueType, IndexType, Ordered, MaxTrial, LegacyBuild, false> : public _CompactDoubleArrayMpTrieImpl<ValueType, IndexType> {
    using _self = CompactDoubleArrayTrie<ValueType, IndexType, Ordered, MaxTrial, LegacyBuild, false>;
    using _impl = _CompactDoubleArrayMpTrieImpl<ValueType, IndexType>;
    
    using _constructor = _DynamicDoubleArrayMpTrieConstructor<Ordered, MaxTrial, not LegacyBuild, _impl>;

private:
    _constructor constructor_;
    
public:
    using value_type = typename _impl::_value_type;
    using index_type = typename _impl::_index_type;
    using char_type = typename _impl::_char_type;
    
    static constexpr index_type kRootIndex = _impl::kRootIndex;
    static constexpr char_type kLeafChar = _impl::kLeafChar;
    
    static constexpr size_t kBlockSize = _impl::kBlockSize;
    
    CompactDoubleArrayTrie() : _impl(), constructor_(*this) {
        constructor_.init();
    }
    
    CompactDoubleArrayTrie(std::ifstream& ifs) : _impl(ifs), constructor_(*this) {}
    
    template <typename StrIter,
    typename Traits = std::iterator_traits<StrIter>>
    explicit CompactDoubleArrayTrie(StrIter begin, StrIter end) : _impl(), constructor_(*this) {
        _arrange_keysets(begin, end, 0, kRootIndex);
    }
    
    CompactDoubleArrayTrie(const std::vector<std::string>& key_set) : CompactDoubleArrayTrie(key_set.begin(), key_set.end()) {}
    
    template <bool _Ordered, size_t _MaxTrial, bool _LegacyBuild>
    explicit CompactDoubleArrayTrie(CompactDoubleArrayTrie<ValueType, IndexType, _Ordered, _MaxTrial, _LegacyBuild, false>&& x) : _impl(std::move(_impl(x))), constructor_(*this) {}
    template <bool _Ordered, size_t _MaxTrial, bool _LegacyBuild>
    CompactDoubleArrayTrie& operator=(CompactDoubleArrayTrie<ValueType, IndexType, _Ordered, _MaxTrial, _LegacyBuild, false>&& x) {
        (_impl&) *this = std::move((_impl&) x);
        return *this;
    }
    
    ~CompactDoubleArrayTrie() = default;
    
    CompactDoubleArrayTrie& rebuild() {
        CompactDoubleArrayTrie<ValueType, IndexType, Ordered, 32, LegacyBuild, false> new_da;
        new_da._arrange_da(*this, kRootIndex, kRootIndex);
        *this = std::move(new_da);
        return *this;
    }

    void shrink_to_fit() {
        rebuild();
    }
    
    size_t size_in_bytes() const {return _impl::_size_in_bytes();}
    
    size_t bc_size_in_bytes() const {return _impl::container_.size_in_bytes();}
    
    size_t pool_size_in_bytes() const {return size_vec(_impl::pool_);}
    
    size_t succinct_size_in_bytes() const {return size_in_bytes() - bc_size_in_bytes() - pool_size_in_bytes();}
    
    size_t bc_blank_size_in_bytes() const {return _impl::kUnitBytes * (_impl::container_.size() - num_nodes());}
    
    size_t pool_blank_size_in_bytes() const {
        size_t blank_size = _impl::pool_.size();
        for (size_t i = 0; i < _impl::container_.size(); i++) {
            auto unit = _impl::container_[i];
            if (_impl::_empty_at(i) or not unit.has_label())
                continue;
            blank_size -= _impl::_suffix_in_pool(unit.pool_index()).size()+1+_impl::kValueBytes;
        }
        return blank_size;
    }
    
    size_t blank_size_in_bytes() const {return bc_blank_size_in_bytes() + pool_blank_size_in_bytes();}
    
    float load_factor() const {return (float)1 - ((float)blank_size_in_bytes() / size_in_bytes());}
    
    float load_factor_bc() const {return (float)1 - ((float)bc_blank_size_in_bytes() / bc_size_in_bytes());}
    
    float load_factor_pool() const {return (float)1 - ((float)pool_blank_size_in_bytes() / pool_size_in_bytes());}
    
    size_t num_nodes() const {
        size_t cnt = 0;
        for (size_t i = 0; i < _impl::basic_block_.size(); i++) {
            cnt += 256 - _impl::basic_block_[i].num_empties();
        }
        return cnt;
    }
    
    value_type* find(std::string_view key) const {
        return _traverse(key, [](auto) {},
                         [](auto, auto) {return nullptr;},
                         [](auto, auto, auto) {return nullptr;}).first;
    }
    
    std::pair<value_type*, bool> insert(std::string_view key) {
        auto [ptr, res] = _traverse(key, [](auto) {},
                             [&](index_type node, size_t key_pos) {
                                 return constructor_._insert_in_bc(node, key.substr(key_pos));
                             }, [&](index_type node, size_t pool_pos, size_t key_pos) {
                                 return constructor_._insert_in_tail(node, pool_pos, key.substr(key_pos));
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
        return _traverse(key,
                         [&](index_type node) {
                             constructor_._delete_leaf(node);
                         },
                         [](auto, auto) {return nullptr;}, [](auto, auto, auto) {return nullptr;}).second;
    }
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex] \tterminal, \tcheck, \tsibling, \thas label, \tbase, \tchild"  << std::endl;
        for (size_t i = 0; i < std::min(_impl::container_.size(), (size_t)0x10000); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            auto unit = _impl::container_[i];
            std::cout << "\t\t"<<i<<"] \t" << unit.child()<<"| \t";
            auto empty =_impl::_empty_at(i);
            if (not empty) {
                std::cout<<unit.check()<<", \t"<<unit.sibling()<<", \t"<<unit.has_label();
                if (unit.has_label())
                    std::cout<<", \t"<<unit.pool_index();
                if (not unit.is_leaf())
                    std::cout<<", \t"<<_impl::_base_at(i);
            } else {
                std::cout<<(size_t)unit.pred()<<", \t"<<(size_t)unit.succ();
            }
            std::cout << std::endl;
        }
    }
    
    std::array<size_t, 257> get_num_of_children_table() const {
        std::array<size_t, 257> table = {};
        auto make_table = [f=[&](auto dfs, index_type node) {
            if (_impl::container_[node].is_leaf())
                return;
            auto cnt = 0;
            _impl::_for_each_children(node, [&](index_type index, auto) {
                dfs(dfs, index);
                ++cnt;
            });
            table[cnt]++;
        }] {
            f(f, kRootIndex);
        };
        make_table();
        return table;
    }
    
    void print_num_of_children_table() const {
        auto table = get_num_of_children_table();
        std::cout << "num of children | counts" << std::endl;
        for (int i = 0; i <= 256; i++)
            std::cout << table[i] << std::endl;
    }
    
    template <bool O, size_t MT, bool LB>
    void _arrange_da(const CompactDoubleArrayTrie<ValueType, IndexType, O, MT, LB, false>& da, const index_type node, const index_type co_node) {
        std::vector<typename _constructor::_internal_label_container> label_datas;
        std::vector<char_type> children;
        da._for_each_children(node, [&](index_type index, char_type child) {
            children.push_back(child);
            std::string label;
            label.push_back(child);
            auto unit = da.unit_at(index);
            if (unit.has_label())
                label += da._suffix_in_pool(unit.pool_index());
            while (da._is_single_node(index)) {
                auto child = da.unit_at(da._base_at(index)).child();
                label += child;
                da._transition_bc(index, child);
                unit = da.unit_at(index);
                if (unit.has_label())
                    label += da._suffix_in_pool(unit.pool_index());
            }
            label_datas.push_back({label, unit.label_is_suffix()});
        });
        
        auto new_base = constructor_._find_base(children);
        constructor_._insert_nodes(co_node, label_datas, new_base);
        for (auto label_data : label_datas) {
            auto target = node;
            auto c = label_data.label.front();
            da._transition_bc(target, c);
            if (not label_data.suffix)
                _arrange_da(da, target, new_base xor c);
        }
    }
    
    template <class SuccessAction, class FailedInBcAction, class FailedInSuffixAction>
    std::pair<value_type*, bool> _traverse(std::string_view key, SuccessAction success, FailedInBcAction failed_in_bc, FailedInSuffixAction failed_in_suffix) const {
        index_type node = kRootIndex;
        size_t key_pos = 0;
        for (; key_pos < key.size(); key_pos++) {
            if (not _transition_bc(node, key[key_pos])) {
                return {failed_in_bc(node, key_pos), false};
            }
            if (_impl::container_[node].has_label()) {
                auto [ptr, res] = _transition_suffix(node, key, ++key_pos, failed_in_suffix);
                if (not res) {
                    return {ptr, false};
                }
                success(node);
                return {ptr, true};
            }
        }
        if (not _transition_bc(node, kLeafChar)) {
            return {failed_in_bc(node, key_pos), false};
        }
        success(node);
        return {const_cast<value_type*>(reinterpret_cast<const value_type*>(_impl::pool_.data()+_impl::container_[node].pool_index())), true};
    }
    
    bool _transition_bc(index_type& node, char_type c) const {
        if (_impl::container_[node].is_leaf())
            return false;
        auto next = _impl::_base_at(node) xor c;
        if (_impl::_empty_at(next) or
            _impl::container_[next].check() != c)
            return false;
        node = next;
        return true;
    }
    
    template <class FailedAction>
    std::pair<value_type*, bool> _transition_suffix(index_type node, std::string_view key, size_t& key_pos, FailedAction failed) const {
        assert(_impl::container_[node].has_label());
        auto pool_index = _impl::container_[node].pool_index();
        char_type* pool_ptr = (char_type*)_impl::pool_.data() + pool_index;
        size_t i = 0;
        while (key_pos < key.size()) {
            char_type char_in_tail = *pool_ptr;
            if (char_in_tail == kLeafChar or
                char_in_tail != (char_type)key[key_pos]) {
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
        return {reinterpret_cast<value_type*>(pool_ptr+1), true};
    }
    
    template <typename StrIter,
    typename Traits = std::iterator_traits<StrIter>>
    void _arrange_keysets(StrIter begin, StrIter end, size_t depth, index_type co_node) {
        if (begin >= end)
            return;
        
        std::vector<typename _constructor::_internal_label_container> label_datas;
        std::vector<char_type> children;
        while (begin < end and ((*begin).size() == depth)) {
            label_datas.push_back({"", true});
            children.push_back(kLeafChar);
            ++begin;
        }
        std::vector<StrIter> iters = {begin};
        std::string_view front_label((*begin).data() + depth);
        char_type prev_key = (*begin)[depth];
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
            char_type c = (*it)[depth];
            if (c != prev_key) {
                append(it-1);
                front_label = std::string_view((*it).data() + depth);
                prev_key = c;
            }
        }
        append(end-1);
        
        auto new_base = constructor_._find_base(children);
        constructor_._insert_nodes(co_node, label_datas, new_base);
        for (size_t i = 0; i < label_datas.size(); i++) {
            auto label = label_datas[i].label;
            if (not label_datas[i].suffix)
                _arrange_keysets(iters[i], iters[i+1], depth+label.size(), new_base xor (char_type)label.front());
        }
    }
    
};


template <typename ValueType, typename IndexType, bool Ordered, size_t MaxTrial, bool LegacyBuild>
class CompactDoubleArrayTrie<ValueType, IndexType, Ordered, MaxTrial, LegacyBuild, true> : public _CompactDoubleArrayBcPatriciaTrieImpl<ValueType, IndexType> {
    using _self = CompactDoubleArrayTrie<ValueType, IndexType, Ordered, MaxTrial, LegacyBuild, true>;
    using _impl = _CompactDoubleArrayBcPatriciaTrieImpl<ValueType, IndexType>;
    
    using _constructor = _DynamicDoubleArrayPatriciaTrieConstructor<Ordered, MaxTrial, not LegacyBuild, _impl>;
    
private:
    _constructor constructor_;
    
public:
    using value_type = typename _impl::_value_type;
    using index_type = typename _impl::_index_type;
    using char_type = typename _impl::_char_type;
    
    static constexpr index_type kRootIndex = _impl::kRootIndex;
    static constexpr char_type kLeafChar = _impl::kLeafChar;
    
    static constexpr size_t kBlockSize = _impl::kBlockSize;
    
    CompactDoubleArrayTrie() : _impl(), constructor_(*this) {
        constructor_.init();
    }
    
    CompactDoubleArrayTrie(std::ifstream& ifs) : _impl(ifs), _constructor(*this) {}
    
    template <typename StrIter,
    typename Traits = std::iterator_traits<StrIter>>
    CompactDoubleArrayTrie(StrIter begin, StrIter end) : _impl(), constructor_(*this) {
        _arrange_keysets(begin, end, 0, kRootIndex);
    }
    
    CompactDoubleArrayTrie(const std::vector<std::string>& key_set) : CompactDoubleArrayTrie(key_set.begin(), key_set.end()) {}
    
    template <bool _Ordered, size_t _MaxTrial, bool _LegacyBuild>
    explicit CompactDoubleArrayTrie(CompactDoubleArrayTrie<ValueType, IndexType, _Ordered, _MaxTrial, _LegacyBuild, true>&& x) : _impl(std::move(_impl(x))), constructor_(*this) {}
    template <bool _Ordered, size_t _MaxTrial, bool _LegacyBuild>
    CompactDoubleArrayTrie& operator=(CompactDoubleArrayTrie<ValueType, IndexType, _Ordered, _MaxTrial, _LegacyBuild, true>&& x) {
        (_impl&) *this = std::move((_impl&) x);
        return *this;
    }
    
    ~CompactDoubleArrayTrie() = default;
    
    CompactDoubleArrayTrie& rebuild() {
        CompactDoubleArrayTrie<ValueType, IndexType, Ordered, 32, LegacyBuild, true> new_da;
        new_da._arrange_da(*this, kRootIndex, kRootIndex);
        *this = std::move(new_da);
        return *this;
    }

    void shrink_to_fit() {
        rebuild();
    }
    
    size_t size_in_bytes() const {return _impl::_size_in_bytes();}
    
    size_t bc_size_in_bytes() const {return _impl::container_.size_in_bytes();}
    
    size_t pool_size_in_bytes() const {return size_vec(_impl::pool_);}
    
    size_t succinct_size_in_bytes() const {return size_in_bytes() - bc_size_in_bytes() - pool_size_in_bytes();}
    
    size_t bc_blank_size_in_bytes() const {return _impl::kUnitBytes * (_impl::container_.size() - num_nodes());}
    
    size_t pool_blank_size_in_bytes() const {
        size_t blank_size = _impl::pool_.size();
        for (size_t i = 0; i < _impl::container_.size(); i++) {
            auto unit = _impl::container_[i];
            if (_impl::_empty_at(i) or not unit.has_label())
                continue;
            auto pool_index = unit.pool_index();
            if (unit.label_is_suffix())
                blank_size -= _impl::_suffix_in_pool(pool_index).size()+1+_impl::kValueBytes;
            else
                blank_size -= _impl::kIndexBytes + _impl::_suffix_in_pool(pool_index+_impl::kIndexBytes).size()+1;
        }
        return blank_size;
    }
    
    size_t blank_size_in_bytes() const {return bc_blank_size_in_bytes() + pool_blank_size_in_bytes();}
    
    float load_factor() const {return (float)1 - ((float)blank_size_in_bytes() / size_in_bytes());}
    
    float load_factor_bc() const {return (float)1 - ((float)bc_blank_size_in_bytes() / bc_size_in_bytes());}
    
    float load_factor_pool() const {return (float)1 - ((float)pool_blank_size_in_bytes() / pool_size_in_bytes());}
    
    size_t num_nodes() const {
        size_t cnt = 0;
        for (size_t i = 0; i < _impl::basic_block_.size(); i++) {
            cnt += 256 - _impl::basic_block_[i].num_empties();
        }
        return cnt;
    }
    
    void serialize(std::ifstream& ifs) const {
        _impl::_serialize(ifs);
    }
    
    value_type* find(std::string_view key) const {
        return _traverse(key, [](auto){},
                         [](auto, auto) {return nullptr;},
                         [](auto, auto, auto) {return nullptr;},
                         [](auto, auto, auto) {return nullptr;}).first;
    }
    
    std::pair<value_type*, bool> insert(std::string_view key) {
        auto [ptr, res] = _traverse(key, [](auto){},
                             [&](index_type node, size_t key_pos) {
                                 return constructor_._insert_in_bc(node, key.substr(key_pos));
                             },
                             [&](index_type node, size_t label_pos, size_t key_pos) {
                                 return constructor_._insert_in_internal_label(node, label_pos, key.substr(key_pos));
                             },
                             [&](index_type node, size_t label_pos, size_t key_pos) {
                                 return constructor_._insert_in_suffix(node, label_pos, key.substr(key_pos));
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
        return _traverse(key, [&](index_type node) {
                             constructor_._delete_leaf(node);
                         }, [](auto, auto) {return nullptr;},
                         [](auto, auto, auto) {return nullptr;},
                         [](auto, auto, auto) {return nullptr;}).second;
    }
    
    void print_for_debug() const {
        std::cout << "------------ Double-array implementation ------------" << std::endl;
        std::cout << "\tindex] \texists, \tcheck, \tsibling, \tbase, \tchild"  << std::endl;
        for (size_t i = 0; i < std::min(_impl::container_.size(), (size_t)0x10000); i++) {
            if (i % 0x100 == 0)
                std::cout << std::endl;
            auto unit = _impl::container_[i];
            std::cout << "\t\t"<<i<<"] \t" << unit.child()<<"| \t";
            auto empty =_impl::_empty_at(i);
            if (not empty) {
                std::cout<<unit.check()<<", \t"<<unit.sibling()<<", \t"<<unit.has_label();
                if (unit.has_label())
                    std::cout<<", \t"<<unit.pool_index();
                if (not unit.is_leaf())
                    std::cout<<", \t"<<_impl::_base_at(i);
            } else {
                std::cout<<(size_t)unit.pred()<<", \t"<<(size_t)unit.succ();
            }
            std::cout << std::endl;
        }
    }
    
    std::array<size_t, 257> get_num_of_children_table() const {
        std::array<size_t, 257> table = {};
        auto make_table = [f = [&](auto dfs, index_type node) {
            if (_impl::container_[node].is_leaf())
                return;
            auto cnt = 0;
            _impl::_for_each_children(node, [&](index_type index, auto) {
                dfs(dfs, index);
                ++cnt;
            });
            table[cnt]++;
        }] {
            f(f, kRootIndex);
        };
        make_table();
        return table;
    }
    
    void print_num_of_children_table() const {
        auto table = get_num_of_children_table();
        std::cout << "num of children | counts" << std::endl;
        for (int i = 0; i <= 256; i++)
            std::cout << table[i] << std::endl;
    }
    
    template <bool O, size_t MT, bool LB>
    void _arrange_da(const CompactDoubleArrayTrie<ValueType, IndexType, O, MT, LB>& da, const index_type node, const index_type co_node) {
        std::vector<typename _constructor::_internal_label_container> label_datas;
        std::vector<char_type> children;
        da._for_each_children(node, [&](index_type index, char_type child) {
            children.push_back(child);
            std::string label;
            label.push_back(child);
            auto unit = da.unit_at(index);
            assert(unit.check() == child);
            if (unit.has_label())
                label += da._suffix_in_pool(unit.pool_index()+(unit.label_is_suffix()?0:_impl::kIndexBytes));
            while (da._is_single_node(index)) {
                auto base = da._base_at(index);
                auto child = da.unit_at(base).child();
                label += child;
                da._transition_bc(index, base, child);
                unit = da.unit_at(index);
                if (unit.has_label())
                    label += da._suffix_in_pool(unit.pool_index()+(unit.label_is_suffix()?0:_impl::kIndexBytes));
            }
            label_datas.push_back({label, unit.label_is_suffix()});
        });
        
        auto new_base = constructor_._find_base(children);
        constructor_._insert_nodes(co_node, label_datas, new_base);
        for (auto label_data : label_datas) {
            auto target = node;
            uint8_t c = label_data.label.front();
            auto res = da._transition_bc(target, da._base_at(target), c);
            assert(res);
            if (not label_data.suffix)
                _arrange_da(da, target, new_base xor c);
        }
    }
    
    template <class SuccessAction, class FailedInBcAction, class FailedInInternalLabelAction, class FailedInSuffixAction>
    std::pair<value_type*, bool> _traverse(std::string_view key, SuccessAction success, FailedInBcAction failed_in_bc, FailedInInternalLabelAction failed_in_internal_label, FailedInSuffixAction failed_in_suffix) const {
        if (_impl::container_[kRootIndex].is_leaf()) {
            // First insertion
            return {failed_in_bc(kRootIndex, 0), false};
        }
        index_type node = kRootIndex;
        index_type base = _impl::_base_at(node);
        for (size_t key_pos = 0; key_pos < key.size(); key_pos++) {
            if (not _transition_bc(node, base, key[key_pos])) {
                return {failed_in_bc(node, key_pos), false};
            }
            auto unit = _impl::container_[node];
            if (unit.has_label()) {
                if (not unit.label_is_suffix()) {
                    auto [nbase, ptr, res] = _transition_internal_label(node, key, ++key_pos, failed_in_internal_label);
                    if (not res)
                        return {ptr, false};
                    base = nbase;
                } else {
                    auto [ptr, res] =  _transition_suffix(node, key, ++key_pos, failed_in_suffix);
                    if (not res)
                        return {ptr, false};
                    success(node);
                    return {ptr, true};
                }
            } else {
                base = unit.base();
            }
        }
        if (not _transition_bc(node, base, kLeafChar)) {
            return {failed_in_bc(node, key.size()), false};
        }
        success(node);
        return {const_cast<value_type*>(reinterpret_cast<const value_type*>(_impl::pool_.data()+_impl::container_[node].pool_index())), true};
    }
    
    bool _transition_bc(index_type& node, index_type base, char_type c) const {
        auto next = base xor c;
        if (_impl::_empty_at(next) or
            _impl::container_[next].check() != c)
            return false;
        node = next;
        return true;
    }
    
    template <class FailedAction>
    std::tuple<index_type, value_type*, bool> _transition_internal_label(index_type node, std::string_view key, size_t& key_pos, FailedAction failed) const {
        auto pool_index = _impl::container_[node].pool_index();
        auto base = _impl::_base_in_pool(pool_index);
        auto label_index = pool_index + _impl::kIndexBytes;
        char_type* pool_ptr = (char_type*)_impl::pool_.data() + label_index;
        assert(*pool_ptr != kLeafChar);
        size_t i = 0;
        while (key_pos < key.size()) {
            char_type char_in_label = *pool_ptr;
            if (char_in_label == kLeafChar) {
                --key_pos;
                return {base, nullptr, true};
            }
            if (char_in_label != (char_type)key[key_pos]) {
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
    std::pair<value_type*, bool> _transition_suffix(index_type node, std::string_view key, size_t& key_pos, FailedAction failed) const {
        auto label_index = _impl::container_[node].pool_index();
        char_type* pool_ptr = (char_type*)_impl::pool_.data() + label_index;
        size_t i = 0;
        while (key_pos < key.size()) {
            char_type char_in_label = *pool_ptr;
            if (char_in_label == kLeafChar or
                char_in_label != (char_type)key[key_pos]) {
                return {failed(node, label_index+i, key_pos), false};
            }
            ++pool_ptr;
            i++;
            key_pos++;
        }
        if (*pool_ptr != kLeafChar) {
            return {failed(node, label_index+i, key_pos), false};
        }
        --key_pos;
        return {reinterpret_cast<value_type*>(pool_ptr+1), true};
    }
    
    template <typename StrIter,
    typename Traits = std::iterator_traits<StrIter>>
    void _arrange_keysets(StrIter begin, StrIter end, size_t depth, index_type co_node) {
        if (begin >= end)
            return;
        
        std::vector<typename _constructor::_internal_label_container> label_datas;
        std::vector<char_type> children;
        if ((*begin).size() == depth) {
            label_datas.push_back({"", true});
            children.push_back(kLeafChar);
            ++begin;
        }
        if (begin == end) {
            constructor_._insert_nodes(co_node, label_datas, constructor_._find_base(children));
        } else {
            assert(begin < end);
            assert(begin->size() > depth);
            std::vector<StrIter> iters = {begin};
            std::string_view front_label(begin->data() + depth);
            char_type prev_key = begin->size() <= depth ? kLeafChar : (*begin)[depth];
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
                char_type c = (*it)[depth];
                if (c != prev_key) {
                    append(it-1);
                    front_label = std::string_view(it->data() + depth);
                    prev_key = c;
                }
            }
            append(end-1);
            auto new_base = constructor_._find_base(children);
            constructor_._insert_nodes(co_node, label_datas, new_base);
            for (size_t i = 0; i < iters.size()-1; i++) {
                auto& ld = label_datas[i+(children.front()==kLeafChar?1:0)];
                auto label = ld.label;
                if (not ld.suffix)
                    _arrange_keysets(iters[i], iters[i+1], depth+label.size(), new_base xor (char_type)label.front());
            }
        }
        
    }
    
};


template <typename ValueType>
using u32DoubleArray = CompactDoubleArrayTrie<ValueType, uint32_t>;
template <typename ValueType>
using u64DoubleArray = CompactDoubleArrayTrie<ValueType, uint64_t>;

} // namespace sim_ds

#endif /* DoubleArray_hpp */
    
