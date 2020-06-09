//
//  FitValuesArray.hpp
//  bench
//
//  Created by 松本拓真 on 2018/01/05.
//

#ifndef FitValuesArray_hpp
#define FitValuesArray_hpp

#include "basic.hpp"
#include "calc.hpp"

namespace sim_ds {


template <class SerializedSequence>
class BlockReference {
    using _self = BlockReference<SerializedSequence>;
    using Storage = SerializedSequence;
    using storage_type = typename SerializedSequence::storage_type;
    using storage_pointer = typename SerializedSequence::storage_pointer;
    
    static constexpr size_t kBitsPerWord = Storage::kBitsPerWord;
    
    storage_pointer pointer_;
    
    using table_type = typename SerializedSequence::table_type;
    using table_reference = const table_type&;
    table_reference element_table_;
    
    friend typename SerializedSequence::Self;
    
public:
    template <int Id>
    id_type get() const {
        return _get<Id, id_type>(element_table_[Id].size);
    }
    
    template <int Id, typename T>
    T restricted_get() const {
        return _get<Id, T>(std::min(size_t(element_table_[Id].size), sizeof(T)));
    }
    
    id_type get_at(size_t id) const {
        return _get_at(id);
    }
    
    template <int Id>
    id_type set(id_type value) {
        return _set<Id, id_type>(element_table_[Id].size, value);
    }
    
    template <int Id, typename T>
    T restricted_set(T value) {
        return _set<Id, T>(std::min(size_t(element_table_[Id].size), sizeof(T)), value);
    }
    
    template <typename T>
    id_type set_at(size_t id, T x) {
        return _set_at(id, x);
    }
    
    _self operator=(std::initializer_list<id_type> x) {
        size_t i = 0;
        for (auto v : x)
            set_at(i++, v);
    }
    
private:
    explicit BlockReference(storage_pointer pointer,
                            table_reference element_table
                            ) noexcept : pointer_(pointer), element_table_(element_table) {}
    
    template <int Id, typename T>
    T _get(size_t width) const {
        assert(Id < element_table_.size());
        assert(width <= element_table_[Id].size);
        
        id_type value = 0;
        auto relative_pointer = pointer_ + element_table_[Id].pos;
        for (size_t i = 0; i < width; i++)
            value |= static_cast<id_type>(*(relative_pointer + i)) << (i * kBitsPerWord);
        
        return value;
    }
    
    id_type _get_at(size_t id) const {
        id_type value = 0;
        auto relative_pointer = pointer_ + element_table_[id].pos;
        auto width = element_table_[id].size;
        for (size_t i = 0; i < width; i++)
            value |= static_cast<id_type>(*(relative_pointer + i)) << (i * kBitsPerWord);
        
        return value;
    }
    
    template <int Id, typename T>
    T _set(size_t width, T value) {
        assert(Id < element_table_.size());
        assert(width <= element_table_[Id].size);
        
        auto relative_pointer = pointer_ + element_table_[Id].pos;
        for (size_t i = 0; i < width; i++)
            *(relative_pointer + i) = static_cast<storage_type>(value >> (i * kBitsPerWord));
        
        return value;
    }
    
    template <typename T>
    T _set_at(size_t id, T value) {
        auto relative_pointer = pointer_ + element_table_[id].pos;
        auto width = element_table_[id].size;
        for (size_t i = 0; i < width; i++)
            *(relative_pointer + i) = static_cast<storage_type>(value >> (i * kBitsPerWord));
        
        return value;
    }
    
};


template <class SerializedSequence>
class BlockConstReference {
    using Storage = SerializedSequence;
    using storage_type = typename SerializedSequence::storage_type;
    using storage_pointer = typename SerializedSequence::const_storage_pointer;
    static constexpr size_t kBitsPerWord = SerializedSequence::kBitsPerWord;
    
    storage_pointer pointer_;
    
    using table_type = typename SerializedSequence::table_type;
    using table_reference = const table_type&;
    table_reference element_table_;
    
    friend typename SerializedSequence::Self;
    
public:
    template <int Id, typename T>
    T restricted_get() const {
        return get_<Id, T>(std::min(size_t(element_table_[Id].size), sizeof(T)));
    }
    
    template <int Id>
    id_type get() const {
        return get_<Id, id_type>(element_table_[Id].size);
    }
    
private:
    explicit BlockConstReference(storage_pointer pointer,
                                 table_reference element_table
                                 ) noexcept : pointer_(pointer), element_table_(element_table) {}
    
    template <int Id, typename T>
    T get_(size_t width) const {
        assert(Id < element_table_.size());
        assert(width <= element_table_[Id].size);
        
        id_type value = 0;
        auto relative_pointer = pointer_ + element_table_[Id].pos;
        for (size_t i = 0; i < width; i++)
            value |= static_cast<T>(*(relative_pointer + i)) << (i * kBitsPerWord);
        
        return value;
    }
    
};


class MultipleVector {
public:
    using Self = MultipleVector;
    using storage_type = uint8_t;
    using storage_pointer = storage_type*;
    using const_storage_pointer = const storage_type*;
    
    static constexpr size_t kBitsPerWord = sizeof(storage_type) * 8;
    
    using param_type = size_t;
    struct Element {
        param_type pos;
        param_type size;
        Element() = default;
        Element(param_type pos, param_type size) : pos(pos), size(size) {}
    };
    using table_type = std::vector<Element>;
    
    using reference = BlockReference<MultipleVector>;
    using const_reference = BlockConstReference<MultipleVector>;
    
    friend class BlockReference<MultipleVector>;
    friend class BlockConstReference<MultipleVector>;
    
protected:
    table_type element_table_;
    std::vector<storage_type> bytes_ = {};

    Element element_(size_t index) const {
      return element_table_[index];
    }

    uint8_t byte_(size_t index) const {
      return bytes_[index];
    }
    
    size_t offset_(size_t index) const {
        return index * block_size();
    }
    
    id_type set_(size_t offset, size_t width, id_type value) {
        for (size_t i = 0; i < width; i++)
            bytes_[offset + i] = static_cast<storage_type>(value >> (i * kBitsPerWord));
        
        return value;
    }
    
    id_type get_(size_t offset, size_t width) const {
        id_type value = 0;
        for (size_t i = 0; i < width; i++)
            value |= static_cast<id_type>(bytes_[offset + i]) << (i * kBitsPerWord);
        
        return value;
    }
    
public:
    MultipleVector() = default;
    
    template <typename T>
    MultipleVector(std::initializer_list<T> sizes) {
        size_t pos = 0;
        element_table_.reserve(sizes.size());
        for (auto size : sizes) {
            element_table_.emplace_back(pos, size);
            pos += size;
        }
    }
    
    template <typename T>
    void set_element_sizes(std::vector<T> sizes) {
        size_t pos = 0;
        element_table_.reserve(sizes.size());
        for (auto size : sizes) {
            element_table_.emplace_back(pos, size);
            pos += size;
        }
    }
    
    size_t block_size() const {
        auto& elem_back = element_table_.back();
        return elem_back.pos + elem_back.size;
    }
    
    size_t element_size(size_t id) const {
        return element_table_[id].size;
    }
    
    size_t size() const {
        return bytes_.size() / block_size();
    }
    
    reference block(size_t index) {
        return reference(&bytes_[offset_(index)], element_table_);
    }
    
    const_reference block(size_t index) const {
        return const_reference(&bytes_[offset_(index)], element_table_);
    }
    
    template <int Id>
    id_type set_nested_element(size_t index, id_type value) {
        assert(Id < element_table_.size());
        assert(index < size());
        assert(element_table_[Id].size == 8 || // 8 Byte element has no problem.
               sim_ds::calc::SizeFitsInBytes(value) <= element_table_[Id].size);
        return set_(offset_(index) + element_table_[Id].pos, element_table_[Id].size, value);
    }
    
    template <int Id>
    id_type nested_element(size_t index) const {
        assert(Id < element_table_.size());
        assert(index < size());
        return get_(offset_(index) + element_table_[Id].pos, element_table_[Id].size);
    }
    
    void reserve(size_t size) {
        bytes_.reserve(offset_(size));
    }
    
    void resize(size_t size) {
        bytes_.resize(offset_(size));
    }
    
    void resize(size_t new_size, std::initializer_list<id_type> values) {
        if (new_size < size()) {
            resize(new_size);
        } else if (new_size > size()) {
            auto prev_size = size();
            resize(new_size);
            for (size_t i = prev_size; i < new_size; i++) {
                auto b = block(i);
                size_t id = 0;
                for (auto v : values) {
                    b.set_at(id++, v);
                }
            }
        }
    }
    
    void emplace_back() {
        bytes_.resize(offset_(size() + 1));
    }
    
    void push_back(std::initializer_list<id_type> values) {
        bytes_.resize(offset_(size() + 1));
        size_t i = 0;
        auto b = block(size()-1);
        for (auto v : values) {
            b.set_at(i++, v);
        }
    }
    
    // MARK: IO
    
    size_t size_in_bytes() const {
        auto size = size_vec(bytes_);
        size += size_vec(element_table_) / 2;
        return size;
    }
    
    void LoadFrom(std::istream& is) {
        read_vec(is, bytes_);
        
        std::vector<param_type> element_sizes;
        read_vec(is, element_sizes);
        set_element_sizes(element_sizes);
    }
    
    void StoreTo(std::ostream& os) const {
        write_vec(bytes_, os);
        
        std::vector<param_type> element_sizes;
        for (auto table : element_table_) {
            element_sizes.push_back(table.size);
        }
        write_vec(element_sizes, os);
    }
    
};
    
}

#endif /* FitValuesArray_hpp */
