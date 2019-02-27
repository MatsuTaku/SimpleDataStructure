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
    
    std::string check_;
    std::vector<id_type> base_;
    std::vector<id_type> next_;
    
protected:
    const id_type kEmptyValue = std::numeric_limits<id_type>::max();
    
public:
    FactorOracleBaseCTAFO(const std::string& text) : check_(text) {
        assert(text.size() < 0xFFFFFFFF);
        base_.assign(text.size() + 1, kEmptyValue);
        size_t empty_trans_front = 0;
        BitVector used_state;
        BitVector used_trans;
        std::vector<id_type> prev;
        
        auto resize = [&](size_t size) {
            next_.resize(size);
            used_state.resize(size);
            used_trans.resize(size);
            prev.resize(size);
        };
        
        auto expand_block = [&] {
            size_t begin = next_.size();
            resize(next_.size() + kBlockSize);
            size_t end = next_.size();
            // empty-element linking
            if (empty_trans_front != 0 or not used_trans[0]) {
                auto old_back = prev[empty_trans_front];
                prev[begin] = old_back;
                next_[old_back] = begin;
                prev[empty_trans_front] = end - 1;
            } else {
                prev[begin] = end - 1;
                empty_trans_front = begin;
            }
            next_[begin] = begin + 1;
            for (size_t i = begin + 1; i < end - 1; i++) {
                next_[i] = i + 1;
                prev[i] = i - 1;
            }
            next_[end - 1] = empty_trans_front;
            prev[end - 1] = end - 2;
        };
        
        auto get_transes = [&](size_t state) -> std::vector<id_type> {
            auto b = base(state);
            if (b == kEmptyValue)
                return {};
            std::vector<id_type> transes;
            for (size_t c = 1; c <= 0xff; c++) {
                auto n = next(b ^ c);
                if (used_trans[b ^ c] and n > 0 and check(n) == c) {
                    transes.push_back(n);
                }
            }
            return transes;
        };
        
        auto find_base = [&](std::vector<id_type> transes) -> size_t {
            for (auto index = empty_trans_front;; index = next(index)) {
                assert(not used_trans[index]);
                auto b = index ^ check(transes.front());
                if (not used_state[b]) {
                    bool skip = false;
                    for (auto t : transes) {
                        if (used_trans[b ^ check(t)]) {
                            skip = true;
                            break;
                        }
                    }
                    if (not skip)
                        return b;
                }
                if (next(index) == empty_trans_front) {
                    expand_block();
                }
            }
            return 0;
        };
        
        auto set_next = [&](size_t index, id_type x) {
            assert(not used_trans[index]);
            if (next(index) != kEmptyValue) {
                if (empty_trans_front == index) {
                    empty_trans_front = next(index) == empty_trans_front ? 0 : next(index);
                }
                next_[prev[index]] = next(index);
                prev[next(index)] = prev[index];
            }
            used_trans[index] = true;
            next_[index] = x;
        };
        
        auto insert_trans = [&](id_type state, id_type next_state) {
            auto c = check(next_state);
            if (base(state) == kEmptyValue) {
                auto init_b = find_base({next_state});
                auto trans = init_b ^ c;
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
                // Move existing transes
                for (size_t i = 0; i < transes.size() - 1; i++) {
                    auto t = transes[i];
                    auto old_trans = old_b ^ check(t);
                    assert(used_trans[old_trans]);
                    next_[old_trans] = kEmptyValue;
                    used_trans[old_trans] = false;
                    set_next(new_b ^ check(t), t);
                }
                // Insert new trans
                set_next(new_b ^ c, next_state);
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
            if (not used_trans[trans])
                return 0;
            auto next_state = next(trans);
            if (next_state == 0 or check(next_state) != check(n)) {
                return 0;
            }
            return next_state;
        };
        
        while (next_.size() < text.size() + 1) {
            expand_block();
        }
        std::vector<id_type> lrs(text.size() + 1);
        
        // Add letters on-line algorithm
        set_next(0, 0);
        for (size_t i = 0; i < text.size(); i++) {
            lrs[0] = i + 1;
            size_t k = lrs[i];
            while (k < i + 1 and translate(k, i + 1) == 0) {
                insert_trans(k, i + 1);
                k = lrs[k];
            }
            lrs[i + 1] = k == i + 1 ? 0 : translate(k, i + 1);
        }
        
        // Initialize empty-element of next to zero.
        for (size_t i = 0; i < next_.size(); i++) {
            if (not used_trans[i])
                next_[i] = 0;
        }
    }
    
    uint8_t check(size_t index) const {return index == 0 ? '\0' : check_[index - 1];}
    
    id_type base(size_t index) const {return base_[index];}
    
    id_type next(size_t index) const {return next_[index];}
    
};

class FactorOracle : FactorOracleBaseCTAFO {
private:
    using Base = FactorOracleBaseCTAFO;
    
public:
    FactorOracle(const std::string& text) : Base(text) {}
    
    bool accept(std::string_view str) const {
        size_t state = 0;
        for (uint8_t c : str) {
            if (Base::check(state + 1) == c) {
                state++;
            } else {
                auto b = Base::base(state);
                if (b == Base::kEmptyValue) {
                    return false;
                }
                auto next_state = Base::next(b ^ c);
                if (next_state <= state or Base::check(next_state) != c) {
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
