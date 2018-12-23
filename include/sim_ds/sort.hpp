//
//  sort.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2018/12/24.
//

#ifndef sort_hpp
#define sort_hpp

#include "basic.hpp"
#include "MinHeap.hpp"

namespace sim_ds {

template <class Iterator>
inline void HeapSort(Iterator begin, Iterator end) {
    MinHeap heap(begin, end);
    while (begin < end) {
        *begin = heap.front();
        heap.pop();
        begin++;
    }
}

}

#endif /* sort_hpp */
