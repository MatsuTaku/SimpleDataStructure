//
//  sort.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2018/12/24.
//

#ifndef sort_hpp
#define sort_hpp

#include "basic.hpp"
#include "Heap.hpp"

namespace sim_ds {

template <class Iterator, class Compare>
class HeapSorter {
private:
    Iterator begin_, end_;
    Compare compare_;
    
public:
    HeapSorter(Iterator begin, Iterator end, Compare compare) : begin_(begin), end_(end), compare_(compare) {
        for (auto id = size() / 2; id > 0; --id)
            Heapify(id);
        while (begin_ < end_ - 1) {
            std::swap(*begin_, *(end_ - 1));
            --end_;
            Heapify(1);
        }
        
    }
    
    size_t size() {return end_ - begin_;}
    
    Iterator itr(size_t pos) {return begin_ + (pos - 1);}
    
    bool is_leaf(size_t id) {return id > (size() / 2);}
    
    size_t left(size_t id) {return 2 * id;}
    
    size_t right(size_t id) {return 2 * id + 1;}
    
    void Heapify(size_t id) {
        if (is_leaf(id))
            return;
        auto l = left(id);
        auto min_i = l;
        auto r = right(id);
        if (r <= size()) {
            if (compare_(*itr(l), *itr(r))) {
                min_i = r;
            }
        }
        auto& id_v = *itr(id);
        auto& min_i_v = *itr(min_i);
        if (compare_(id_v, min_i_v)) {
            std::swap(id_v, min_i_v);
            Heapify(min_i);
        }
    }
    
};

template <class Iterator, class Compare>
inline void HeapSort(Iterator begin, Iterator end, Compare compare) {
    HeapSorter sorter(begin, end, compare);
}

template <class Iterator>
inline void HeapSort(Iterator begin, Iterator end) {
    return HeapSort(begin, end, [](auto l, auto r) {return l < r;});
}

}

#endif /* sort_hpp */
