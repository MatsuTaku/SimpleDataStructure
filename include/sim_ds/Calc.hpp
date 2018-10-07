//
//  Calc.hpp
//  array_fsa
//
//  Created by 松本拓真 on 2017/12/10.
//

#ifndef Calc_hpp
#define Calc_hpp

#include <stdio.h>
#include <malloc/malloc.h>

namespace sim_ds {
    
    namespace calc {
    
        size_t sizeFitInBytes(uint64_t value) {
            return sizeFitInUnits(value, 8);
        }
        
        size_t sizeFitInBits(uint64_t value) {
            return sizeFitInUnits(value, 1);
        }
        
        size_t sizeFitInUnits(uint64_t value, size_t unit) {
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
        
        size_t sizeFitAsSizeList(size_t value, size_t sizes[]) {
            size_t size = 0;
            while (value >>= sizes[size++]);
            assert(size <= 8);
            return size;
        }
        
        std::vector<size_t> separateCountsInSizeOf(const std::vector<size_t> &list) {
            std::vector<size_t> counts(4);
            for (auto v : list) {
                auto size = sizeFitInBytes(v);
                ++counts[size - 1];
            }
            return counts;
        }
        
        std::vector<size_t> separateCountsInXorSizeOf(const std::vector<size_t> &list) {
            std::vector<size_t> counts(4);
            for (auto i = 0; i < list.size(); i++) {
                auto size = sizeFitInBytes(list[i] ^ i);
                ++counts[size - 1];
            }
            return counts;
        }
        
        size_t vectorMapOfSizeBits(std::vector<size_t>* map, const std::vector<size_t> &list, bool show = false) {
            auto maxSize = 0;
            for (auto v : list) {
                auto size = sizeFitInBits(v);
                if (size > maxSize) {
                    map->resize(size, 0);
                    maxSize = size;
                }
                (*map)[size - 1]++;
            }
            if (show) {
                for (auto i = 0; i < map->size(); i++) {
                    std::cout << "[" << i + 1 << "]: " << (*map)[i] << std::endl;
                }
            }
            
            return map->size();
        }
        
        size_t setCummulativeFrequency(std::vector<size_t>* cf, const std::vector<size_t> &list) {
            std::vector<size_t> map;
            vectorMapOfSizeBits(&map, list);
            auto count = 0;
            cf->assign(map.size(), 0);
            for (auto i = map.size(); i > 0; i--) {
                count += map[i - 1];
                (*cf)[i - 1] = count;
            }
            return cf->size();
        }
        
        size_t sizeOfDacFromParams(double l) {
            using std::ceil;
            auto a = ceil(l / 8) * 8;
            auto b = (4 * 8 + ceil(log2(l))) * ceil(l / 256) ;
            return a + b;
        }
        
        template <typename T>
        size_t optimizedBitsListForDac(std::vector<T>* bits, const std::vector<size_t> &list, size_t minCost = 1, size_t maxLevels = 8) {
            
            std::vector<size_t> cf;
            auto cfSize = setCummulativeFrequency(&cf, list);
            
            const auto m = cfSize - 1;
            std::vector<size_t> s(cfSize), l(cfSize), b(cfSize);
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
            auto &bk = *bits;
            bk.reserve(8);
            bk.resize(L);
            for (auto k = 0; k <= L; k++) {
                bk[k] = b[t];
                t = t + b[t];
            }
            
//            assert(malloc_zone_check(NULL));
            
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
            return bk.size();
        }
        
    };
    
}

#endif /* Calc_hpp */
