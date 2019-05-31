//
//  log.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/21.
//

#ifndef log_hpp
#define log_hpp

#include <cstdio>
#include <iomanip>

namespace sim_ds {
    
template <typename T>
inline void ShowAsBinary(T value, size_t size = sizeof(T)) {
    using std::cout;
    using std::endl;
    for (int i = size * 8 - 1; i >= 0; i--) {
        cout << (value >> i & 1);
        if (i % 8 == 0)
            cout << ' ';
    }
    cout << endl;
}
    
} // namespace sim_ds

#endif /* log_hpp */
