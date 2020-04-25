#ifndef initializable_array_hpp
#define initializable_array_hpp

#include <vector>

namespace sim_ds {

template <typename T>
class InitializableArray {
    using _base = std::vector<T>;
public:
    using value_type = T;
private:
    T init_v_;
    std::vector<std::pair<size_t, T>> fv_;
    std::vector<size_t> t_;
    
public:
    explicit InitializableArray(size_t size = 0, T val = T()) : init_v_(val), fv_(size), t_() {};
    
    template <typename It>
    explicit InitializableArray(It begin, It end) : InitializableArray(end-begin) {
        static_assert(std::is_convertible_v<typename std::iterator_traits<It>::value_type, T>);
        for (auto it = begin; it != end; ++it) {
            set(it-begin, *it);
        }
    }
    
    T get(size_t index) const {
        return _is_chained(index) ? fv_[index].second : init_v_;
    }
    
    void set(size_t index, T val) {
        if (not _is_chained(index)) {
            fv_[index].first = t_.size();
            t_.push_back(index);
        }
        fv_[index].second = val;
    }
    
    // Initialize all values in O(1) time operation
    void init(T init_v=T()) {
        init_v_ = init_v;
        t_.clear();
    }
    
    void fill(T init_v=T()) {
        init(init_v);
    }
    
    size_t size() const {return fv_.size();}
    
private:
    bool _is_chained(size_t index) const {
        auto f = fv_[index].first;
        return f < t_.size() and t_[f] == index;
    }
    
};

}

#endif //initializable_array_hpp
