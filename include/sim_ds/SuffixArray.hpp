 //
//  SuffixArray.hpp
//  build
//
//  Created by 松本拓真 on 2018/04/25.
//

#ifndef SuffixArray_hpp
#define SuffixArray_hpp

#include "basic.hpp"
#include "FitVector.hpp"
#include "calc.hpp"

#include <limits>

namespace sim_ds {
    
using std::vector;
using std::string;
using std::string_view;

class SuffixArray {
public:
    static constexpr size_t kInf = std::numeric_limits<size_t>::max();
    
private:
    string str_;
    FitVector s_arr_;
    FitVector lcp_arr_;
    
public:
    SuffixArray() = default;
    
    SuffixArray(const string &str) {
        BuildSA_(str);
        BuildLCP_();
    }
    
    uint8_t char_at(size_t index) const {
        return str_[index];
    }
    
    string_view suffix(size_t index, size_t size) const {
        return string_view(str_).substr(index, size);
    }
    
    string_view suffix(size_t index) const {
        return suffix(index, str_.size() - index);
    }
    
    void WriteSA(std::ostream &os) const {
        s_arr_.Write(os);
    }
    
    void WriteLCP(std::ostream &os) const {
        lcp_arr_.Write(os);
    }
    
    void Read(std::istream &is) {
        str_ = read_string(is);
        s_arr_ = FitVector(is);
        lcp_arr_ = FitVector(is);
    }
    
    void Write(std::ostream &os) const {
        write_string(str_, os);
        s_arr_.Write(os);
        lcp_arr_.Write(os);
    }
    
private:
    /* <0>: suffix array
     * <1>: subStr
     * <2>: count shared - size_t value. ((count(lmss) - 1) - max_val(subStr)). if 0, it is unique.
     */
    using InducedRV = std::tuple<vector<size_t>, vector<size_t>, size_t>;
    
    struct Bucket {
        size_t index;
        size_t count = 0;
        size_t lPos = 0;
        size_t sPos = 0;
        
        void setPos() {
            initLPos();
            initSPos();
        }
        
        void initLPos() {
            lPos = index;
        }
        
        void initSPos() {
            sPos = index + count - 1;
        }
        
        bool full() const {
            return count == 0 || sPos < lPos;
        }
    };
    
    void BuildSA_(const string &str);
    
    void SaisStr_(vector<size_t> &saisS, const string &str) const;
    
    void MakeBuckets_(vector<Bucket> &buckets, const vector<size_t> &str, size_t maxValue) const;
    
    vector<bool> Classified_(const vector<size_t> &str) const;
    
    vector<size_t> FindLMSs_(const vector<bool> &slTypes) const;
    
    bool EqualLMSs_(const vector<size_t> &str, const vector<bool> &slTypes, size_t li, size_t ri) const;
    
    size_t PutBucket_(vector<Bucket> *buckets, vector<size_t> *sArr, bool slType, size_t firstC, size_t id) const;
    
    vector<size_t> Sais_(const vector<size_t> &str, size_t maxValue) const;
    
    InducedRV InducedSort_(size_t maxValue, const vector<size_t> &str, const vector<bool> &slTypes, const vector<size_t> &lmsIds) const;
    
    vector<size_t> Induce_(size_t maxValue, const vector<size_t> &str, const vector<bool> &slTypes, const vector<size_t> &lmsIds, const vector<size_t> *lmssArr = nullptr) const;
    
    void BuildLCP_();
    
    size_t CompareLCP_(size_t li, size_t ri, size_t lcp) const;
    
};

void SuffixArray::BuildSA_(const string &str) {
    str_ = str;
    vector<size_t> strVec;
    SaisStr_(strVec, str);
    vector<size_t> sArr = Sais_(strVec, 0xff);
    // Erase first element. suffix[0]: '\0'
    sArr.erase(sArr.begin());
    s_arr_ = FitVector(sArr);
}


void SuffixArray::SaisStr_(vector<size_t> &saisS, const string &str) const {
    saisS.resize(str.size());
    for (auto i = 0; i < str.size(); i++)
        saisS[i] = size_t(str[i]);
    saisS.push_back(0);
}


void SuffixArray::MakeBuckets_(vector<Bucket> &buckets, const vector<size_t> &str, size_t maxValue) const {
    buckets.resize(maxValue + 1);
    for (auto s : str) {
        buckets[s].count++;
    }
    auto count = 0;
    for (auto &b : buckets) {
        b.index = count;
        b.lPos = b.index;
        b.sPos = b.index + b.count - 1;
        count += b.count;
    }
}

inline vector<bool> SuffixArray::Classified_(const vector<size_t> &str) const {
    vector<bool> slTypes(str.size());
    slTypes[slTypes.size() - 1] = true;
    for (auto i = str.size() - 1; i > 0; i--) {
        slTypes[i - 1] = (str[i - 1] == str[i]) ? slTypes[i] : str[i - 1] < str[i];
    }
    return slTypes;
}

inline vector<size_t> SuffixArray::FindLMSs_(const vector<bool> &slTypes) const {
    vector<size_t> lmss;
    for (auto i = 1; i < slTypes.size(); i++) {
        if (!slTypes[i - 1] && slTypes[i])
            lmss.push_back(i);
    }
    return lmss;
}

inline bool SuffixArray::EqualLMSs_(const vector<size_t> &str, const vector<bool> &slTypes, size_t li, size_t ri) const {
    bool leftL = false;
    bool curType = true;
    do {
        if (str[li] != str[ri])
            return false;
        leftL = !curType;
        curType = slTypes[li];
        li++; ri++;
    } while (!(leftL && curType));
    return true;
}

inline size_t SuffixArray::PutBucket_(vector<Bucket> *buckets, vector<size_t> *sArr, bool slType, size_t firstC, size_t id) const {
    Bucket &bucket = (*buckets)[firstC];
    if (bucket.full()) abort();
    size_t pos;
    if (!slType) {
        (*sArr)[bucket.lPos] = id;
        pos = bucket.lPos;
        bucket.lPos++;
    } else {
        (*sArr)[bucket.sPos] = id;
        pos = bucket.sPos;
        bucket.sPos--;
    }
    return pos;
}

inline vector<size_t> SuffixArray::Sais_(const vector<size_t> &str, size_t maxValue) const {
    using std::move;
    const vector<bool> slTypes = Classified_(str);
    const vector<size_t> lmsIds = FindLMSs_(slTypes);
    const InducedRV inducedSorted = InducedSort_(maxValue, str, slTypes, lmsIds);
    using std::get;
    const vector<size_t> sArr = std::move(get<0>(inducedSorted));
    size_t countShared = get<2>(inducedSorted);
    if (countShared == 0) {
        return sArr;
    } else {
        const vector<size_t> subStr = std::move(get<1>(inducedSorted));
        const vector<size_t> subSais = Sais_(subStr, lmsIds.size() - 1 - countShared);
        const vector<size_t> rSais = Induce_(maxValue, str, slTypes, lmsIds, &subSais);
        return rSais;
    }
}

inline typename SuffixArray::InducedRV SuffixArray::InducedSort_(size_t maxValue, const vector<size_t> &str, const vector<bool> &slTypes, const vector<size_t> &lmsIds) const {
    const vector<size_t> &induced = Induce_(maxValue, str, slTypes, lmsIds);
    vector<size_t> subAsStr(str.size(), kInf);
    size_t charNumber = 0;
    auto lastLmsId = 0;
    for (auto id : induced) {
        if (id == 0) continue;
        if (!slTypes[id - 1] && slTypes[id]) {
            if (lastLmsId > 0 && !EqualLMSs_(str, slTypes, lastLmsId, id)) {
                charNumber++;
            }
            subAsStr[id] = charNumber;
            lastLmsId = id;
        }
    }
    vector<size_t> subStr(lmsIds.size());
    auto i = 0;
    for (auto subC : subAsStr) {
        if (subC == kInf) continue;
        subStr[i++] = subC;
        subC++;
    }
    auto countShared = lmsIds.size() - 1 - charNumber;
    return InducedRV(induced, subStr, countShared);
}

inline vector<size_t> SuffixArray::Induce_(size_t maxValue, const vector<size_t> &str, const vector<bool> &slTypes, const vector<size_t> &lmsIds, const vector<size_t> *lmssArr) const {
    vector<size_t> sArr(str.size(), kInf);
    vector<Bucket> buckets;
    MakeBuckets_(buckets, str, maxValue);
    // Step1: insert LMSs in bucket
    vector<size_t> insertedLMSPoses(lmsIds.size());
    if (lmssArr == nullptr) {
        for (int i = lmsIds.size() - 1; i >= 0; i--) {
            auto id = lmsIds[i];
            auto pos = PutBucket_(&buckets, &sArr, true, str[id], id);
            insertedLMSPoses[i] = pos;
        }
    } else {
        for (auto it = lmssArr->rbegin(); it != lmssArr->rend(); it++) {
            auto i = *it;
            auto id = lmsIds[i];
            auto pos = PutBucket_(&buckets, &sArr, true, str[id], id);
            insertedLMSPoses[i] = pos;
        }
    }
    // Step2: insert Ls
    for (auto i = 0; i < sArr.size(); i++) {
        auto id = (sArr)[i];
        if (id == kInf || id == 0) continue;
        auto nextId = id - 1;
        if (slTypes[nextId]) continue;
        PutBucket_(&buckets, &sArr, false, str[nextId], nextId);
    }
    // Step3-a: remove LMSs in bucket
    for (auto pos : insertedLMSPoses) {
        // Don't remove [0]: '\0'
        if (pos == 0) continue;
        sArr[pos] = kInf;
    }
    for (auto &b : buckets) {
        b.initSPos();
    }
    // Step3-b: insert Ss
    for (auto i = sArr.size(); i > 0; i--) {
        auto id = (sArr)[i - 1];
        if (id == kInf || id == 0) continue;
        auto nextId = id - 1;
        if (!slTypes[nextId]) continue;
        PutBucket_(&buckets, &sArr, true, str[nextId], nextId);
    }
    return sArr;
}

inline void SuffixArray::BuildLCP_() {
    vector<size_t> posInArr(s_arr_.size());
    for (auto i = 0; i < s_arr_.size(); i++) {
        posInArr[s_arr_[i]] = i;
    }
    vector<size_t> lcp_arr(s_arr_.size());
    auto prevLCP = 0;
    for (auto i = 0; i < posInArr.size(); i++) {
        auto pos = posInArr[i];
        auto lcp = (pos == posInArr.size() - 1) ? 0 : CompareLCP_(i, s_arr_[pos + 1], prevLCP > 1 ? prevLCP - 1 : 0);
        lcp_arr[pos] = lcp;
        prevLCP = lcp;
    }
    
    lcp_arr_ = FitVector(lcp_arr);
    
    size_t maxL = 0;
    long long sum = 0;
    std::sort(lcp_arr.begin(), lcp_arr.end());
    for (auto l : lcp_arr) {
        maxL = std::max(l, maxL);
        sum += l;
    }
    float ave = float(sum) / lcp_arr.size();
    size_t median = lcp_arr[lcp_arr.size() / 2];
    using std::cout, std::endl;
    cout << "------ LCP-Arr Status ------" << endl;
    cout << "Number of elements: " << lcp_arr.size() << endl;
    cout << "Maximum value: " << maxL << endl;
    cout << "Average value: " << ave << endl;
    cout << "Median value: " << median << endl;
    cout << "---- size map ----" << endl;
    auto i = 0;
    std::vector<size_t> map;
    calc::bit_length_frequencies(lcp_arr, &map);
    for (auto num : map)
        cout << "[" << i++ << "]: " << num << endl;
    cout << "----------------------------" << endl;
}

inline size_t SuffixArray::CompareLCP_(size_t li, size_t ri, size_t lcp) const {
    auto matches = lcp;
    li += lcp;
    ri += lcp;
    while (std::max(li, ri) < str_.size() && str_[li] == str_[ri]) {
        matches++;
        li++;
        ri++;
    }
    return matches;
}
    
} // namespace sim_ds

#endif /* SuffixArray_hpp */
