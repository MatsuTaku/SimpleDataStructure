//
//  BitsReference.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/01/11.
//

#ifndef BitsReference_hpp
#define BitsReference_hpp

#include "bit_util.hpp"

namespace sim_ds {


template <class Sequence, bool IsConst> class BitsIterator;
template <class Sequence> class BitsConstReference;


template<class Sequence>
class BitsReference {
    using storage_type = typename Sequence::storage_type;
    using storage_pointer = typename Sequence::storage_pointer;
    using value_type = typename Sequence::value_type;
    
    storage_pointer seg_;
    size_t offset_;
    
    const size_t bits_per_element_;
    const storage_type mask_;
    
    friend typename Sequence::Self;
    friend class BitsConstReference<Sequence>;
    
    static constexpr size_t kBitsPerWord = Sequence::kBitsPerWord;
    
public:
    operator value_type() const {
        if (bits_per_element_ + offset_ <= kBitsPerWord) {
            return (*seg_ >> offset_) & mask_;
        } else {
            return ((*seg_ >> offset_) | (*(seg_+1) << (kBitsPerWord - offset_))) & mask_;
        }
    }
    
    BitsReference& operator=(value_type value) {
        *seg_ = (*seg_ & ~(mask_ << offset_)) | (value << offset_);
        if (bits_per_element_ + offset_ > kBitsPerWord) {
            auto roffset = kBitsPerWord - offset_;
            *(seg_+1) = (*(seg_+1) & ~(mask_ >> roffset)) | value >> roffset;
        }
        return *this;
    }
    
    BitsReference& operator=(const BitsReference& x) {
        return static_cast<value_type>(x);
    }
    
    BitsIterator<Sequence, false> operator&() const {
        return BitsIterator<Sequence, false>(seg_, offset_, bits_per_element_);
    }
    
private:
    BitsReference(storage_pointer seg,
                  size_t offset,
                  size_t bits_per_element,
                  storage_type bit_mask
                  ) : seg_(seg), offset_(offset), bits_per_element_(bits_per_element), mask_(bit_mask) {}
    
};


template<class Sequence>
class BitsConstReference {
    using storage_type = typename Sequence::storage_type;
    using storage_pointer = typename Sequence::const_storage_pointer;
    using value_type = typename Sequence::value_type;
    
    storage_pointer seg_;
    size_t offset_;
    
    const size_t bits_per_element_;
    const storage_type mask_;
    
    friend typename Sequence::Self;
    
    static constexpr size_t kBitsPerWord = Sequence::kBitsPerWord;
    
public:
    BitsConstReference(const BitsReference<Sequence>& x) : seg_(x.seg_), offset_(x.offset_), bits_per_element_(x.bits_per_element_), mask_(x.mask_) {}
    
    operator value_type() const {
        if (bits_per_element_ + offset_ <= kBitsPerWord) {
            return (*seg_ >> offset_) & mask_;
        } else {
            return ((*seg_ >> offset_) | (*(seg_+1) << (kBitsPerWord - offset_))) & mask_;
        }
    }
    
    BitsIterator<Sequence, true> operator&() const {
        return BitsIterator<Sequence, true>(seg_, offset_, bits_per_element_);
    }
    
private:
    BitsConstReference(storage_pointer seg,
                       size_t offset,
                       size_t bits_per_element,
                       storage_type bit_mask
                       ) : seg_(seg), offset_(offset), bits_per_element_(bits_per_element), mask_(bit_mask) {}
    
};


template <class Sequence, bool IsConst>
class BitsIterator {
public:
    using value_type = typename Sequence::value_type;
    using difference_type = typename Sequence::difference_type;
    using reference = std::conditional_t<IsConst, BitsConstReference<Sequence>, BitsReference<Sequence>>;
    
    static constexpr size_t kBitsPerWord = Sequence::kBitsPerWord;
    
private:
    using storage_type = typename Sequence::storage_type;
    using storage_pointer = std::conditional_t<IsConst, typename Sequence::const_storage_pointer, typename Sequence::storage_pointer>;
    
    storage_pointer seg_;
    size_t start_;
    
    const size_t bits_per_entity_;
    
public:
    BitsIterator() : seg_(nullptr), start_(0), bits_per_entity_(64) {}
    
    BitsIterator(const BitsIterator<Sequence, false>& x) : seg_(x.seg_), start_(x.offset_), bits_per_entity_(x.bits_per_entity_) {}
    
    value_type operator*() const {
        return reference(seg_, start_, bits_per_entity_, bit_util::WidthMask(bits_per_entity_));
    }
    
    BitsIterator& operator++() {
        if (start_ == kBitsPerWord - 1) {
            ++seg_;
            start_ = 0;
        } else {
            ++start_;
        }
    }
    
    BitsIterator operator++(int) {
        BitsIterator itr = *this;
        ++(*this);
        return itr;
    }
    
    BitsIterator& operator--() {
        if (start_ == 0) {
            --seg_;
            start_ = kBitsPerWord - 1;
        } else {
            --start_;
        }
    }
    
    BitsIterator operator--(int) {
        BitsIterator itr = *this;
        --(*this);
        return itr;
    }
    
    BitsIterator& operator+=(difference_type difference) {
        if (difference >= 0) {
            seg_ += (start_ + difference) / kBitsPerWord;
        } else {
            seg_ += static_cast<difference_type>(-difference - kBitsPerWord + start_ + 1) / static_cast<difference_type>(kBitsPerWord);
        }
        difference &= kBitsPerWord - 1;
        start_ = static_cast<size_t>((difference + start_) % kBitsPerWord);
        return *this;
    }
    
    BitsIterator& operator-=(difference_type difference) {
        return *this += -difference;
    }
    
    BitsIterator& operator+(difference_type difference) {
        BitsIterator itr = *this;
        itr += difference;
        return itr;
    }
    
    BitsIterator& operator-(difference_type difference) {
        BitsIterator itr = *this;
        itr -= difference;
        return itr;
    }
    
    friend BitsIterator& operator+(difference_type difference, const BitsIterator& x) {return x + difference;}
    
    friend difference_type operator-(const BitsIterator& x, const BitsIterator& y) {return static_cast<difference_type>(x.seg_ - y.seg_) * kBitsPerWord + static_cast<difference_type>(x.offset_ - y.offset_);}
    
    value_type operator[](difference_type pos) const {return *(*this + pos);}
    
    friend bool operator==(const BitsIterator& x, const BitsIterator& y) {return x.seg_ == y.seg_ and x.offset_ == y.offset_;}
    
    friend bool operator!=(const BitsIterator& x, const BitsIterator& y) {return !(x == y);}
    
    friend bool operator<(const BitsIterator& x, const BitsIterator& y) {return x.seg_ < y.seg_ or (x.seg_ == y.seg_ and x.offset_ < y.offset_);}
    
    friend bool operator>(const BitsIterator& x, const BitsIterator& y) {return y < x;}
    
    friend bool operator<=(const BitsIterator& x, const BitsIterator& y) {return !(x > y);}
    
    friend bool operator>=(const BitsIterator& x, const BitsIterator& y) {return !(x < y);}
    
private:
    BitsIterator(storage_pointer seg, size_t start, size_t bits_per_entity) : seg_(seg), start_(start), bits_per_entity_(bits_per_entity) {}
    
    friend typename Sequence::Self;
    
};


}

#endif /* BitsReference_hpp */
