//
//  FactorOracle.hpp
//  BitVector_test
//
//  Created by 松本拓真 on 2019/02/24.
//

#ifndef FactorOracle_hpp
#define FactorOracle_hpp

#include "basic.hpp"
#include "BitVector.hpp"

namespace sim_ds {

class FactorOracleBaseCTAFO {
private:
    using id_type = uint32_t;
    
    const size_t kBlockSize = 0x100;
    const id_type kEmptyValue = std::numeric_limits<id_type>::max();
    
    std::string check_;
    std::vector<id_type> base_;
    std::vector<id_type> next_;
    
public:
    FactorOracleBaseCTAFO(const std::string& text) : check_(text), base_(text.size() + 1, kEmptyValue) {
        size_t empty_trans_header = 0;
        BitVector used_state;
        BitVector used_trans;
        
        auto resize = [&](size_t size) {
            next_.resize(size);
            used_state.resize(size);
            used_trans.resize(size);
        };
        
        auto expand_block = [&] {
            size_t begin = next_.size();
            resize(next_.size() + kBlockSize);
            size_t end = next_.size();
            // empty-element linking
            for (long i = begin - 1; i >= 0; i--) {
                if (not used_trans[i]) {
                    next_[i] = begin;
                    break;
                }
            }
            for (size_t i = begin; i < end - 1; i++) {
                next_[i] = (i + 1) % next_.size();
            }
            for (size_t i = 0; i < begin; i++) {
                if (not used_trans[i]) {
                    next_[end - 1] = i;
                    break;
                }
            }
            if (empty_trans_header == 0)
                empty_trans_header = begin;
        };
        
        auto get_transes = [&](size_t state) -> std::vector<id_type> {
            auto b = base(state);
            std::vector<id_type> transes;
            for (id_type c = 0; c <= 0xff; c++) {
                auto n = next(b ^ c);
                if (check(n) == c) {
                    transes.push_back(n);
                }
            }
            return transes;
        };
        
        auto find_base = [&](std::vector<id_type> transes) -> size_t {
            for (auto index = empty_trans_header;;) {
                auto b = index ^ check(transes.front());
                bool skip = used_state[b];
                if (not skip) {
                    for (auto t : transes) {
                        if (used_trans[b ^ check(t)]) {
                            skip = true;
                            break;
                        }
                    }
                }
                if (not skip)
                    return b;
                if (next(index) <= empty_trans_header) {
                    expand_block();
                }
                if (used_state[b])
                    index++;
                else
                    index = next(index);
            }
            return 0;
        };
        
        auto set_next = [&](size_t index, id_type x) {
            if (empty_trans_header == index) {
                empty_trans_header = next_[index];
            }
            used_trans[index] = true;
            next_[index] = x;
        };
        
        auto insert_trans = [&](id_type state, id_type next_state) {
            auto c = check(next_state);
            if (base(state) == kEmptyValue) {
                auto init_b = find_base({next_state});
                auto trans = init_b ^ check(next_state);
                base_[state] = init_b;
                set_next(trans, next_state);
                used_state[init_b] = true;
            } else if (used_trans[base(state) ^ c]) {
                auto transes = get_transes(state);
                transes.push_back(next_state);
                auto new_b = find_base(transes);
                auto old_b = base(state);
                used_state[old_b] = false;
                used_state[new_b] = true;
                base_[state] = new_b;
                for (auto t : transes) {
                    auto old_trans = old_b ^ check(t);
                    auto new_trans = new_b ^ check(t);
                    next_[old_trans] = 0;
                    used_trans[old_trans] = false;
                    set_next(new_trans, t);
                }
            } else {
                auto trans = base(state) ^ c;
                set_next(trans, next_state);
            }
        };
        
        auto translate = [&](id_type state, id_type n) -> id_type {
            if (check(state + 1) == check(n)) {
                return state + 1;
            }
            auto b = base(state);
            if (b == kEmptyValue)
                return 0;
            auto trans = b ^ check(n);
            if (check(next(trans)) != check(n)) {
                return 0;
            }
            return next(trans);
        };
        
        while (next_.size() < text.size() + 1) {
            expand_block();
        }
        set_next(0, 0);
        std::vector<id_type> lrs(text.size() + 1);
        for (size_t i = 0; i < text.size() + 1; i++) {
            lrs[0] = i + 1;
            if (lrs[i] == i + 1) {
                lrs[i + 1] = 0;
            } else {
                if (lrs[i] == 0) {
                    lrs[i + 1] = translate(0, i + 1);
                } else {
                    lrs[i + 1] = translate(lrs[i], i + 1);
                    if (lrs[i + 1] == 0)
                        lrs[i + 1] = translate(0, i + 1);
                }
            }
            size_t k = lrs[i];
            while (k < i + 1 and translate(k, i + 1) == 0) {
                insert_trans(k, i + 1);
                k = lrs[k];
            }
        }
    }
    
    uint8_t check(size_t index) const {return index == 0 ? '\0' : check_[index - 1];}
    
    id_type base(size_t index) const {return base_[index];}
    
    id_type next(size_t index) const {return next_[index];}
    
};

class FactorOracle : FactorOracleBaseCTAFO {
public:
    FactorOracle(const std::string& text) : FactorOracleBaseCTAFO(text) {}
    
    bool accept(std::string_view str) const {
        size_t state = 0;
        for (uint8_t c : str) {
            if (check(state + 1) == c) {
                state++;
            } else {
                auto next_state = next(base(state) ^ c);
                if (next_state <= state or check(next_state) != c) {
                    return false;
                }
                state = next_state;
            }
        }
        return true;
    }
    
};
    
}

#endif /* FactorOracle_hpp */
