#ifndef initializable_array_hpp
#define initializable_array_hpp

#include <array>
#include <vector>
#include <bitset>
#include <numeric>
#include <type_traits>

namespace sim_ds {

template <typename T>
class InitializableArrayFolklore {
public:
    using value_type = T;
private:
    T init_v_;
    std::vector<std::pair<size_t, T>> fv_;
    std::vector<size_t> t_;
    
public:
    explicit InitializableArrayFolklore(size_t size = 0, T val = T()) : init_v_(val), fv_(size), t_() {};
    
    template <typename It>
    explicit InitializableArrayFolklore(It begin, It end) : InitializableArrayFolklore(end - begin) {
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



// Katoh T, Goto K. In-Place Initializable Arrays. http://arxiv.org/abs/1709.08900
template<typename T, size_t Size,
    bool IsSpecialCase>
class _InitializableArray;


template <typename T, size_t Size>
using InitializableArray = _InitializableArray<T, Size,
    (1ull<<(std::numeric_limits<T>::digits)) >= (Size+(Size%2))>;


// In-Place Initializable Arrays for a Special Case
template <typename T, size_t Size>
class _InitializableArray<T, Size, true> {
public:
    static constexpr size_t kActualSize = Size + (Size % 2);
    static constexpr size_t kNoneBlock = kActualSize/2;
    
    using value_type = T;
private:
    T init_v_;
    size_t b_;
    std::array<T, kActualSize> a_;

public:
    explicit _InitializableArray(T init_v = T()) : init_v_(init_v), b_(0) {}
    
    T get(size_t i) const {
        auto b = i/2;
        auto k = _chained_to(b);
        if (i < 2*b_) { // Unwritten Chained Area
            return k != kNoneBlock ? init_v_ : a_[i];
        } else { // Written Chained Area
            if (k != kNoneBlock) {
                return i%2==0 ? a_[a_[i]+1] : a_[i];
            } else {
                return init_v_;
            }
        }
    }
    
    void set(const size_t i, T val) {
        const auto b = i/2;
        auto k = _chained_to(b);
        auto write_uca = [&]() {
            a_[i] = val;
            _break_chain(b);
        };
        if (b < b_) { // Unwritten Chained Area
            if (k == kNoneBlock) {
                write_uca();
            } else {
                auto j = _extend();
                if (b == j) {
                    write_uca();
                } else {
                    a_[2*j] = a_[2*b];
                    a_[2*j+1] = a_[2*b+1];
                    _make_chain(j, k);
                    _init_block(b);
                    write_uca();
                }
            }
        } else { // Written Chained Area
            auto write_wca = [&]() {
                if (i%2==0)
                    a_[2*k+1] = val;
                else
                    a_[i] = val;
            };
            if (k != kNoneBlock) {
                write_wca();
            } else {
                k = _extend();
                if (b == k) {
                    write_uca();
                } else {
                    _init_block(b);
                    _make_chain(k, b);
                    write_wca();
                }
            }
        }
    }
    
    void init(T init_v = T()) {
        b_ = 0;
        init_v_ = init_v;
    }
    
    void fill(T init_v = T()) {
        init(init_v);
    }
    
    size_t size() const {return Size;}
    
    void print_for_debug() const {
        std::cout<<"E: ";
        for (int i = 0; i < kActualSize; i++)
            std::cout << get(i) << " ";
        std::cout << "\n";
        std::cout << "b: " << b_ << "\n";
        std::cout<<"A: ";
        for (int i = 0; i < kActualSize; i++)
            std::cout << a_[i] << " ";
        std::cout << std::endl;
    }

private:
    size_t _to(size_t i) const {
        size_t res = 0;
        memcpy(&res, &a_[i], sizeof(T));
        return res;
    }
    
    void _set_to(size_t i, size_t b) {
        memcpy(&a_[i], &b, sizeof(T));
    }
    
    size_t _chained_to(size_t b) const {
        size_t bki = _to(2 * b);
        if (bki < kActualSize and bki % 2 == 0 and _to(bki) == 2 * b and
            ((b < b_ and b_ <= bki / 2) or (bki / 2 < b_ and b_ <= b)))
            return bki/2;
        else
            return kNoneBlock;
    }
    
    void _make_chain(size_t bi, size_t bj) {
        assert(bi < b_ and b_ <= bj);
        _set_to(2*bi, 2*bj);
        _set_to(2*bj, 2*bi);
    }
    
    void _break_chain(size_t b) {
        auto bk = _chained_to(b);
        if (bk != kNoneBlock){
            _set_to(2*bk, 2*bk);
        }
    }
    
    void _init_block(size_t b) {
        a_[2*b] = a_[2*b+1] = init_v_;
    }
    
    size_t _extend() {
        auto k = _chained_to(b_);
        b_++;
        if (k == kNoneBlock) {
            k = b_-1;
        } else {
            a_[2*(b_-1)] = a_[2*k+1];
//            a_[2*(b_-1)+1] = a_[2*(b_-1)+1]; // obviously
            _break_chain(b_-1);
        }
        _init_block(k);
        _break_chain(k);
        return k;
    }
    
    size_t _shrink() {
        assert(b_ > 0);
        auto k = _chained_to(b_-1);
        b_--;
        if (k == kNoneBlock) {
            k = b_;
        } else {
            a_[2*k] = a_[2*b_-1];
        }
        return k;
    }

public:
    class const_reference {
    private:
        const _InitializableArray &arr_;
        size_t i_;
        friend class _InitializableArray;
        const_reference(const _InitializableArray &arr, size_t i) : arr_(arr), i_(i) {}
    public:
        operator T() const { return arr_.get(i_); }
    };
    class reference {
    private:
        _InitializableArray &arr_;
        size_t i_;
        friend class _InitializableArray;
        reference(_InitializableArray &arr, size_t i) : arr_(arr), i_(i) {}
    public:
        operator T() const { return arr_.get(i_); }
        reference& operator=(T val) {
            arr_.set(i_, val);
            return *this;
        }
    };
    
    const_reference operator[](size_t i) const { return const_reference(*this, i); }
    
    reference operator[](size_t i) { return reference(*this, i); }
    
private:
    template <bool IsConst>
    class _iterator : public std::iterator<std::random_access_iterator_tag, T> {
        using _base = std::iterator<std::random_access_iterator_tag, T>;
    public:
        using ptrdiff_t = typename _base::ptrdiff_t;
        using reference_type = std::conditional_t<IsConst, const_reference, reference>;
    private:
        const _InitializableArray &arr_;
        size_t i_;
        friend class _InitializableArray;
        _iterator(const _InitializableArray &arr, size_t i) : arr_(arr), i_(i) {}
    public:
        reference_type operator*() const { return reference_type(arr_, i_); }
        _iterator& operator++() { ++i_; return *this; }
        _iterator operator++(int) { auto c = *this; ++(*this); return c; }
        _iterator& operator--() { --i_; return *this; }
        _iterator operator--(int) { auto c = *this; --(*this); return c; }
        _iterator operator+(ptrdiff_t x) const { return _iterator(arr_, i_ + x); }
        _iterator& operator+=(ptrdiff_t x) { return *this = *this + x; }
        _iterator operator-(ptrdiff_t x) const { return *this + -x; }
        _iterator& operator-=(ptrdiff_t x) { return *this = *this - x; }
    };
public:
    using const_iterator = _iterator<true>;
    using iterator = _iterator<false>;
    
    const_iterator begin() const { return _iterator(*this, 0); }
    const_iterator end() const { return _iterator(*this, Size); }
    iterator begin() { return _iterator(*this, 0); }
    iterator end() { return _iterator(*this, Size); }
    const_iterator cbegin() const { return _iterator(*this, 0); }
    const_iterator cend() const { return _iterator(*this, Size); }
    
};


// In-Place Initializable Arrays for a General Case
template <typename T, size_t Size>
class _InitializableArray<T, Size, false> {};

}

#endif //initializable_array_hpp
