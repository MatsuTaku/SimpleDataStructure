
//  StringTransFSA.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/14.
//

#ifndef StringTransFSA_hpp
#define StringTransFSA_hpp

#include "NextCheck.hpp"
#include "BitVector.hpp"
#include "StringArray.hpp"

namespace sim_ds {
    
    class PlainFSA;
    
    template <class C_CODES, class STR_ARR>
    class StringTransFSA {
    public:
        using checkCodesType = C_CODES;
        using NCType = NextCheck<false, true, DACs, C_CODES>;
        static constexpr bool useLink = true;
        using SAType = STR_ARR;
    public:
        static std::string name() {
            std::string link = (useLink ? "ST" : "STC");
            return link + "FSA";
        }
        
//        static StringTransFSA build(const PlainFSA& fsa);
        
        StringTransFSA() = default;
        
        StringTransFSA(std::istream &is) {
            read(is);
        }
        
        ~StringTransFSA() = default;
        
        bool isMember(const std::string& str) const { // TODO: -
            size_t trans = 0;
            for (size_t pos = 0, size = str.size(); pos < size;) {
                uint8_t c = str[pos];
                trans = target(trans) ^ c;
                if (!isStringTrans(trans)) {
                    auto checkE = check(trans);
                    if (checkE != c)
                        return false;
                    pos++;
                } else {
                    auto sid = stringId(trans);
//                    strings_.showLabels(sid - 32, sid + 32);
                    if (!strings_.isMatch(&pos, str, sid)) {
                        strings_.showLabels(sid - 32, sid + 32);
                        return false;
                    }
                }
            }
            return isFinal(trans);
        }
        
        size_t target(size_t index) const {
            return next(index) ^ index;
        }
        
        size_t next(size_t index) const {
            return nc_.next(index) >> 1;
        }
        
        uint8_t check(size_t index) const {
            return nc_.check(index);
        }
        
        size_t stringId(size_t index) const {
            return nc_.stringId(index);
        }
        
        bool isFinal(size_t index) const {
            return (nc_.next(index) & 1) != 0;
        }
        
        bool isStringTrans(size_t index) const {
            if (useLink)
                return nc_.getBitInFlow(index);
            else
                return is_string_bits_[index];
        }
        
        // MARK: - build
        
        void setCheck(size_t index, uint8_t check) {
            nc_.setCheck(index, check);
        }
        
        void setNextAndIsFinal(size_t index, size_t next, bool isFinal) {
            size_t value = next << 1 | isFinal;
            nc_.setNext(index, value);
        }
        
        void setIsStringTrans(size_t index, bool isString) {
            if (useLink) {
                nc_.setBitInFlow(index, isString);
            } else {
                is_string_bits_.set(index, isString);
            }
        }
        
        void setStringIndex(size_t index, size_t strIndex) {
            nc_.setStringIndex(index, strIndex);
        }
        
        void setStringArray(const STR_ARR& sArr) {
            strings_ = sArr;
        }
        
        void buildBitArray() {
            nc_.buildBitArray();
        }
        
        // MARK: - Protocol setting
        
        void setNumElement(size_t num) {
            nc_.setNumElement(num, true);
        }
        
        void setNumStrings(size_t num) {
            nc_.setNumStrings(num);
        }
        
        void setNumTrans(size_t num) {
            num_trans_ = num;
        }
        
        // MARK: - ByteData method
        
        size_t sizeInBytes() const {
            auto size = sizeof(num_trans_);
            size += nc_.sizeInBytes();
            if (!useLink)
                size += is_string_bits_.sizeInBytes();
            size += strings_.sizeInBytes();
            return size;
        }
        
        void write(std::ostream& os) const {
            write_val(num_trans_, os);
            nc_.write(os);
            if (!useLink)
                is_string_bits_.write(os);
            strings_.write(os);
        }
        
        void writeCheckId(std::ostream &os) const {
            std::vector<size_t> c_vec_src(nc_.numElements());
            for (auto i = 0; i < nc_.numElements(); i++)
                c_vec_src[i] = nc_.check(i);
            Vector c_vec(c_vec_src);
            c_vec.write(os);
        }
        
        void read(std::istream& is) {
            num_trans_ = read_val<size_t>(is);
            nc_.read(is);
            if (!useLink)
                is_string_bits_.read(is);
            strings_.read(is);
        }
        
        void show_stat(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << name() << " ---" << endl;
            os << "#trans: " << num_trans_ << endl;
            os << "#elems: " << nc_.numElements() << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size is final:   " << is_final_bits_.sizeInBytes() << endl;
            os << "size is string:  " << is_string_bits_.sizeInBytes() << endl;
            os << "size strings:    " << strings_.sizeInBytes() << endl;
            nc_.showStatus(os);
        }
        
        // MARK: - Copy guard
        
        StringTransFSA (const StringTransFSA&) = delete;
        StringTransFSA& operator=(const StringTransFSA&) = delete;
        
        StringTransFSA(StringTransFSA&& rhs) noexcept = default;
        StringTransFSA& operator=(StringTransFSA&& rhs) noexcept = default;
        
    private:
        size_t num_trans_ = 0;
        NCType nc_;
        BitVector is_final_bits_;
        BitVector is_string_bits_;
        SAType strings_;
        
        friend class ArrayFSATailBuilder;
    };
    
    using STFSA = StringTransFSA<DACs, StringArray<false>>;
//    using STCFSA = StringTransFSA<SACs2, StringArray<false>>;
//    using STCFSAB = StringTransFSA<SACs2, StringArray<true>>;
    
}

#endif /* StringTransFSA_hpp */
