//
//  EmptyLinkedVector.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/05/24.
//

#ifndef EmptyLinkedVector_hpp
#define EmptyLinkedVector_hpp

#include "basic.hpp"
#include "bit_util.hpp"
#include "BitVector.hpp"

namespace sim_ds {
    
template <class VectorType>
class _EmptyLinkedVectorElement {
    using vector_type = VectorType;
    using value_type = typename vector_type::value_type;
    using size_type = uint32_t;
    
private:
    friend typename vector_type::Self;
    
    size_type next_ = 0;
    size_type prev_ = 0;
    
public:
    _EmptyLinkedVectorElement() = default;
    
    _EmptyLinkedVectorElement(size_type next, size_type prev) {
        set_next(next);
        set_prev(prev);
    }
    
    size_type next() const {return next_;}
    
    size_type prev() const {return prev_;}
    
    value_type value() const {
        if constexpr (sizeof(value_type) <= sizeof(size_type))
            return static_cast<value_type>(next_);
        else
            return static_cast<value_type>(uint64_t(next_) | uint64_t(prev_) << 32);
    }
    
private:
    void set_next(size_type next) {
        next_ = next;
    }
    
    void set_prev(size_type prev) {
        prev_ = prev;
    }
    
    void set_value(value_type value) {
        next_ = value & 0x00000000FFFFFFFF;
        if constexpr (sizeof(value_type) > sizeof(size_type))
            prev_ = (value >> 32);
        else
            prev_ = 0;
    }
    
};


template <typename T>
class EmptyLinkedVector {
public:
    using value_type = T;
    using Self = EmptyLinkedVector<value_type>;
    using element_type = _EmptyLinkedVectorElement<Self>;
    
private:
    size_t empty_head_ = -1;
    std::vector<element_type> container_;
    BitVector exists_;
    
public:
    EmptyLinkedVector() = default;
    EmptyLinkedVector(size_t size) {
        resize(size);
    }
    
    size_t size() const {return container_.size();}
    
    void resize(size_t new_size) {
        if (new_size > size()) {
            if (empty_head_ != -1) {
                auto& empty_back = operator[](operator[](empty_head_).prev());
                empty_back.set_next(container_.size());
            } else {
                empty_head_ = size();
            }
            auto old_size = size();
            container_.resize(new_size);
            
            for (size_t i = old_size; i < container_.size(); i++) {
                container_[i].set_next(i+1);
                container_[i].set_prev(i-1);
            }
            container_.back().set_next(empty_head_);
            operator[](empty_head_).set_prev(container_.size()-1);
        } else if (new_size < size()) {
            if (empty_head_ >= size())
                empty_head_ = -1;
            if (empty_head_ != -1) {
                operator[](empty_head_).set_prev(operator[](size()).prev());
                operator[](operator[](empty_head_).prev()).set_next(empty_head_);
            }
            container_.resize(new_size);
        }
        exists_.resize(new_size);
    }
    
    bool empty_at(size_t index) const {return not exists_[index];}
    
    const element_type& operator[](size_t index) const {
        return container_[index];
    }
    
    element_type& operator[](size_t index) {
        return container_[index];
    }
    
    size_t empty_front_index() const {return empty_head_;}
    
    void set_value(size_t index, value_type new_value) {
        assert(empty_at(index));
        auto& target = operator[](index);
        auto next = target.next();
        if (next == index) {
            assert(empty_head_ == index);
            empty_head_ = -1;
        } else {
            if (index == empty_head_)
                empty_head_ = next;
            auto prev = target.prev();
            container_[prev].set_next(next);
            container_[next].set_prev(prev);
        }
        target.set_value(new_value);
        exists_[index] = true;
    }
    
    const BitVector& bit_vector() const {return exists_;}
    
};

}

#endif /* EmptyLinkedVector_hpp */
