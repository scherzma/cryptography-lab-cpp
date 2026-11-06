// src/algorithms/gcm.h
#ifndef GCM_H
#define GCM_H

#include <vector>
#include <string>
#include <cstdint>
#include <tuple>

#include "math/Element.h"




std::tuple<std::vector<uint8_t>, std::array<uint8_t, 16>, std::array<uint8_t, 16>, std::array<uint8_t, 16>> decrypt_gcm(
        const std::array<uint8_t, 16>& key,
        const std::vector<uint8_t>& ciphertext,
        const std::vector<uint8_t>& ad,
        const std::vector<uint8_t>& nonce,
        bool aes);

std::tuple<std::vector<uint8_t>, std::array<uint8_t, 16>, std::array<uint8_t, 16>, std::array<uint8_t, 16>> encrypt_gcm(
        const std::array<uint8_t, 16>& key,
        const std::vector<uint8_t>& plaintext,
        const std::vector<uint8_t>& ad,
        const std::vector<uint8_t>& nonce,
        bool aes);

std::vector<uint8_t> crypt_gcm(
        const std::array<uint8_t, 16>& key,
        const std::vector<uint8_t>& text,
        const std::vector<uint8_t>& nonce,
        bool aes);

std::tuple<std::array<uint8_t, 16>, std::array<uint8_t, 16>, std::array<uint8_t, 16>> ghash(
                                                const std::array<uint8_t, 16>& key,
                                                const std::vector<uint8_t>& nonce,
                                                const std::vector<uint8_t>& text,
                                                const std::vector<uint8_t>& ad,
                                                bool aes);


std::vector<uint8_t> pad_array(const std::vector<uint8_t>& arr);

std::array<uint8_t, 16> get_l(const std::vector<uint8_t>& ad, const std::vector<uint8_t>& ciphertext);

#endif // GCM_H
