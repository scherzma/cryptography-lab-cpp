// sea128.h
#ifndef SEA128_H
#define SEA128_H

#include <vector>
#include <cstdint>
#include <emmintrin.h>
#include <array>

// Encrypt using SEA128
std::vector<uint8_t> encrypt_sea(const std::array<uint8_t, 16>& key, const std::vector<uint8_t>& input);

// Decrypt using SEA128
std::vector<uint8_t> decrypt_sea(const std::array<uint8_t, 16>& key, const std::vector<uint8_t>& input);

// AES Encrypt
std::vector<uint8_t> aes_encrypt_v(const std::array<uint8_t, 16>& key, const std::vector<uint8_t>& input);
__m128i aes_encrypt(__m128i key, __m128i block);
// AES Decrypt
std::vector<uint8_t> aes_decrypt_v(const std::array<uint8_t, 16>& key, const std::vector<uint8_t>& input);
__m128i aes_decrypt(__m128i key, __m128i block);

#endif // SEA128_H
