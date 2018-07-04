//
//  NextCheck.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/01/13.
//

#ifndef NextCheck_hpp
#define NextCheck_hpp

#include "basic.hpp"
#include "FitValuesArray.hpp"
#include "DACs.hpp"
#include "SACs.hpp"

#include "Calc.hpp"
#include "basic.hpp"

namespace sim_ds {
    
    template <
    bool N,
    bool C,
    class N_CODES = DACs,
    class C_CODES = SACs2
    >
    class NextCheck {
    public:
        static constexpr bool useNextCodes = N;
        static constexpr bool useCheckCodes = C;
        using nCodes = N_CODES;
        using cCodes = C_CODES;
    public:
        NextCheck() = default;
        ~NextCheck() = default;
        
        size_t next(size_t index) const {
            auto next = bytes_.getValue<size_t>(index, 0);
            if (!N) return next;
            return next | nextFlow[index] << 8;
        }
        
        uint8_t check(size_t index) const {
            return bytes_.getValue<uint8_t>(index, 1);
        }
        
        size_t stringId(size_t index) const {
            assert(C);
            auto id = bytes_.getValue<uint8_t>(index, 1);
            return id | checkFlow[index] << 8;
        }
        
        size_t numElements() const {
            return bytes_.numElements();
        }
        
        bool getBitInFlow(size_t index) const {
            if (!C) abort();
            return checkFlow.getBitInFirstUnit(index);
        }
        
        // MARK: - build
        
        void setNext(size_t index, size_t next) {
            bytes_.setValue(index, 0, next);
            if (!N) return;
            nextFlow.setValue(index, next >> 8);
        }
        
        void setCheck(size_t index, uint8_t check) {
            bytes_.setValue(index, 1, check);
        }
        
        void setStringIndex(size_t index, size_t strIndex) {
            assert(C);
            setCheck(index, strIndex & 0xff);
            checkFlow.setValue(index, strIndex >> 8);
        }
        
        void setBitInFlow(size_t index, bool bit) {
            assert(C);
            checkFlow.setBitInFirstUnit(index, bit);
        }
        
        // MARK: - Protocol settings
        
        // No.1
        void setNumElement(size_t num, bool bitInto) {
            auto nextSize = Calc::sizeFitInBytes(bitInto ? num << 1 : num);
            std::vector<size_t> sizes = { N ? 1 : nextSize, 1 };
            bytes_.setValueSizes(sizes);
            bytes_.resize(num);
        }
        
//        // No.2 if use dac check
//        void setNumStrings(size_t num) {
//            assert(C);
//            auto maxSize = Calc::sizeFitInBytes(num - 1);
//            auto cCodesName = typeid(cCodes).name();
//            if (cCodesName == typeid(DACs<true>).name() ||
//                cCodesName == typeid(DACs<false>).name())
//                checkFlow.setUnitSize(std::max(maxSize - 1, size_t(1)));
//        }
        
        // Finaly. If use dac
        void buildBitArray() {
            if (N) nextFlow.build();
            if (C) checkFlow.build();
        }
        
        // MARK: - ByteData methods
        
        size_t sizeInBytes() const {
            auto size = bytes_.sizeInBytes();
            size += nextFlow.sizeInBytes();
            size += checkFlow.sizeInBytes();
            return size;
        }
        void write(std::ostream& os) const {
            bytes_.write(os);
            nextFlow.write(os);
            checkFlow.write(os);
        }
        void read(std::istream& is) {
            bytes_.read(is);
//            nextFlow.read(is);
//            checkFlow.read(is);
        }
        
        void showStatus(std::ostream& os) const {
            using std::endl;
            os << "--- Stat of " << "NextCheck " << nCodes::name() << "|" << cCodes::name() << " ---" << endl;
            os << "size:   " << sizeInBytes() << endl;
            os << "size bytes:   " << bytes_.sizeInBytes() << endl;
            os << "size next flow:   " << nextFlow.sizeInBytes() << endl;
            os << "size check flow:   " << checkFlow.sizeInBytes() << endl;
            nextFlow.showStats(os);
            checkFlow.showStats(os);
            showSizeMap(os);
        }
        
        void showSizeMap(std::ostream &os) const {
            auto numElem = numElements();
            std::vector<size_t> nexts(numElem);
            for (auto i = 0; i < numElem; i++)
                nexts[i] = next(i) >> (!N ? 1 : 0);
            
            auto showList = [&](const std::vector<size_t> &list) {
                using std::endl;
                os << "-- " << "Next Map" << " --" << endl;
                for (auto c : list)
                    os << c << "\t" << endl;
                os << "/ " << numElem << endl;
            };
            auto counts = Calc::separateCountsInSizeOf(nexts);
            showList(counts);
            auto xorCounts = Calc::separateCountsInXorSizeOf(nexts);
            showList(xorCounts);
        }
        
        // MARK: - Copy guard
        
        NextCheck (const NextCheck&) = delete;
        NextCheck& operator =(const NextCheck&) = delete;
        
        NextCheck (NextCheck&&) noexcept = default;
        NextCheck& operator =(NextCheck&&) noexcept = default;
        
    private:
        FitValuesArray bytes_;
        nCodes nextFlow;
        cCodes checkFlow;
        
    };
    
}

#endif /* NextCheck_hpp */
