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

namespace sim_ds {
    
template <class VECTOR_TYPE>
class _EmptyLinkedVectorElement {
    using vector_type = VECTOR_TYPE;
    using value_type = typename vector_type::value_type;
    using element_type = uint64_t;
    using size_type = uint32_t;
    
    const element_type kNextMask = bit_util::width_mask<32>;
    const element_type kPrevMask = bit_util::width_mask<31>;
    const element_type kPrevPos = bit_util::offset_mask<32>;
    const element_type kExistsMask = bit_util::offset_mask<63>;
    const element_type kElementMask = bit_util::width_mask<63>;
    
private:
    friend typename vector_type::Self;
    
    element_type element_ = 0;
    
public:
    _EmptyLinkedVectorElement() = default;
    
    _EmptyLinkedVectorElement(size_type next, size_type prev) {
        set_next(next);
        set_prev(prev);
    }
    
    size_t next() const {return element_ & kNextMask;}
    
    size_t prev() const {return (element_ / kPrevPos) & kPrevMask;}
    
    bool empty() const {return not (element_ & kExistsMask);}
    
    value_type value() const {return element_ & kElementMask;}
    
private:
    void set_next(size_type next) {
        assert(empty());
        element_ &= 0xFFFFFFFF00000000;
        element_ |= next;
    }
    
    void set_prev(size_type prev) {
        assert(empty());
        element_ &= 0x80000000FFFFFFFF;
        element_ |= (prev & kPrevMask) * kPrevPos;
    }
    
    void set_value(element_type value) {
        assert(empty());
        element_ = bit_util::bextr(value, 0, 63);
        element_ |= kExistsMask;
    }
    
};


template <typename T, typename Value_Traits = uint64_t(T)>
class EmptyLinkedVector {
public:
    using value_type = T;
    using Self = EmptyLinkedVector<value_type>;
    using element_type = _EmptyLinkedVectorElement<Self>;
    
private:
    size_t empty_head_ = -1;
    std::vector<element_type> container_;
    
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
    }
    
    bool empty_at(size_t index) const {return container_[index].empty();}
    
    const element_type& operator[](size_t index) const {
        return container_[index];
    }
    
    element_type& operator[](size_t index) {
        return container_[index];
    }
    
    size_t empty_front_index() const {return empty_head_;}
    
    void set_value(size_t index, value_type new_value) {
        auto& target = operator[](index);
        assert(target.empty());
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
    }
    
};

}

#endif /* EmptyLinkedVector_hpp */
