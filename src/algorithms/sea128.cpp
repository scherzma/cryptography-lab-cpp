// sea128.cpp
#include "sea128.h"

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <wmmintrin.h>

// AES-128 key expansion
inline __m128i AES_128_ASSIST (__m128i temp1, __m128i temp2) {
    __m128i temp3;
    temp2 = _mm_shuffle_epi32 (temp2 ,0xff);
    temp3 = _mm_slli_si128 (temp1, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp3 = _mm_slli_si128 (temp3, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp3 = _mm_slli_si128 (temp3, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp1 = _mm_xor_si128 (temp1, temp2);
    return temp1;
}


static inline void AES_128_Key_Expansion(const uint8_t *userkey, uint8_t *key) {
    __m128i temp1, temp2;
    __m128i *Key_Schedule = (__m128i*)key;
    temp1 = _mm_loadu_si128((__m128i*)userkey);
    Key_Schedule[0] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1 ,0x1);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[1] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x2);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[2] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x4);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[3] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x8);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[4] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x10);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[5] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x20);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[6] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x40);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[7] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x80);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[8] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x1b);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[9] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x36);
    temp1 = AES_128_ASSIST(temp1, temp2); Key_Schedule[10] = temp1;
}

static inline void AES_ECB_encrypt(const uint8_t *in, uint8_t *out, size_t length,
                                   const uint8_t *key, int rounds) {
    for (size_t i = 0; i < length; i += 16) {
        __m128i tmp = _mm_loadu_si128((__m128i*)(in + i));
        tmp = _mm_xor_si128 (tmp, ((__m128i*)key)[0]);
        for (int j = 1; j < rounds; j++) {
            tmp = _mm_aesenc_si128(tmp, ((__m128i*)key)[j]);
        }
        tmp = _mm_aesenclast_si128(tmp, ((__m128i*)key)[rounds]);
        _mm_storeu_si128((__m128i*)(out + i), tmp);
    }
}

static inline void AES_ECB_decrypt(const uint8_t *in, uint8_t *out, size_t length,
                                   const uint8_t *key, int rounds) {
    // To decrypt, we need the inverse key schedule.
    // For simplicity, we derive it here from the encryption schedule:
    __m128i decrypt_schedule[11];
    decrypt_schedule[0] = ((__m128i*)key)[rounds];
    for (int i = 1; i < rounds; i++) {
        decrypt_schedule[i] = _mm_aesimc_si128(((__m128i*)key)[rounds - i]);
    }
    decrypt_schedule[rounds] = ((__m128i*)key)[0];

    for (size_t i = 0; i < length; i += 16) {
        __m128i tmp = _mm_loadu_si128((__m128i*)(in + i));
        tmp = _mm_xor_si128(tmp, decrypt_schedule[0]);
        for (int j = 1; j < rounds; j++) {
            tmp = _mm_aesdec_si128(tmp, decrypt_schedule[j]);
        }
        tmp = _mm_aesdeclast_si128(tmp, decrypt_schedule[rounds]);
        _mm_storeu_si128((__m128i*)(out + i), tmp);
    }
}

__m128i aes_encrypt(__m128i key, __m128i block) {
    alignas(16) uint8_t key_bytes[16];
    _mm_storeu_si128((__m128i*)key_bytes, key);

    alignas(16) uint8_t expanded_key[176]; // 11 round keys * 16 bytes
    AES_128_Key_Expansion(key_bytes, expanded_key);
    __m128i* ks = (__m128i*)expanded_key;

    __m128i tmp = _mm_xor_si128(block, ks[0]);
    for (int i = 1; i < 10; i++) {
        tmp = _mm_aesenc_si128(tmp, ks[i]);
    }
    tmp = _mm_aesenclast_si128(tmp, ks[10]);
    return tmp;
}

// Single-block AES-128 decrypt with AES-NI
__m128i aes_decrypt(__m128i key, __m128i block) {
    alignas(16) uint8_t key_bytes[16];
    _mm_storeu_si128((__m128i*)key_bytes, key);

    alignas(16) uint8_t expanded_key[176];
    AES_128_Key_Expansion(key_bytes, expanded_key);
    __m128i* ks = (__m128i*)expanded_key;

    // Build decryption schedule
    __m128i decrypt_schedule[11];
    decrypt_schedule[0] = ks[10];
    for (int i = 1; i < 10; i++) {
        decrypt_schedule[i] = _mm_aesimc_si128(ks[10 - i]);
    }
    decrypt_schedule[10] = ks[0];

    __m128i tmp = _mm_xor_si128(block, decrypt_schedule[0]);
    for (int i = 1; i < 10; i++) {
        tmp = _mm_aesdec_si128(tmp, decrypt_schedule[i]);
    }
    tmp = _mm_aesdeclast_si128(tmp, decrypt_schedule[10]);
    return tmp;
}

// AES-128 encrypt function
std::vector<uint8_t> aes_encrypt_v(const std::array<uint8_t, 16>& key, const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output(input.size());
    alignas(16) uint8_t expanded_key[176]; // 11 round keys * 16 bytes = 176 bytes for AES-128
    AES_128_Key_Expansion(key.data(), expanded_key);

    AES_ECB_encrypt(input.data(), output.data(), input.size(), expanded_key, 10);
    return output;
}

// AES-128 decrypt function
std::vector<uint8_t> aes_decrypt_v(const std::array<uint8_t, 16>& key, const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output(input.size());
    alignas(16) uint8_t expanded_key[176];
    AES_128_Key_Expansion(key.data(), expanded_key);

    AES_ECB_decrypt(input.data(), output.data(), input.size(), expanded_key, 10);
    return output;
}


//https://www.intel.com/content/dam/doc/white-paper/advanced-encryption-standard-new-instructions-set-paper.pdf
constexpr static uint8_t SEA_CONST[] = {
    0xc0, 0xff, 0xee, 0xc0, 0xff, 0xee, 0xc0, 0xff,
    0xee, 0xc0, 0xff, 0xee, 0xc0, 0xff, 0xee, 0x11
};

std::vector<uint8_t> encrypt_sea(const std::array<uint8_t, 16>& key, const std::vector<uint8_t>& input){
    std::vector<uint8_t> encrypted = aes_encrypt_v(key, input);
    for(size_t i = 0; i < encrypted.size(); ++i){
        encrypted[i] ^= SEA_CONST[i];
    }
    return encrypted;
}

std::vector<uint8_t> decrypt_sea(const std::array<uint8_t, 16>& key, const std::vector<uint8_t>& input){
    std::vector<uint8_t> decrypted(input.size());
    for(size_t i = 0; i < input.size(); ++i){
        decrypted[i] = input[i] ^ SEA_CONST[i];
    }
    return aes_decrypt_v(key, decrypted);
}
