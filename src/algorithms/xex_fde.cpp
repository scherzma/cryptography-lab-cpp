// src/algorithms/xex_fde.cpp
#include "xex_fde.h"

#include <emmintrin.h>
#include <functional>

#include "sea128.h"
#include <vector>
#include <string>

#include "util/helper_functions.h"

static inline __m128i mul_alpha(__m128i block) {
    // Extract 128-bit block into two 64-bit parts
    uint64_t low  = (uint64_t)_mm_cvtsi128_si64(block);
    uint64_t high = (uint64_t)_mm_extract_epi64(block, 1);

    // Carry is the top bit of 'high'
    uint64_t carry = high >> 63;

    // Shift entire 128 bits left by 1
    uint64_t new_low  = (low << 1);
    uint64_t new_high = (high << 1) | (low >> 63);

    // XOR with 0x87 if carry set
    new_low ^= carry * 0x87ULL;

    return _mm_set_epi64x((long long)new_high, (long long)new_low);
}


alignas(16) constexpr static uint8_t sea_const_bytes[16] = {
    0xc0, 0xff, 0xee, 0xc0, 0xff, 0xee, 0xc0, 0xff,
    0xee, 0xc0, 0xff, 0xee, 0xc0, 0xff, 0xee, 0x11
};
static const __m128i& sea_const_m = reinterpret_cast<const __m128i&>(sea_const_bytes);


std::vector<uint8_t> crypt_fde(const std::vector<uint8_t>& key, std::vector<uint8_t> tweak, const std::vector<uint8_t>& input, const std::string& mode){
    std::vector<uint8_t> k1(key.begin(), key.begin()+16);
    __m128i k1_m = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k1.data()));
    std::array<uint8_t, 16> k2;
    std::copy(key.begin()+16, key.end(), k2.begin());

    tweak = encrypt_sea(k2, tweak);
    __m128i tweak_m = _mm_loadu_si128(reinterpret_cast<const __m128i*>(tweak.data()));

    std::vector<__m128i> output_m;
    output_m.reserve(input.size() / 16);

    for(size_t i=0; i < (input.size() / 16); ++i){
        std::vector<uint8_t> block(input.begin()+(i*16), input.begin() + std::min((i+1)*16, input.size()));
        __m128i block_m = _mm_loadu_si128(reinterpret_cast<const __m128i*>(block.data()));
        block_m = _mm_xor_si128(block_m, tweak_m);

        if (mode == "encrypt") {
            block_m = aes_encrypt(k1_m, block_m);
            block_m = _mm_xor_si128(block_m, sea_const_m);
        } else {
            block_m = _mm_xor_si128(block_m, sea_const_m);
            block_m = aes_decrypt(k1_m, block_m);
        }

        block_m = _mm_xor_si128(block_m, tweak_m);
        output_m.emplace_back(block_m);
        tweak_m = mul_alpha(tweak_m);
    }

    std::vector<uint8_t> output_bytes;
    output_bytes.resize(input.size());

    for (size_t i = 0; i < output_m.size(); i++) {
        _mm_store_si128((__m128i*)&output_bytes[i * 16], output_m[i]);
    }
    return output_bytes;
}

std::vector<uint8_t> encrypt_fde(const std::vector<uint8_t>& key, const std::vector<uint8_t>& tweak, const std::vector<uint8_t>& input){
    std::vector<uint8_t> tweak_copy = tweak;
    return crypt_fde(key, tweak_copy, input, "encrypt");
}

std::vector<uint8_t> decrypt_fde(const std::vector<uint8_t>& key, const std::vector<uint8_t>& tweak, const std::vector<uint8_t>& input){
    std::vector<uint8_t> tweak_copy = tweak;
    return crypt_fde(key, tweak_copy, input, "decrypt");
}
