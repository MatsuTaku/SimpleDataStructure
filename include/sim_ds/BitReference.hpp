//
//  BitReference.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/01/11.
//

#ifndef BitReference_hpp
#define BitReference_hpp

#include <algorithm>

namespace sim_ds {


template <class _Bv, bool IsConst, typename _Bv::word_type = 0> class BitIterator;
template <class _Bv> class BitConstReference;


template <class _Bv>
class BitReference {
    using word_type = typename _Bv::word_type;
    using word_pointer = typename _Bv::word_pointer;
    
    word_pointer seg_;
    word_type mask_;
    
    friend typename _Bv::_self;
    
    friend class BitConstReference<_Bv>;
    friend class BitIterator<_Bv, false>;
    
public:
    operator bool() const {
        return static_cast<bool>(*seg_ & mask_);
    }
    
    BitReference& operator=(bool x) {
        if (x)
            *seg_ |= mask_;
        else
            *seg_ &= ~mask_;
        return *this;
    }
    
    BitReference& operator=(const BitReference& x) {
        return operator=(static_cast<bool>(x));
    }
    
    BitIterator<_Bv, false> operator&() const {
        return BitIterator<_Bv, false>(seg_, static_cast<size_t>(std::__ctz(mask_)));
    }
    
private:
    BitReference(word_pointer seg, word_type mask) : seg_(seg), mask_(mask) {}
    
};


template <class _Bv>
class BitConstReference {
    using word_type = typename _Bv::word_type;
    using word_pointer = typename _Bv::const_word_pointer;
    
    word_pointer seg_;
    word_type mask_;
    
    friend typename _Bv::_self;
    friend class BitIterator<_Bv, true>;
    
public:
    BitConstReference(const BitReference<_Bv>& x) : seg_(x.seg_), mask_(x.mask_) {}
    
    operator bool() const {
        return static_cast<bool>(*seg_ & mask_);
    }
    
    BitIterator<_Bv, true> operator&() const {
        return BitIterator<_Bv, true>(seg_, static_cast<size_t>(__ctz(mask_)));
    }
    
private:
    BitConstReference(word_pointer seg, word_type mask) : seg_(seg), mask_(mask) {}
    
};


template <class _Bv, bool _IsConst,
          typename _Bv::word_type>
class BitIterator {
public:
    using value_type = typename _Bv::value_type;
    using difference_type = typename _Bv::difference_type;
    using reference = std::conditional_t<_IsConst, typename _Bv::const_reference, typename _Bv::reference>;
    using pointer = std::conditional_t<_IsConst, typename _Bv::const_pointer, typename _Bv::pointer>;
    using iterator_category = std::random_access_iterator_tag;
private:
    using word_type = typename _Bv::word_type;
    using word_pointer = std::conditional_t<_IsConst, typename _Bv::const_word_pointer, typename _Bv::word_pointer>;
    
    static constexpr size_t kBitsPerWord = _Bv::kBitsPerWord;
    
    word_pointer seg_;
    size_t ctz_;
    
public:
    BitIterator() : seg_(nullptr), ctz_(0) {}
    
    BitIterator(const BitIterator<_Bv, false>& x) : seg_(x.seg_), ctz_(x.ctz_) {}
    BitIterator& operator=(const BitIterator<_Bv, false>& x) {
        seg_ = x.seg_;
        ctz_ = x.ctz_;
        return *this;
    }
    
    reference operator*() const {
        return reference(seg_, word_type(1) << ctz_);
    }
    
    BitIterator& operator++() {
        if (ctz_ == kBitsPerWord - 1) {
            ++seg_;
            ctz_ = 0;
        } else {
            ++ctz_;
        }
        return *this;
    }
    
    BitIterator operator++(int) {
        BitIterator itr = *this;
        ++this;
        return itr;
    }
    
    BitIterator& operator--() {
        if (ctz_ == 0) {
            --seg_;
            ctz_ = kBitsPerWord - 1;
        } else {
            --ctz_;
        }
        return *this;
    }
    
    BitIterator operator--(int) {
        BitIterator itr = *this;
        --this;
        return itr;
    }
    
    BitIterator& operator+=(difference_type difference) {
        if (difference >= 0) {
            seg_ += (ctz_ + difference) / kBitsPerWord;
        } else {
            seg_ += static_cast<difference_type>(-difference - kBitsPerWord + ctz_ + 1) / static_cast<difference_type>(kBitsPerWord);
        }
        difference &= kBitsPerWord - 1;
        ctz_ = static_cast<size_t>((difference + ctz_) % kBitsPerWord);
        return *this;
    }
    
    BitIterator& operator-=(difference_type difference) {
        return (*this) += -difference;
    }
    
    BitIterator operator+(size_t difference) const {
        BitIterator itr = *this;
        itr += difference;
        return itr;
    }
    
    BitIterator operator-(size_t difference) const {
        BitIterator itr = *this;
        itr -= difference;
        return itr;
    }
    
    friend BitIterator operator+(size_t difference, const BitIterator& x) {return x + difference;}
    
    friend difference_type operator-(const BitIterator& x, const BitIterator& y) {return (x.seg_ - y.seg_) * kBitsPerWord + x.ctz_ - y.ctz_;}
    
    reference operator[](difference_type difference) const {return *(*this + difference);}
    
    friend bool operator==(const BitIterator& x, const BitIterator& y) {return x.seg_ == y.seg_ and x.ctz_ == y.ctz_;}
    
    friend bool operator!=(const BitIterator& x, const BitIterator& y) {return !(x == y);}
    
    friend bool operator<(const BitIterator& x, const BitIterator& y) {return x.seg_ < y.seg_ or (x.seg_ == y.seg_ and x.ctz_ < y.ctz_);}
    
    friend bool operator>(const BitIterator& x, const BitIterator& y) {return y < x;}
    
    friend bool operator<=(const BitIterator& x, const BitIterator& y) {return !(x > y);}
    
    friend bool operator>=(const BitIterator& x, const BitIterator& y) {return !(x < y);}
    
private:
    BitIterator(word_pointer pointer, size_t ctz) : seg_(pointer), ctz_(ctz) {}
    
    friend typename _Bv::_self;
    
};


}

#endif /* BitReference_hpp */
