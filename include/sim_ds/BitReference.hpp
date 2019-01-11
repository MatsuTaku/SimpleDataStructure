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


template <class BitSequence, bool IsConst> class BitIterator;
template <class BitSequence> class BitConstReference;


template <class BitSequence>
class BitReference {
    using storage_type = typename BitSequence::storage_type;
    using storage_pointer = typename BitSequence::storage_pointer;
    
    storage_pointer seg_;
    storage_type mask_;
    
    friend typename BitSequence::Self;
    
    friend class BitConstReference<BitSequence>;
    friend class BitIterator<BitSequence, false>;
    
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
    
    BitIterator<BitSequence, false> operator&() const {
        return BitIterator<BitSequence, false>(seg_, static_cast<size_t>(std::__ctz(mask_)));
    }
    
private:
    BitReference(storage_pointer seg, storage_type mask) : seg_(seg), mask_(mask) {}
    
};


template <class BitSequence>
class BitConstReference {
    using storage_type = typename BitSequence::storage_type;
    using storage_pointer = typename BitSequence::const_storage_pointer;
    
    storage_pointer seg_;
    storage_type mask_;
    
    friend typename BitSequence::Self;
    friend class BitIterator<BitSequence, true>;
    
public:
    BitConstReference(const BitReference<BitSequence>& x) : seg_(x.seg_), mask_(x.mask_) {}
    
    operator bool() const {
        return static_cast<bool>(*seg_ & mask_);
    }
    
    BitIterator<BitSequence, true> operator&() const {
        return BitIterator<BitSequence, true>(seg_, static_cast<size_t>(__ctz(mask_)));
    }
    
private:
    BitConstReference(storage_pointer seg, storage_type mask) : seg_(seg), mask_(mask) {}
    
};


template <class BitSequence, bool IsConst>
class BitIterator {
    using value_type = bool;
    using storage_type = typename BitSequence::storage_type;
    using storage_pointer = std::conditional_t<IsConst, typename BitSequence::const_storage_pointer, typename BitSequence::storage_pointer>;
    
    using Reference = std::conditional_t<IsConst, typename BitSequence::ConstReference, typename BitSequence::Reference>;
    
    static constexpr size_t kBitsPerWord = BitSequence::kBitsPerWord;
    
    storage_pointer seg_;
    size_t ctz_;
    
public:
    BitIterator() : seg_(nullptr), ctz_(0) {}
    
    BitIterator(const BitIterator<BitSequence, IsConst>& x) : seg_(x.pointer_), ctz_(x.ctz_) {}
    
    Reference operator*() const {
        return Reference(seg_, storage_type(1) << ctz_);
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
            ++seg_;
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
    
    BitIterator& operator+=(size_t offset) {
        if (offset >= 0) {
            seg_ += (ctz_ + offset) / kBitsPerWord;
        } else {
            seg_ += static_cast<long long>(-offset - kBitsPerWord + ctz_ + 1) / static_cast<long long>(kBitsPerWord);
        }
        offset &= kBitsPerWord - 1;
        ctz_ = static_cast<size_t>((offset + ctz_) % kBitsPerWord);
        return *this;
    }
    
    BitIterator& operator-=(size_t offset) {
        return (*this) += -offset;
        
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
    
    friend long long operator-(const BitIterator& x, const BitIterator& y) {return (x.pointer_ - y.pointer_) * kBitsPerWord + static_cast<long long>(x.ctz_ - y.ctz_);}
    
    Reference operator[](long long difference) const {return *(*this + difference);}
    
    friend bool operator==(const BitIterator& x, const BitIterator& y) {return x.pointer_ == y.pointer_ and x.ctz_ == y.ctz_;}
    
    friend bool operator!=(const BitIterator& x, const BitIterator& y) {return !(x == y);}
    
    friend bool operator<(const BitIterator& x, const BitIterator& y) {return x.pointer_ < y.pointer_ or (x.pointer_ == y.pointer_ and x.ctz_ < y.ctz_);}
    
    friend bool operator>(const BitIterator& x, const BitIterator& y) {return y < x;}
    
    friend bool operator<=(const BitIterator& x, const BitIterator& y) {return !(x > y);}
    
    friend bool operator>=(const BitIterator& x, const BitIterator& y) {return !(x < y);}
    
private:
    BitIterator(storage_pointer pointer, size_t ctz) : seg_(pointer), ctz_(ctz) {}
    
    friend typename BitSequence::Self;
    
};


}

#endif /* BitReference_hpp */
