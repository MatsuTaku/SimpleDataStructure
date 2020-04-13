//
//  FactorOracle.hpp
//
//  Created by 松本拓真 on 2019/02/24.
//

#ifndef FactorOracle_hpp
#define FactorOracle_hpp

#include "sim_ds/BitVector.hpp"

namespace sim_ds {


template <typename IdType>
class FactorOracleBaseCTAFOBuilder {
public:
    using id_type = IdType;
    
    static constexpr size_t kBlockSize = 0x100;
    static constexpr id_type kEmptyValue = std::numeric_limits<id_type>::max();
    
private:
    std::vector<uint8_t> check_;
    std::vector<id_type> base_;
    std::vector<id_type> next_;
    
    size_t empty_trans_front_ = 0;
    BitVector used_state_;
    BitVector used_trans_;
    std::vector<id_type> prev_;
    
    friend class FactorOracleBaseCTAFO;
    
    void _build() {
//        std::cerr << "_build FO" << std::endl;
        const size_t kKeySize = check_.size();
        const size_t kNumStates = kKeySize + 1;
        
//        std::cerr << " expand block" << std::endl;
        int cnt = 0;
        while (next_.size() < kNumStates - 1) {
//            std::cerr << ++cnt << std::endl;
            _expand_block();
        }
        std::vector<id_type> lrs(kNumStates);
        
//        std::cerr << " edging" << std::endl;
        // Add letters on-line algorithm
        set_next(0, 0);
        for (size_t i = 0; i < kKeySize; i++) {
            lrs[0] = i + 1;
            size_t k = lrs[i];
            while (k < i + 1 and transition(k, i + 1) == 0) {
                _insert_trans(k, i + 1);
                k = lrs[k];
            }
            lrs[i + 1] = k == i + 1 ? 0 : transition(k, i + 1);
        }
        
//        std::cerr << " clear" << std::endl;
        // Initialize empty-element of next to zero.
        for (size_t i = 0; i < next_.size(); i++) {
            if (not used_trans_[i])
                next_[i] = 0;
        }
    }
    
    void _resize(size_t new_size) {
        next_.resize(new_size);
        used_state_.resize(new_size);
        used_trans_.resize(new_size);
        prev_.resize(new_size);
    }
    
    void _expand_block() {
        size_t begin = next_.size();
        _resize(next_.size() + kBlockSize);
        size_t end = next_.size();
        // empty-element linking
        if (empty_trans_front_ != 0 or not used_trans_[0]) {
            auto old_back = prev_[empty_trans_front_];
            prev_[begin] = old_back;
            next_[old_back] = begin;
            prev_[empty_trans_front_] = end - 1;
        } else {
            prev_[begin] = end - 1;
            empty_trans_front_ = begin;
        }
        next_[begin] = begin + 1;
        for (size_t i = begin + 1; i < end - 1; i++) {
            next_[i] = i + 1;
            prev_[i] = i - 1;
        }
        next_[end - 1] = empty_trans_front_;
        prev_[end - 1] = end - 2;
    }
    
    size_t _find_base(const std::vector<id_type>& transes) {
        for (auto index = empty_trans_front_; ; index = next(index)) {
            assert(not used_trans_[index]);
            auto b = index ^ check(transes.front());
            if (not used_state_[b]) {
                bool skip = false;
                for (auto t : transes) {
                    if (used_trans_[b ^ check(t)]) {
                        skip = true;
                        break;
                    }
                }
                if (not skip)
                    return b;
            }
            if (next(index) == empty_trans_front_) {
                _expand_block();
            }
        }
        throw std::logic_error("Not found base due to unknown factor.");
    }
    
    void _insert_trans(id_type state, id_type next_state) {
        auto c = check(next_state);
        if (base(state) == kEmptyValue) { // First trans of state
            auto init_b = _find_base({next_state});
            auto trans = init_b ^ c;
            base_[state] = init_b;
            set_next(trans, next_state);
            used_state_[init_b] = true;
        } else if (used_trans_[base(state) ^ c]) { // Confrict
            auto transes = get_transes(state);
            transes.push_back(next_state);
            auto new_b = _find_base(transes);
            auto old_b = base(state);
            used_state_[old_b] = false;
            used_state_[new_b] = true;
            base_[state] = new_b;
            // Move existing transes
            for (size_t i = 0; i < transes.size() - 1; i++) {
                auto t = transes[i];
                auto old_trans = old_b ^ check(t);
                assert(used_trans_[old_trans]);
                next_[old_trans] = kEmptyValue;
                used_trans_[old_trans] = false;
                set_next(new_b ^ check(t), t);
            }
            // Insert new trans
            set_next(new_b ^ c, next_state);
        } else { // Insert smoothly
            auto trans = base(state) ^ c;
            set_next(trans, next_state);
        }
    }
    
public:
    explicit FactorOracleBaseCTAFOBuilder(std::string_view text) : check_(text.begin(), text.end()), base_(text.size() + 1, kEmptyValue) {
        _build();
    }
    
    template <class InputIter, class IterTraits = std::iterator_traits<InputIter>,
              class CharTraits = std::char_traits<typename IterTraits::value_type>>
    explicit FactorOracleBaseCTAFOBuilder(InputIter begin, InputIter end) : check_(begin, end), base_(end - begin + 1, kEmptyValue) {
        _build();
    }
    
    std::vector<id_type> get_transes(size_t state) const {
        auto b = base(state);
        if (b == kEmptyValue)
            return {};
        std::vector<id_type> transes;
        for (size_t c = 1; c <= 0xff; c++) {
            auto n = next(b ^ c);
            if (used_trans_[b ^ c] and n > 0 and check(n) == c) {
                transes.push_back(n);
            }
        }
        return transes;
    }
    
    void set_next(size_t index, id_type x) {
        assert(not used_trans_[index]);
        if (next(index) != kEmptyValue) {
            if (empty_trans_front_ == index) {
                empty_trans_front_ = next(index) == empty_trans_front_ ? 0 : next(index);
            }
            next_[prev_[index]] = next(index);
            prev_[next(index)] = prev_[index];
        }
        used_trans_[index] = true;
        next_[index] = x;
    }
    
    id_type transition(id_type state, id_type n) {
        if (check(state + 1) == check(n)) {
            return state + 1;
        }
        auto b = base(state);
        if (b == kEmptyValue)
            return 0;
        auto trans = b ^ check(n);
        if (not used_trans_[trans])
            return 0;
        auto next_state = next(trans);
        if (next_state == 0 or check(next_state) != check(n)) {
            return 0;
        }
        return next_state;
    }
    
    uint8_t check(size_t index) const {return index == 0 ? '\0' : check_[index - 1];}
    
    id_type base(size_t index) const {return base_[index];}
    
    id_type next(size_t index) const {return next_[index];}
    
};


class FactorOracleBaseCTAFO {
public:
    using id_type = uint32_t;
    
    using Builder = FactorOracleBaseCTAFOBuilder<id_type>;
    
    static constexpr id_type kEmptyValue = std::numeric_limits<id_type>::max();
    
private:
    std::vector<uint8_t> check_;
    std::vector<id_type> base_;
    std::vector<id_type> next_;
    
public:
    explicit FactorOracleBaseCTAFO(Builder&& builder) : check_(std::move(builder.check_)), base_(std::move(builder.base_)), next_(std::move(builder.next_)) {}
    
    template <class InputIter>
    explicit FactorOracleBaseCTAFO(InputIter begin, InputIter end) : FactorOracleBaseCTAFO(Builder(begin, end)) {
        static_assert(std::is_convertible_v<typename std::iterator_traits<InputIter>::value_type, char>);
    }
    
    explicit FactorOracleBaseCTAFO(std::string_view text) : FactorOracleBaseCTAFO(text.begin(), text.end()) {}
    
    uint8_t check(size_t index) const {return index == 0 ? '\0' : check_[index - 1];}
    
    id_type base(size_t index) const {return base_[index];}
    
    id_type next(size_t index) const {return next_[index];}
    
};


class FactorOracleExproler {
private:
    std::string_view str_;
    size_t pos_;
    
    friend class FactorOracle;
    
public:
    FactorOracleExproler(std::string_view str) : str_(str), pos_(0) {}
    
    std::string_view text() {return str_;}
    
    uint8_t c() const {return str_[pos_];}
    
    size_t size() const {return str_.size();}
    
    size_t pos() const {return pos_;}
    
};


class FactorOracle : FactorOracleBaseCTAFO {
public:
    using Base = FactorOracleBaseCTAFO;
    using Exproler = FactorOracleExproler;
    
    FactorOracle(const std::string& text) : Base(text) {}
    
    template <class InputIter,
              typename IterTraits = std::iterator_traits<InputIter>,
              typename CharTraits = std::char_traits<typename IterTraits::value_type>>
    FactorOracle(InputIter begin, InputIter end) : Base(begin, end) {}
    
    bool image(size_t& state, uint8_t c) const {
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
        return true;
    }
    
    bool accept(Exproler& exp) const {
        size_t state = 0;
        for (; exp.pos_ < exp.size(); exp.pos_++) {
            if (not image(state, exp.c()))
                return false;
        }
        return true;
    }
    
    bool accept(std::string_view str) const {
        Exproler exp(str);
        return accept(exp);
    }
    
};

}

#endif /* FactorOracle_hpp */
