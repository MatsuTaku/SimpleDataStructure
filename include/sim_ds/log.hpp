//
//  log.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/21.
//

#ifndef log_hpp
#define log_hpp

#include "basic.hpp"

namespace sim_ds {
    
namespace log {
    
inline void ShowAsBinary(size_t value, size_t size = sizeof(id_type)) {
    using std::cout;
    using std::endl;
    for (int i = size * 8 - 1; i >= 0; i--) {
        cout << (value >> i & 1);
    }
    cout << endl;
}
    
} // namespace log
    
} // namespace sim_ds

#endif /* log_hpp */
