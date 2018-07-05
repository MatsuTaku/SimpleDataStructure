//
//  Calc.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/12/10.
//

#ifndef Calc_hpp
#define Calc_hpp

#include <stdio.h>

namespace sim_ds {
    
    class Calc {
    public:
        static constexpr size_t sizeFitInBytes(uint64_t value) {
            return sizeFitInUnits(value, 8);
        }
        
        static constexpr size_t sizeFitInBits(uint64_t value) {
            return sizeFitInUnits(value, 1);
        }
        
        static constexpr size_t sizeFitInUnits(uint64_t value, size_t unit) {
            size_t size = 1;
            while (value >>= unit) {
                size++;
                if (size > 0x40 / unit) {
                    std::cerr << (value >> (unit * size - 2)) << std::endl;
                    abort();
                }
            }
            return size;
        }
        
        static size_t sizeFitAsSizeList(size_t value, size_t sizes[]) {
            size_t size = 0;
            while (value >>= sizes[size++]);
            assert(size <= 8);
            return size;
        }
        
        static std::vector<size_t> separateCountsInSizeOf(const std::vector<size_t> &list) {
            std::vector<size_t> counts(4);
            for (auto v : list) {
                auto size = sizeFitInBytes(v);
                ++counts[size - 1];
            }
            return counts;
        }
        
        static std::vector<size_t> separateCountsInXorSizeOf(const std::vector<size_t> &list) {
            std::vector<size_t> counts(4);
            for (auto i = 0; i < list.size(); i++) {
                auto size = sizeFitInBytes(list[i] ^ i);
                ++counts[size - 1];
            }
            return counts;
        }
        
        static std::vector<size_t> vectorMapOfSizeBits(const std::vector<size_t> &list) {
            std::vector<size_t> map;
            auto maxSize = 0;
            for (auto v : list) {
                auto size = sizeFitInBits(v);
                if (size > maxSize) {
                    map.resize(size, 0);
                    maxSize = size;
                }
                map[size - 1]++;
            }
            for (auto i = 0; i < map.size(); i++) {
                std::cout << "[" << i + 1 << "]: " << map[i] << std::endl;
            }
            return map;
        }
        
        static std::vector<size_t> cummulativeFrequenciesVector(const std::vector<size_t> &list) {
            const auto map = vectorMapOfSizeBits(list);
            auto count = 0;
            std::vector<size_t> cf(map.size());
            for (auto i = map.size(); i > 0; i--) {
                count += map[i - 1];
                cf[i - 1] = count;
            }
            return cf;
        }
        
        static size_t sizeOfDacFromParams(double l) {
            auto a = std::ceil(l / 8);
            auto b = 12 * std::ceil(l / 256) ;
            return (a + b) * 8;
        }
        
        static std::vector<size_t> optimizedBitsListForDac(const std::vector<size_t> &list, size_t minCost = 1, size_t maxLevels = 8) {
            const auto cf = cummulativeFrequenciesVector(list);
            const auto m = cf.size() - 1;
            std::vector<size_t> s(cf.size()), l(cf.size()), b(cf.size());
            for (int t = m; t >= 0; --t) {
                auto minSize = INFINITY;
                auto minPos = m;
                for (auto i = t + minCost; i <= m; i++) {
                    auto currentSize = s[i] + cf[t] * (i - t) + sizeOfDacFromParams(cf[t]);
                    if (currentSize < minSize) {
                        minSize = currentSize;
                        minPos = i;
                    }
                }
                if (minSize < cf[t] * ((m + 1) - t)) {
                    s[t] = minSize;
                    l[t] = l[minPos] + 1;
                    b[t] = minPos - t;
                } else {
                    s[t] = cf[t] * ((m + 1) - t);
                    l[t] = 1;
                    b[t] = (m + 1) - t;
                }
            }
            auto L = l[0];
            auto t = 0;
            std::vector<size_t> bk(L);
            for (auto k = 0; k <= L; k++) {
                bk[k] = b[t];
                t = t + b[t];
            }
            
            if (L > maxLevels) {
                std::vector<bool> sepPos(cf.size() + 1, false);
                auto p = 0;
                for (auto v : bk) {
                    p += v;
                    sepPos[p] = true;
                }
                using P = std::pair<uint64_t, uint64_t>;
                auto cmp = [](P lhs, P rhs) {
                    return lhs.first > rhs.first;
                };
                std::vector<P> pq;
                for (auto i = 1; i < m; i++) {
                    pq.push_back(P(cf[i - 1] - cf[i], i));
                }
                std::sort(pq.begin(), pq.end(), cmp);
                auto numSeps = L;
                while (numSeps > maxLevels && !pq.empty()) {
                    auto curP = pq.back();
                    pq.pop_back();
                    if (sepPos[curP.second]) {
                        sepPos[curP.second] = false;
                        numSeps--;
                    }
                }
                
                bk.resize(0);
                auto lastP = 0;
                for (auto i = 0; i < sepPos.size(); i++) {
                    if (sepPos[i]) {
                        bk.push_back(i - lastP);
                        lastP = i;
                    }
                }
            }
            return bk;
        }
        
        static size_t sizeOfSacFromPrams(double l, long long n) {
            long long ln = n > 2 ? std::ceil(std::log2(n)) : 1;
            auto sb = std::ceil(l * ln / 8);
            auto a = std::floor(64.0 / ln);
            auto b = std::floor(256.0 / a);
            auto sr = (8 + b) * std::ceil(l / (a * b));
            return sb + sr;
        }
        
        static std::vector<size_t> optimizedBitsListForSac(const std::vector<size_t> &list, size_t minCost = 1, size_t maxLevels = 8) {
            const auto cf = cummulativeFrequenciesVector(list);
            const auto m = cf.size() - 1;
            std::vector<size_t> s(cf.size()), gf(cf.size(), 0), gt(cf.size(), cf.size());
            const auto L = cf[0];
            auto n = 1;
            while (n < cf.size() && n < maxLevels) {
                long long maxDiff = 0;
                auto maxPos = m;
                for (int t = m; t > 0; t--) {
                    if (t - gf[t] < minCost || gt[t] - t < minCost) continue;
                    long long curDiff = cf[t - 1] - cf[t];
                    if (curDiff > maxDiff) {
                        maxDiff = curDiff;
                        maxPos = t;
                    }
                }
                auto rankSizeInc = sizeOfSacFromPrams(L, n + 1) * n - sizeOfSacFromPrams(L, n) * (n - 1);
                long long reductiveSize = (gt[maxPos] - maxPos) * (cf[gf[maxPos]] - cf[maxPos]) - rankSizeInc;
                if (reductiveSize <= 0) break;
                for (auto i = gf[maxPos]; i < maxPos; i++)
                    gt[i] = maxPos;
                for (auto i = maxPos; i < gt[maxPos]; i++)
                    gf[i] = maxPos;
                n++;
            }
            std::vector<size_t> bk(n);
            auto t = 0;
            for (auto k = 0; k < n; k++) {
                bk[k] = gt[t] - t;
                t = gt[t];
            }
            return bk;
        }
        
    };
    
}

#endif /* Calc_hpp */
