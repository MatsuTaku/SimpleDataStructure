//
//  CodesMeasurer.hpp
//  ArrayFSA
//
//  Created by 松本拓真 on 2018/02/06.
//

#ifndef CodesMeasurer_hpp
#define CodesMeasurer_hpp

#include "basic.hpp"
#include "DACs.hpp"
#include "SACs.hpp"

namespace sim_ds {
    
    class CodesMeasurer {
    public:
        template <class T>
        static void bench(size_t numSeparates = 8) {
            std::cout << "---- Benchmark of " << T::name() << "----" << std::endl;
            
            const size_t num = 0xFFFFF;
            const std::vector<size_t> sizes(numSeparates, 8);
            
            for (auto byte = 1; byte <= numSeparates; byte++) {
                std::vector<uint8_t> highs(num);
                
                std::vector<uint64_t> numbers;
//                if (byte <= 4) {
                    numbers = byteArray(byte, num);
//                } else if (byte == 5) {
//                    numbers = randomArray(num);
//                } else if (byte == 6) {
//                    numbers = curveArray8(num);
//                } else if (byte == 7) {
//                    numbers = curveArray9(num);
//                } else if (byte == 8) {
//                    numbers = curveArray44(num);
//                }
                
                T code(numbers, sizes);
                
                size_t n; // never use
                Stopwatch sw;
                for (auto i = 0; i < num; i++)
                    n = code[i];
                auto mSec = sw.get_micro_sec();
                
                std::cout << byte << " byte time: " << mSec / num * 1000 << " ns/" << num << std::endl;
                std::cout << "       size: " << code.sizeInBytes() << std::endl;
            }
        }
        
        static std::vector<uint64_t> byteArray(size_t size, size_t num) {
            std::vector<uint64_t> numbers(num);
            const uint64_t val = 1ULL << (8 * size - 1);
            for (auto i = 0; i < num; i++) {
                numbers[i] = val;
            }
            return numbers;
        }
        
        static std::vector<size_t> randomArray(size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                auto size = std::rand() % 4;
                numbers[i] = (1U << (8 * size)) - 1;
            }
            return numbers;
        }
        
        static std::vector<size_t> curveArray8(size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                auto size = std::rand() % 15;
                if (size > 3) size = 0;
                numbers[i] = (1U << (8 * size)) - 1;
            }
            return numbers;
        }
        
        static std::vector<size_t> curveArray9(size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                auto size = std::rand() % 30;
                if (size > 3) size = 0;
                numbers[i] = (1U << (8 * size)) - 1;
            }
            return numbers;
        }
        
        static std::vector<size_t> curveArray44(size_t num) {
            std::vector<size_t> numbers(num);
            for (auto i = 0; i < num; i++) {
                auto size = std::rand() % 10;
                if (size > 6) size = 1;
                if (size > 3) size = 0;
                numbers[i] = (1U << (8 * size)) - 1;
            }
            return numbers;
        }
        
    };
    
}

#endif /* CodesMeasurer_hpp */
