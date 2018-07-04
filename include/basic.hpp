//
//  basic.hpp
//  build
//
//  Created by 松本拓真 on 2018/04/25.
//

#ifndef basic_hpp
#define basic_hpp

#include <array>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <memory>
#include <sstream>
#include <cstring>
#include <fstream>
#include <limits>
#include <chrono>
#include <string_view>
#include <queue>

namespace sim_ds {
    
    class Stopwatch {
    public:
        Stopwatch() : start_(std::chrono::high_resolution_clock::now()) {}
        
        double get_sec() const {
            auto tp = std::chrono::high_resolution_clock::now() - start_;
            return std::chrono::duration<double>(tp).count();
        }
        double get_milli_sec() const {
            auto tp = std::chrono::high_resolution_clock::now() - start_;
            return std::chrono::duration<double, std::milli>(tp).count();
        }
        double get_micro_sec() const {
            auto tp = std::chrono::high_resolution_clock::now() - start_;
            return std::chrono::duration<double, std::micro>(tp).count();
        }
        
    private:
        std::chrono::high_resolution_clock::time_point start_;
    };
    
    template<typename T>
    inline void write_val(const T& val, std::ostream& os) {
        os.write(reinterpret_cast<const char*>(&val), sizeof(val));
    }
    
    template<typename T>
    inline void write_vec(const std::vector<T> &vec, std::ostream &os) {
        write_val(vec.size(), os);
        os.write(reinterpret_cast<const char*>(&vec[0]), sizeof(T) * vec.size());
    }
    
    inline void write_string(const std::string &str, std::ostream &os) {
        write_val(str.size(), os);
        os.write(reinterpret_cast<const char*>(&str[0]), sizeof(char) * str.size());
    }
    
    template<typename T>
    inline T read_val(std::istream& is) {
        T val;
        is.read(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    }
    
    template<typename T>
    inline std::vector<T> read_vec(std::istream& is) {
        auto size = read_val<size_t>(is);
        std::vector<T> vec(size);
        is.read(reinterpret_cast<char*>(&vec[0]), sizeof(T) * size);
        return vec; // expect move
    }
    
    inline std::string read_string(std::istream& is) {
        auto size = read_val<size_t>(is);
        std::string str(size, 0);
        is.read(reinterpret_cast<char*>(&str[0]), sizeof(char) * size);
        return str; // expect move
    }
    
    template<typename T>
    inline size_t size_vec(const std::vector<T>& vec) {
        return sizeof(T) * vec.size() + sizeof(vec.size());
    }
    
}

#endif /* basic_hpp */
