//
//  Exception.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/08.
//

#ifndef Exception_hpp
#define Exception_hpp

#include "basic.hpp"

namespace sim_ds {
    
    class DataNotFoundException : std::exception {
    public:
        DataNotFoundException(const std::string &data_name) : data_name_(data_name) {}
        std::string data_name_;
    };
    
    class DoesntHaveMemberException : std::exception {
    public:
        DoesntHaveMemberException(const std::string &text) : text(text) {}
        std::string text;
    };
    
}

#endif /* Exception_hpp */
