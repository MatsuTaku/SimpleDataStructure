//
//  Log.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/21.
//

#ifndef Log_hpp
#define Log_hpp

#include "basic.hpp"

namespace sim_ds {
    
    class Log {
        Log() = delete;
    public:
        static void showAsBinary(size_t value, size_t size) {
            using std::cout;
            using std::endl;
            for (int i = size * 8 - 1; i >= 0; i--) {
                cout << (value >> i & 1);
            }
            cout << endl;
        }
        
    };
    
}

#endif /* Log_hpp */
