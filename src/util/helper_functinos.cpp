//
// Created by User on 2024-12-11.
//


#include "helper_functions.h"
#include <algorithm>
#include <iostream>

__uint128_t bytes_to_uint128(const std::vector<uint8_t>& bytes) {
    __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(bytes.data()));

    uint64_t high = _mm_extract_epi64(data, 1);
    uint64_t low = _mm_extract_epi64(data, 0);

    return (static_cast<__uint128_t>(high) << 64) | low;
}

__uint128_t bytes_to_uint128(const std::array<uint8_t, 16>& bytes) {
    __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(bytes.data()));

    uint64_t high = _mm_extract_epi64(data, 1);
    uint64_t low = _mm_extract_epi64(data, 0);

    return (static_cast<__uint128_t>(high) << 64) | low;
}

std::vector<uint8_t> uint128_to_bytes(const __uint128_t input) {
    std::vector<uint8_t> output(16);
    __m128i simd_value = _mm_set_epi64x(static_cast<uint64_t>(input >> 64), static_cast<uint64_t>(input));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output.data()), simd_value);
    std::reverse(output.begin(), output.end());
    return output;
}

std::array<uint8_t, 16> m128i_to_bytes(const __m128i input) {
    std::array<uint8_t, 16> output;
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output.data()), input);
    std::reverse(output.begin(), output.end());
    return output;
}



__uint128_t represent_int(const unsigned __int128 value, const Semantic sem) {
    if(sem == Semantic::GCM) {
        return value;
    }
    return reverse_bits_in_each_byte(value);
}


__uint128_t m128i_to_int(__m128i value) {
    const uint64_t result_hi = _mm_extract_epi64(value, 1);
    const uint64_t result_lo = _mm_extract_epi64(value, 0);

    const __uint128_t result = (static_cast<__uint128_t>(result_hi) << 64) | result_lo;
    return result;
}

__m128i int_to_m128i(__uint128_t value) {
    uint64_t a_hi = static_cast<uint64_t>(value >> 64);
    uint64_t a_lo = static_cast<uint64_t>(value);
    __m128i a = _mm_set_epi64x(a_hi, a_lo);
    return a;
}





__uint128_t reverse_bits_in_each_byte(const __uint128_t value) {
    __uint128_t result = 0;
    for (int i = 0; i < 16; ++i) { // There are 16 bytes in 128 bits
        uint8_t byte = static_cast<uint8_t>((value >> (8 * i)) & 0xFF);
        uint8_t rev_byte = reverse8_table[byte];
        result |= static_cast<__uint128_t>(rev_byte) << (8 * i);
    }
    return result;
}


inline void print_uint128(__uint128_t value) {
    if (value == 0) {
        std::cout << 0;
        return;
    }

    char buffer[40] = {0};
    int pos = 39;
    while (value > 0) {
        buffer[--pos] = '0' + (value % 10);
        value /= 10;
    }

    std::cout << &buffer[pos] << std::endl;
}