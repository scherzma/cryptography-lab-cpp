//
// Created by User on 2024-12-11.
//

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H


#include "semantic.h"
#include <immintrin.h>
#include <vector>
#include <array>
#include <cstdint>

constexpr std::array<uint8_t, 256> reverse8_table = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};


inline std::tuple<__m128i, __m128i> gfmul_m128i(const __m128i& a, const __m128i& b, const __m128i& x, const __m128i& y) {
    // Carry out the multiplication algorithm
    __m128i tmp3 = _mm_clmulepi64_si128(a, b, 0x00);
    __m128i tmp3_a = _mm_clmulepi64_si128(x, y, 0x00);
    __m128i tmp4 = _mm_clmulepi64_si128(a, b, 0x10);
    __m128i tmp4_a = _mm_clmulepi64_si128(x, y, 0x10);
    __m128i tmp5 = _mm_clmulepi64_si128(a, b, 0x01);
    __m128i tmp5_a = _mm_clmulepi64_si128(x, y, 0x01);
    __m128i tmp6 = _mm_clmulepi64_si128(a, b, 0x11);
    __m128i tmp6_a = _mm_clmulepi64_si128(x, y, 0x11);

    tmp4 = _mm_xor_si128(tmp4, tmp5);
    tmp4_a = _mm_xor_si128(tmp4_a, tmp5_a);
    tmp5 = _mm_slli_si128(tmp4, 8);
    tmp5_a = _mm_slli_si128(tmp4_a, 8);
    tmp4 = _mm_srli_si128(tmp4, 8);
    tmp4_a = _mm_srli_si128(tmp4_a, 8);
    tmp3 = _mm_xor_si128(tmp3, tmp5);
    tmp3_a = _mm_xor_si128(tmp3_a, tmp5_a);
    tmp6 = _mm_xor_si128(tmp6, tmp4);
    tmp6_a = _mm_xor_si128(tmp6_a, tmp4_a);

    __m128i tmp7 = _mm_srli_epi32(tmp3, 31);
    __m128i tmp7_a = _mm_srli_epi32(tmp3_a, 31);
    __m128i tmp8 = _mm_srli_epi32(tmp6, 31);
    __m128i tmp8_a = _mm_srli_epi32(tmp6_a, 31);
    tmp3 = _mm_slli_epi32(tmp3, 1);
    tmp3_a = _mm_slli_epi32(tmp3_a, 1);
    tmp6 = _mm_slli_epi32(tmp6, 1);
    tmp6_a = _mm_slli_epi32(tmp6_a, 1);

    __m128i tmp9 = _mm_srli_si128(tmp7, 12);
    __m128i tmp9_a = _mm_srli_si128(tmp7_a, 12);
    tmp8 = _mm_slli_si128(tmp8, 4);
    tmp8_a = _mm_slli_si128(tmp8_a, 4);
    tmp7 = _mm_slli_si128(tmp7, 4);
    tmp7_a = _mm_slli_si128(tmp7_a, 4);
    tmp3 = _mm_or_si128(tmp3, tmp7);
    tmp3_a = _mm_or_si128(tmp3_a, tmp7_a);
    tmp6 = _mm_or_si128(tmp6, tmp8);
    tmp6_a = _mm_or_si128(tmp6_a, tmp8_a);
    tmp6 = _mm_or_si128(tmp6, tmp9);
    tmp6_a = _mm_or_si128(tmp6_a, tmp9_a);

    // Reduction steps
    tmp7 = _mm_slli_epi32(tmp3, 31);
    tmp7_a = _mm_slli_epi32(tmp3_a, 31);
    tmp8 = _mm_slli_epi32(tmp3, 30);
    tmp8_a = _mm_slli_epi32(tmp3_a, 30);
    tmp9 = _mm_slli_epi32(tmp3, 25);
    tmp9_a = _mm_slli_epi32(tmp3_a, 25);
    tmp7 = _mm_xor_si128(tmp7, tmp8);
    tmp7_a = _mm_xor_si128(tmp7_a, tmp8_a);
    tmp7 = _mm_xor_si128(tmp7, tmp9);
    tmp7_a = _mm_xor_si128(tmp7_a, tmp9_a);
    tmp8 = _mm_srli_si128(tmp7, 4);
    tmp8_a = _mm_srli_si128(tmp7_a, 4);
    tmp7 = _mm_slli_si128(tmp7, 12);
    tmp7_a = _mm_slli_si128(tmp7_a, 12);
    tmp3 = _mm_xor_si128(tmp3, tmp7);
    tmp3_a = _mm_xor_si128(tmp3_a, tmp7_a);

    __m128i tmp2 = _mm_srli_epi32(tmp3, 1);
    __m128i tmp2_a = _mm_srli_epi32(tmp3_a, 1);
    tmp4 = _mm_srli_epi32(tmp3, 2);
    tmp4_a = _mm_srli_epi32(tmp3_a, 2);
    tmp5 = _mm_srli_epi32(tmp3, 7);
    tmp5_a = _mm_srli_epi32(tmp3_a, 7);
    tmp2 = _mm_xor_si128(tmp2, tmp4);
    tmp2_a = _mm_xor_si128(tmp2_a, tmp4_a);
    tmp2 = _mm_xor_si128(tmp2, tmp5);
    tmp2_a = _mm_xor_si128(tmp2_a, tmp5_a);
    tmp2 = _mm_xor_si128(tmp2, tmp8);
    tmp2_a = _mm_xor_si128(tmp2_a, tmp8_a);
    tmp3 = _mm_xor_si128(tmp3, tmp2);
    tmp3_a = _mm_xor_si128(tmp3_a, tmp2_a);
    tmp6 = _mm_xor_si128(tmp6, tmp3);
    tmp6_a = _mm_xor_si128(tmp6_a, tmp3_a);

    return {tmp6, tmp6_a};
}

inline __m128i gfmul_m128i(__m128i& a, __m128i& b) {
    // Carry out the multiplication algorithm
    __m128i tmp3 = _mm_clmulepi64_si128(a, b, 0x00);
    __m128i tmp4 = _mm_clmulepi64_si128(a, b, 0x10);
    __m128i tmp5 = _mm_clmulepi64_si128(a, b, 0x01);
    __m128i tmp6 = _mm_clmulepi64_si128(a, b, 0x11);

    tmp4 = _mm_xor_si128(tmp4, tmp5);
    tmp5 = _mm_slli_si128(tmp4, 8);
    tmp4 = _mm_srli_si128(tmp4, 8);
    tmp3 = _mm_xor_si128(tmp3, tmp5);
    tmp6 = _mm_xor_si128(tmp6, tmp4);

    __m128i tmp7 = _mm_srli_epi32(tmp3, 31);
    __m128i tmp8 = _mm_srli_epi32(tmp6, 31);
    tmp3 = _mm_slli_epi32(tmp3, 1);
    tmp6 = _mm_slli_epi32(tmp6, 1);

    __m128i tmp9 = _mm_srli_si128(tmp7, 12);
    tmp8 = _mm_slli_si128(tmp8, 4);
    tmp7 = _mm_slli_si128(tmp7, 4);
    tmp3 = _mm_or_si128(tmp3, tmp7);
    tmp6 = _mm_or_si128(tmp6, tmp8);
    tmp6 = _mm_or_si128(tmp6, tmp9);

    // Reduction steps
    tmp7 = _mm_slli_epi32(tmp3, 31);
    tmp8 = _mm_slli_epi32(tmp3, 30);
    tmp9 = _mm_slli_epi32(tmp3, 25);
    tmp7 = _mm_xor_si128(tmp7, tmp8);
    tmp7 = _mm_xor_si128(tmp7, tmp9);
    tmp8 = _mm_srli_si128(tmp7, 4);
    tmp7 = _mm_slli_si128(tmp7, 12);
    tmp3 = _mm_xor_si128(tmp3, tmp7);

    __m128i tmp2 = _mm_srli_epi32(tmp3, 1);
    tmp4 = _mm_srli_epi32(tmp3, 2);
    tmp5 = _mm_srli_epi32(tmp3, 7);
    tmp2 = _mm_xor_si128(tmp2, tmp4);
    tmp2 = _mm_xor_si128(tmp2, tmp5);
    tmp2 = _mm_xor_si128(tmp2, tmp8);
    tmp3 = _mm_xor_si128(tmp3, tmp2);
    return _mm_xor_si128(tmp6, tmp3);
}

__uint128_t represent_int(const unsigned __int128 value, Semantic sem);
std::vector<uint8_t> uint128_to_bytes(const __uint128_t input);
std::array<uint8_t, 16> m128i_to_bytes(const __m128i input);
__uint128_t bytes_to_uint128(const std::vector<uint8_t>& bytes);
__uint128_t bytes_to_uint128(const std::array<uint8_t, 16>& bytes);
__uint128_t reverse_bits_in_each_byte(const __uint128_t value);
__m128i int_to_m128i(__uint128_t value);
__uint128_t m128i_to_int(__m128i value);
__uint128_t reverse_bits(unsigned __int128 n);



#endif //HELPER_FUNCTIONS_H
