#ifndef SIDS_INCLUDE_SIM_DS_CHT_H_
#define SIDS_INCLUDE_SIM_DS_CHT_H_

#include "FitVector.hpp"
#include "bit_util.hpp"

#include <bitset>

namespace sim_ds {

class Xorshift {
private:
    unsigned shifts_;

public:
    explicit Xorshift(unsigned width) : shifts_((width+1)/2) {}

    template<unsigned K>
    uint64_t hash(uint64_t x) const {
        x = (x ^ (x >> (shifts_+K)));
        return x;
    }
    template<unsigned K>
    uint64_t ihash(uint64_t x) const {
        return hash<K>(x);
    }

};

constexpr uint64_t PRIME_TABLE[][2][3] = {
    {{0ULL, 0ULL, 0ULL}, {0ULL, 0ULL, 0ULL}},  // 0
    {{1ULL, 1ULL, 1ULL}, {1ULL, 1ULL, 1ULL}},  // 1
    {{3ULL, 1ULL, 3ULL}, {3ULL, 1ULL, 3ULL}},  // 2
    {{7ULL, 5ULL, 3ULL}, {7ULL, 5ULL, 3ULL}},  // 3
    {{13ULL, 11ULL, 7ULL}, {5ULL, 3ULL, 7ULL}},  // 4
    {{31ULL, 29ULL, 23ULL}, {31ULL, 21ULL, 7ULL}},  // 5
    {{61ULL, 59ULL, 53ULL}, {21ULL, 51ULL, 29ULL}},  // 6
    {{127ULL, 113ULL, 109ULL}, {127ULL, 17ULL, 101ULL}},  // 7
    {{251ULL, 241ULL, 239ULL}, {51ULL, 17ULL, 15ULL}},  // 8
    {{509ULL, 503ULL, 499ULL}, {341ULL, 455ULL, 315ULL}},  // 9
    {{1021ULL, 1019ULL, 1013ULL}, {341ULL, 819ULL, 93ULL}},  // 10
    {{2039ULL, 2029ULL, 2027ULL}, {455ULL, 1509ULL, 195ULL}},  // 11
    {{4093ULL, 4091ULL, 4079ULL}, {1365ULL, 819ULL, 3855ULL}},  // 12
    {{8191ULL, 8179ULL, 8171ULL}, {8191ULL, 4411ULL, 4291ULL}},  // 13
    {{16381ULL, 16369ULL, 16363ULL}, {5461ULL, 4369ULL, 12483ULL}},  // 14
    {{32749ULL, 32719ULL, 32717ULL}, {13797ULL, 10031ULL, 1285ULL}},  // 15
    {{65521ULL, 65519ULL, 65497ULL}, {4369ULL, 3855ULL, 36969ULL}},  // 16
    {{131071ULL, 131063ULL, 131059ULL}, {131071ULL, 29127ULL, 110907ULL}},  // 17
    {{262139ULL, 262133ULL, 262127ULL}, {209715ULL, 95325ULL, 200463ULL}},  // 18
    {{524287ULL, 524269ULL, 524261ULL}, {524287ULL, 275941ULL, 271853ULL}},  // 19
    {{1048573ULL, 1048571ULL, 1048559ULL}, {349525ULL, 209715ULL, 986895ULL}},  // 20
    {{2097143ULL, 2097133ULL, 2097131ULL}, {1864135ULL, 1324517ULL, 798915ULL}},  // 21
    {{4194301ULL, 4194287ULL, 4194277ULL}, {1398101ULL, 986895ULL, 3417581ULL}},  // 22
    {{8388593ULL, 8388587ULL, 8388581ULL}, {1118481ULL, 798915ULL, 3417581ULL}},  // 23
    {{16777213ULL, 16777199ULL, 16777183ULL}, {5592405ULL, 986895ULL, 15760415ULL}},  // 24
    {{33554393ULL, 33554383ULL, 33554371ULL}, {17207401ULL, 31500079ULL, 15952107ULL}},  // 25
    {{67108859ULL, 67108837ULL, 67108819ULL}, {53687091ULL, 62137837ULL, 50704475ULL}},  // 26
    {{134217689ULL, 134217649ULL, 134217617ULL}, {17207401ULL, 113830225ULL, 82223473ULL}},  // 27
    {{268435399ULL, 268435367ULL, 268435361ULL}, {131863031ULL, 96516119ULL, 186492001ULL}},  // 28
    {{536870909ULL, 536870879ULL, 536870869ULL}, {357913941ULL, 32537631ULL, 274678141ULL}},  // 29
    {{1073741789ULL, 1073741783ULL, 1073741741ULL}, {889671797ULL, 1047552999ULL, 349289509ULL}},  // 30
    {{2147483647ULL, 2147483629ULL, 2147483587ULL}, {2147483647ULL, 1469330917ULL, 1056139499ULL}},  // 31
    {{4294967291ULL, 4294967279ULL, 4294967231ULL}, {858993459ULL, 252645135ULL, 1057222719ULL}},  // 32
    {{8589934583ULL, 8589934567ULL, 8589934543ULL}, {7635497415ULL, 1030792151ULL, 3856705327ULL}},  // 33
    {{17179869143ULL, 17179869107ULL, 17179869071ULL}, {9637487591ULL, 11825104763ULL, 12618841967ULL}},  // 34
    {{34359738337ULL, 34359738319ULL, 34359738307ULL}, {1108378657ULL, 21036574511ULL, 22530975979ULL}},  // 35
    {{68719476731ULL, 68719476719ULL, 68719476713ULL}, {13743895347ULL, 64677154575ULL, 8963410009ULL}},  // 36
    {{137438953447ULL, 137438953441ULL, 137438953427ULL}, {43980465111ULL, 35468117025ULL, 70246576219ULL}},  // 37
    {{274877906899ULL, 274877906857ULL, 274877906837ULL}, {207685529691ULL, 41073710233ULL, 208085144509ULL}},  // 38
    {{549755813881ULL, 549755813869ULL, 549755813821ULL}, {78536544841ULL, 347214198245ULL, 369238979477ULL}},  // 39
    {{1099511627689ULL, 1099511627609ULL, 1099511627581ULL}, {315951617177ULL, 928330176745ULL, 343949791253ULL}},  // 40
    {{2199023255531ULL, 2199023255521ULL, 2199023255497ULL}, {209430786243ULL, 1134979744801ULL, 1119502748281ULL}},  // 41
    {{4398046511093ULL, 4398046511087ULL, 4398046511071ULL}, {1199467230301ULL, 3363212037903ULL, 3331853417503ULL}},  // 42
    {{8796093022151ULL, 8796093022141ULL, 8796093022091ULL}, {8178823336439ULL, 918994793365ULL, 2405769031715ULL}},  // 43
    {{17592186044399ULL, 17592186044299ULL, 17592186044297ULL}, {16557351571215ULL, 2405769031715ULL, 2365335938745ULL}},  // 44
    {{35184372088777ULL, 35184372088763ULL, 35184372088751ULL}, {27507781814905ULL, 17847145262451ULL, 11293749065551ULL}},  // 45
    {{70368744177643ULL, 70368744177607ULL, 70368744177601ULL}, {13403570319555ULL, 34567102403063ULL, 4467856773185ULL}},  // 46
    {{140737488355213ULL, 140737488355201ULL, 140737488355181ULL}, {88113905752901ULL, 4432676798593ULL, 22020151239269ULL}},  // 47
    {{281474976710597ULL, 281474976710591ULL, 281474976710567ULL}, {100186008659725ULL, 4330384257087ULL, 123342967322647ULL}},  // 48
    {{562949953421231ULL, 562949953421201ULL, 562949953421189ULL}, {222399981598543ULL, 25358106009969ULL, 366146311168333ULL}},  // 49
    {{1125899906842597ULL, 1125899906842589ULL, 1125899906842573ULL}, {667199944795629ULL, 289517118902389ULL, 286994093901061ULL}},  // 50
    {{2251799813685119ULL, 2251799813685109ULL, 2251799813685083ULL}, {558586000294015ULL, 161999986596061ULL, 232003617167571ULL}},  // 51
    {{4503599627370449ULL, 4503599627370353ULL, 4503599627370323ULL}, {3449565672028465ULL, 3558788516733329ULL, 3514369651416283ULL}},  // 52
    {{9007199254740881ULL, 9007199254740847ULL, 9007199254740761ULL}, {2840107873116529ULL, 496948924399503ULL, 4991002184445225ULL}},  // 53
    {{18014398509481951ULL, 18014398509481931ULL, 18014398509481853ULL}, {16922616781634591ULL, 13595772459986403ULL, 6600695637062101ULL}},  // 54
    {{36028797018963913ULL, 36028797018963901ULL, 36028797018963869ULL}, {20962209174669945ULL, 20434243085382549ULL, 11645671763705525ULL}},  // 55
    {{72057594037927931ULL, 72057594037927909ULL, 72057594037927889ULL}, {14411518807585587ULL, 18681598454277613ULL, 21463964181510449ULL}},  // 56
    {{144115188075855859ULL, 144115188075855823ULL, 144115188075855811ULL}, {88686269585142075ULL, 44116894308935471ULL, 18900352534538475ULL}},  // 57
    {{288230376151711687ULL, 288230376151711681ULL, 288230376151711607ULL}, {126416831645487607ULL, 18300341342965825ULL, 136751638320155207ULL}},  // 58
    {{576460752303423263ULL, 576460752303423061ULL, 576460752303422971ULL}, {5124095576030431ULL, 2700050362076925ULL, 198471980483577139ULL}},  // 59
    {{1152921504606846883ULL, 1152921504606846803ULL, 1152921504606846697ULL}, {12397005425880075ULL, 566464323072728283ULL, 4132335141960025ULL}},  // 60
    {{2305843009213693951ULL, 2305843009213693669ULL, 2305843009213693613ULL}, {2305843009213693951ULL, 1768084568902373101ULL, 360500529464087845ULL}},  // 61
    {{4611686018427387733ULL, 4611686018427387421ULL, 4611686018427387271ULL}, {4557748170258646525ULL, 152768066863019061ULL, 1515372340968241207ULL}},  // 62
    {{9223372036854775291ULL, 9223372036854775279ULL, 9223372036854775181ULL}, {3657236494304118067ULL, 2545580940228350223ULL, 3339243145719352645ULL}}  // 63
};

class MultiplyHash {
private:
    unsigned width_;
    uint64_t mask_;
public:
    explicit MultiplyHash(unsigned width) :
        width_(width),
        mask_(width < 64 ? (1ull<<width)-1 : ~0ll) {}

    template<unsigned K>
    uint64_t hash(uint64_t x) const {
        x = (x * PRIME_TABLE[width_][0][K]) & mask_;
        return x;
    }

    template<unsigned K>
    uint64_t ihash(uint64_t x) const {
        x = (x * PRIME_TABLE[width_][1][K]) & mask_;
        return x;
    }
};

class SplitMixHash {
private:
    Xorshift xorshift_;
    MultiplyHash multiply_hash_;

public:
    explicit SplitMixHash(unsigned width) :
        xorshift_(width), multiply_hash_(width) {}

    uint64_t hash(uint64_t x) const {
        x = hash_<0>(x);
        x = hash_<1>(x);
        x = hash_<2>(x);
        return x;
    }

    uint64_t ihash(uint64_t x) const {
        x = ihash_<2>(x);
        x = ihash_<1>(x);
        x = ihash_<0>(x);
        return x;
    }

private:
    template<unsigned K>
    uint64_t hash_(uint64_t x) const {
        x = xorshift_.hash<K>(x);
        x = multiply_hash_.hash<K>(x);
        return x;
    }

    template<unsigned K>
    uint64_t ihash_(uint64_t x) const {
        x = multiply_hash_.ihash<K>(x);
        x = xorshift_.ihash<K>(x);
        return x;
    }
};

template <
    unsigned ValueBits,
    unsigned MaxLoadFactorPercent = 50,
    typename BijectiveHash = SplitMixHash>
class CHT {
    static_assert(MaxLoadFactorPercent < 100);
public:
    static constexpr size_t kDefaultBucketSize = 8;
    static constexpr unsigned kFlagBits = 3;
    static constexpr uint64_t kOccupiedMask = 1u << 0;
    static constexpr uint64_t kContinuationMask = 1u << 1;
    static constexpr uint64_t kShiftedMask = 1u << 2;

private:
    unsigned key_bits_;
    size_t bucket_size_;
    unsigned bucket_bits_;
    uint64_t bucket_mask_;
    size_t max_size_;
    size_t min_size_;
    BijectiveHash hasher_;
    FitVector arr_;
    FitVector values_;
    uint64_t quo_mask_;
    size_t size_ = 0;
    size_t used_ = 0;

    bool IsOccupied(size_t i) const { return arr_[i] & kOccupiedMask; }
    bool IsContinuation(size_t i) const { return arr_[i] & kContinuationMask; }
    bool IsShifted(size_t i) const { return arr_[i] & kShiftedMask; }

    uint64_t Quotient(size_t i) const { return (arr_[i] & quo_mask_) >> kFlagBits; }

public:
    CHT(unsigned key_bits, size_t bucket_size = kDefaultBucketSize) :
        key_bits_(key_bits),
        bucket_size_(bucket_size),
        bucket_bits_(bit_util::ctz(bucket_size)),
        bucket_mask_((1ull<<bucket_bits_)-1),
        max_size_(bucket_size*MaxLoadFactorPercent/100),
        min_size_(bucket_size*(MaxLoadFactorPercent-1)/(MaxLoadFactorPercent+100)+1),
        values_(ValueBits, bucket_size_),
        hasher_(key_bits_) {
        assert(bit_util::popcnt(bucket_size) == 1);
        unsigned quo_bits = std::max(0, (int)key_bits - (int)bucket_bits_);
        arr_ = FitVector(quo_bits + kFlagBits, bucket_size_);
        quo_mask_ = ((1ull << quo_bits)-1) << kFlagBits;
    }

    size_t ToClusterHead(size_t i) const {
        if (!IsShifted(i))
            return i;
        size_t t = 0;
        do {
            i = pred(i);
            if (IsOccupied(i))
                ++t;
        } while (IsShifted(i));
        while (t) {
            i = succ(i);
            if (!IsContinuation(i))
                --t;
        }
        return i;
    }

    std::pair<bool, uint64_t> get(uint64_t key) const {
        assert(64-bit_util::clz(key) <= key_bits_);

        uint64_t initial_h = hasher_.hash(key);
        auto quo = initial_h >> bucket_bits_;
        size_t i = initial_h & bucket_mask_;
        if (!IsOccupied(i))
            return {false, 0};
        i = ToClusterHead(i);
        do {
            if (Quotient(i) == quo) {
                return {true, values_[i]};
            }
            i = succ(i);
        } while (IsContinuation(i));
        return {false, 0};
    }

    bool needs_expand() const {
        return used_ == max_size_;
    }

    void set(size_t key, uint64_t value) {
        assert(64-bit_util::clz(key) <= key_bits_);
        assert(64-bit_util::clz(value) <= ValueBits);

        if (needs_expand()) {
            expand();
        }

        uint64_t initial_h = hasher_.hash(key);
        auto quo = initial_h >> bucket_bits_;
        size_t initial_i = initial_h & bucket_mask_;
        auto i = ToClusterHead(initial_i);
        if (IsOccupied(initial_i)) {
            do {
                if (Quotient(i) == quo) {
                    values_[i] = value;
                    return;
                }
                i = succ(i);
            } while (IsContinuation(i));
        }
        if (arr_[i] % (1u<<kFlagBits) != 0) {
            auto j = i;
            do {
                j = succ(j);
            } while (arr_[j] % (1u<<kFlagBits) != 0);
            do {
                auto pj = pred(j);
                arr_[j] = (
                    (arr_[j] & kOccupiedMask) |
                    (arr_[pj] & (quo_mask_|kContinuationMask)) |
                    kShiftedMask
                );
                values_[j] = values_[pj];
                j = pj;
            } while (i != j);
            assert(j == i);
        }
        if (!IsOccupied(initial_i)) { // New cluster
            arr_[initial_i] = arr_[initial_i] | kOccupiedMask;
            arr_[i] = (
                (arr_[i] & kOccupiedMask) |
                (quo << kFlagBits) |
                (i != initial_i ? kShiftedMask : 0)
            );
        } else { // Already exists cluster
            arr_[i] = (
                (arr_[i] & kOccupiedMask) |
                (quo << kFlagBits) |
                kContinuationMask |
                kShiftedMask
            );
        }
        ++size_;
        ++used_;
        values_[i] = value;
    }

    size_t succ(size_t i) const {
        i++;
        [[unlikely]] if (i == bucket())
            i = 0;
        return i;
    }

    size_t pred(size_t i) const {
        [[unlikely]] if (i == 0)
            return bucket()-1;
        else
            return i-1;
    }

    void print_for_debug() const {
        int cnt=0;
        for (int i = 0; i < bucket(); i++) {
            std::cout << i << "] "
                      << bool(arr_[i]&kOccupiedMask)
                      << bool(arr_[i]&kContinuationMask)
                      << bool(arr_[i]&kShiftedMask)
                      << std::endl;
            if (IsShifted(i))
                ++cnt;
        }
        std::cout<<"cnt shifted: "<<cnt<<std::endl;
    }

    void expand() {
        auto need_size = size_*2*(100-1)/MaxLoadFactorPercent+1;
        auto new_bucket_bits = 64-bit_util::clz(need_size-1);
        expand(1ull<<new_bucket_bits);
    }

    void expand(size_t new_bucket_size) {
        CHT next(key_bits_, new_bucket_size);
        size_t i = 0;
        size_t cnt = 0;
        while (cnt < size()) {
            while (!(IsOccupied(i) and !IsShifted(i))) {
                i = succ(i);
            }
            std::queue<uint64_t> qs;
            uint64_t f;
            do {
                if (IsOccupied(i)) {
                    qs.push(i);
                }
                if (!IsContinuation(i)) {
                    assert(!qs.empty());
                    f = qs.front();
                    qs.pop();
                }
                uint64_t x = (Quotient(i) << bucket_bits_) | f;
                x = hasher_.ihash(x);
                next.set(x, values_[i]);
                cnt++;
                i = succ(i);
            } while (IsShifted(i));
        }
        *this = next;
    }
    
    size_t size() const {return size_;}
    size_t bucket() const {return bucket_size_;}

};

}

#endif //SIDS_INCLUDE_SIM_DS_CHT_H_
