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
    
template <typename T>
auto DefaultCompare = [](T l, T r) {
    return l < r;
};

template <class Iterator, class Compare>
inline void HeapSort(Iterator begin, Iterator end) {
    Heap heap(begin, end);
    while (begin < end) {
        *begin = heap.front();
        heap.pop();
        begin++;
        std::sort(<#_RandomAccessIterator __first#>, <#_RandomAccessIterator __last#>, <#_Compare __comp#>)
    }
}

}

#endif /* sort_hpp */
