//
// Created by User on 2024-12-11.
//

#include "Element.h"

#include <random>
#include <unordered_map>

#include "util/base64_utils.h"

#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

Element::Element(__m128i value) : value(value) {}
Element::Element(Block value) : value(value.get_value()) {}



inline size_t m128i_hash(const __m128i& v) {
    uint64_t lower = _mm_extract_epi64(v, 0);
    uint64_t higher = _mm_extract_epi64(v, 1);
    return static_cast<size_t>(higher ^ lower);
}

inline bool m128i_equal(const __m128i& a, const __m128i& b) {
    __m128i cmp = _mm_cmpeq_epi64(a, b);
    return _mm_movemask_pd(_mm_castsi128_pd(cmp)) == 0x3;
}

static std::unordered_map<__m128i, __m128i, decltype(&m128i_hash), decltype(&m128i_equal)> inverse_cache(50, m128i_hash, m128i_equal);



const Element Element::ONE = Element(_mm_set_epi64x(0x8000000000000000LL, 0));
const Element Element::ZERO = Element(_mm_setzero_si128());


bool Element::operator!=(const Element& other) const {
    return !(*this == other);
}

bool Element::operator==(const Element& other) const {
    return _mm_test_all_ones(_mm_cmpeq_epi8(this->value, other.value));
}

bool Element::operator==(const __m128i& other) const {
    return _mm_test_all_ones(_mm_cmpeq_epi8(this->value, other));
}


bool Element::operator<(const Element& other) const {  // Schon bad.
    if (*this == other) {
        return false;
    }

    __uint128_t a = m128i_to_int(this->value);
    __uint128_t b = m128i_to_int(other.value);

    __uint128_t diff = a ^ b;
    unsigned long long lower = diff & 0xFFFFFFFFFFFFFFFF;

    int ind = lower ? __builtin_ctzll(lower) :
              __builtin_ctzll(static_cast<unsigned long long>(diff >> 64)) + 64;

    return !(a & ((__uint128_t)1 << ind));
}





Element Element::operator+(const Element& other) const {
    return {_mm_xor_si128(this->value, other.value)};
}

Element& Element::operator+=(const Element& other) {
    this->value = _mm_xor_si128(this->value, other.value);
    return *this;
}


Element Element::power_sqrt() const {
    __m128i result = this->value;
    for(int i=0; i<127; ++i)
        result = gfmul_m128i(result, result);
    return {result};
}


Element Element::sqrt() const {
    if (unlikely(this->is_zero())) {
        return *this;
    }
    return this->power_sqrt();
}


inline Element inv_pow(Element base_fe) {
    __m128i base = base_fe.value;
    __m128i result = base_fe.value;

    for(int i=0; i<127; ++i){
        result = gfmul_m128i(result, base);
        result = gfmul_m128i(result, result);
    }
    result = gfmul_m128i(result, result);

    return {result};
}


Element Element::inverse() const {
    if (likely(this->value == Element::ONE)) {
        return *this;
    }
    // return inv_pow(*this).value;

    auto it = inverse_cache.find(this->value);
    if (it != inverse_cache.end()) {
        return it->second;
    }

    __m128i result = inv_pow(*this).value;
    inverse_cache[this->value] = result;

    return result;
}

Element Element::operator/(const Element& other) const { // TODO: Implement this with extended euclidean algorithm
    return *this * other.inverse();
}

bool Element::is_zero() const {
    return _mm_test_all_ones(_mm_cmpeq_epi8(this->value, _mm_setzero_si128()));
}


std::string Element::to_block() const {
    return base64_encode_16(m128i_to_bytes(this->value));
}

Element Element::random() {
    Element result;
    //thread_local std::mt19937 generator(std::random_device{}());
    //std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    //int64_t random = distribution(generator);

    result.value = _mm_set_epi64x(rand(), 0);
    return result;
}

