//
//  basic.hpp
//  build
//
//  Created by 松本拓真 on 2018/04/25.
//

#ifndef basic_hpp
#define basic_hpp

#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <queue>
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <limits>
#include <chrono>

#ifdef _MSC_VER
#include <iso646.h>
#endif

namespace sim_ds {

using id_type = size_t;

class Stopwatch {
    using hrc = std::chrono::high_resolution_clock;
    hrc::time_point start_;
public:
    Stopwatch() : start_(hrc::now()) {}
    auto time_process() const {
        return hrc::now() - start_;
    }
    double get_sec() const {
        return std::chrono::duration<double>(time_process()).count();
    }
    double get_milli_sec() const {
        return std::chrono::duration<double, std::milli>(time_process()).count();
    }
    double get_micro_sec() const {
        return std::chrono::duration<double, std::micro>(time_process()).count();
    }
};

// MARK: Read

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

// MARK: Write

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
inline size_t size_vec(const std::vector<T>& vec) {
    return sizeof(T) * vec.size() + sizeof(vec.size());
}

} // namespace sim_ds

#endif /* basic_hpp */
