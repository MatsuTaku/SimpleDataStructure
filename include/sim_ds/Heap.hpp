//
//  Heap.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2018/12/24.
//
//  Reference:
//    https://medium.com/@yasufumy/data-structure-heap-ecfd0989e5be
//

#ifndef Heap_hpp
#define Heap_hpp

#include "basic.hpp"

namespace sim_ds {

template <bool Ascending = true>
class Heap {
public:
    Heap() = default;
    
    template <typename T>
    Heap(const std::vector<T> array) {
        base_.reserve(array.size());
        for (auto v : array) {
            base_.push_back(v);
        }
        Build();
    }
    
    template <class Iterator>
    explicit Heap(Iterator begin, Iterator end) {
        base_.reserve(end - begin);
        while (begin < end) {
            base_.push_back(*begin);
            ++begin;
        }
        Build();
    }
    
    void Heapify(size_t id) {
        if (is_leaf(id))
            return;
        auto l = left(id);
        auto min_i = l;
        auto r = right(id);
        if (r <= size()) {
            if (!Compare(value(l), value(r))) {
                min_i = r;
            }
        }
        if (!Compare(value(id), value(min_i))) {
            std::swap(base_[id - 1], base_[min_i - 1]);
            Heapify(min_i);
        }
    }
    
    void Build() {
        for (size_t i = size() / 2; i > 0; i--) {
            Heapify(i);
        }
    }
    
    void push(id_type value) {
        base_.insert(base_.begin(), value);
        Heapify(root());
    }
    
    void pop() {
        std::swap(base_.front(), base_.back());
        base_.resize(base_.size() - 1);
        Heapify(root());
    }
    
    id_type value(size_t id) const {
        assert(id > 0 && id <= size());
        return base_[id - 1];
    }
    
    id_type& value_reference(size_t id) {
        assert(id > 0 && id <= size());
        return base_[id - 1];
    }
    
    id_type front() const {
        return base_.front();
    }
    
    size_t root() const {
        return 1;
    }
    
    size_t parent(size_t id) const {
        return id / 2;
    }
    
    size_t left(size_t id) const {
        return 2 * id;
    }
    
    bool is_leaf(size_t id) const {
        return id > (size() / 2);
    }
    
    size_t right(size_t id) const {
        return 2 * id + 1;
    }
    
    size_t size() const {
        return base_.size();
    }
    
private:
    std::vector<id_type> base_;
    
    bool Compare(id_type l, id_type r) {
        return Ascending ? l < r : l > r;
    }
    
};

using MinHeap = Heap<true>;
using MaxHeap = Heap<false>;

}

#endif /* Heap_hpp */
