// src/algorithms/gcm.cpp
#include "gcm.h"
#include "../util/semantic.h"
#include "../util/helper_functions.h"
#include "../math/Element.h"
#include "../algorithms/sea128.h"
#include <vector>
#include <cstdint>
#include <tuple>
#include <algorithm>

#include <functional>
#include <iostream>

std::vector<uint8_t> pad_array(const std::vector<uint8_t>& arr) {
    size_t final_size = arr.size() + ((16 - (arr.size() % 16)) % 16);
    std::vector<uint8_t> padded;
    padded.reserve(final_size);  // Reserve once
    padded.insert(padded.end(), arr.begin(), arr.end());
    padded.resize(final_size, 0);
    return padded;
}

std::array<uint8_t, 16> get_l(const std::vector<uint8_t>& ad, const std::vector<uint8_t>& ciphertext) {
    uint64_t ad_bit_len = static_cast<uint64_t>(ad.size()) * 8;
    uint64_t text_bit_len = static_cast<uint64_t>(ciphertext.size()) * 8;

    // Convert to big endian bytes
    std::array<uint8_t, 16> len_block;
    for(int i = 0; i < 8; ++i){
        len_block[15 - i] = (text_bit_len >> (i * 8)) & 0xFF;
        len_block[7 - i] = (ad_bit_len >> (i * 8)) & 0xFF;
    }

    return len_block;
}


std::tuple<std::array<uint8_t, 16>, std::array<uint8_t, 16>, std::array<uint8_t, 16>> ghash(
                                                const std::array<uint8_t, 16>& key,
                                                const std::vector<uint8_t>& nonce,
                                                const std::vector<uint8_t>& text,
                                                const std::vector<uint8_t>& ad,
                                                bool aes) {

    std::function<std::vector<uint8_t>(const std::array<uint8_t, 16>&, std::vector<uint8_t>&)> enc_func;
    if (aes) {
        enc_func = aes_encrypt_v;
    } else {
        enc_func = encrypt_sea;
    }


    std::array<uint8_t, 16> len_block = get_l(ad, text);

    std::vector<uint8_t> padded_ad = pad_array(ad);
    std::vector<uint8_t> padded_text = pad_array(text);

    std::vector<uint8_t> data;
    data.reserve(padded_ad.size() + padded_text.size() + len_block.size());
    data.insert(data.end(), padded_ad.begin(), padded_ad.end());
    data.insert(data.end(), padded_text.begin(), padded_text.end());
    data.insert(data.end(), len_block.begin(), len_block.end());


    std::vector<uint8_t> zero_block{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<uint8_t> encrypted_zero = enc_func(key, zero_block);
    __m128i auth_key = _mm_set_epi8( // Rip performance btw.
        encrypted_zero[0], encrypted_zero[1], encrypted_zero[2], encrypted_zero[3],
        encrypted_zero[4], encrypted_zero[5], encrypted_zero[6], encrypted_zero[7],
        encrypted_zero[8], encrypted_zero[9], encrypted_zero[10], encrypted_zero[11],
        encrypted_zero[12], encrypted_zero[13], encrypted_zero[14], encrypted_zero[15]
    );
    __m128i Y = _mm_setzero_si128();

    std::vector<__m128i> fe_blocks;
    fe_blocks.reserve(data.size() / 16);
    for(size_t i = 0; i < data.size(); i += 16) {
        std::vector<uint8_t> block(data.begin() + i, data.begin() + i + 16);
        __m128i block_fe = _mm_set_epi8( // Rip performance btw.
            block[0], block[1], block[2], block[3],
            block[4], block[5], block[6], block[7],
            block[8], block[9], block[10], block[11],
            block[12], block[13], block[14], block[15]
        );
        fe_blocks.push_back(block_fe);
    }

    for(const auto& block_fe : fe_blocks) {
        Y = _mm_xor_si128(Y, block_fe);
        Y = gfmul_m128i(Y, auth_key);
    }

    std::vector<uint8_t> ctr_block = nonce;
    ctr_block.insert(ctr_block.end(), {0, 0, 0, 1});

    std::vector<uint8_t> S = enc_func(key, ctr_block);
    __m128i S_m128 = _mm_set_epi8( // Rip performance btw.
            S[0], S[1], S[2], S[3],
            S[4], S[5], S[6], S[7],
            S[8], S[9], S[10], S[11],
            S[12], S[13], S[14], S[15]);
    __m128i tag = _mm_xor_si128(Y, S_m128);

    return {m128i_to_bytes(tag), len_block, m128i_to_bytes(auth_key)};
}

std::vector<uint8_t> crypt_gcm(
        const std::array<uint8_t, 16>& key,
        const std::vector<uint8_t>& text,
        const std::vector<uint8_t>& nonce,
        bool aes) {
    std::vector<uint8_t> result_text;
    result_text.reserve(text.size());

    std::function<std::vector<uint8_t> (const std::array<uint8_t, 16>&, const std::vector<uint8_t>&)> enc_func;
    enc_func = aes ? aes_encrypt_v : encrypt_sea;

    uint32_t counter = 2;
    std::vector<uint8_t> temp_nonce(16);

    for (size_t i = 0; i < text.size(); i += 16) {
        std::move(nonce.begin(), nonce.end(), temp_nonce.begin());
        temp_nonce[12] = (counter >> 24) & 0xFF;
        temp_nonce[13] = (counter >> 16) & 0xFF;
        temp_nonce[14] = (counter >> 8) & 0xFF;
        temp_nonce[15] = counter & 0xFF;
        std::vector<uint8_t> enc_ctr = enc_func(key, temp_nonce);

        std::vector<uint8_t> block(text.begin() + i, text.begin() + std::min(i + 16, text.size()));
        std::vector<uint8_t> encrypted = enc_func(key, block);

        for (size_t j = 0; j < encrypted.size(); ++j) {
            encrypted[j] = enc_ctr[j] ^ block[j];
        }
        result_text.insert(result_text.end(), encrypted.begin(), encrypted.end());
        counter++;
    }

    return result_text;
}


std::tuple<std::vector<uint8_t>, std::array<uint8_t, 16>, std::array<uint8_t, 16>, std::array<uint8_t, 16>> encrypt_gcm(
        const std::array<uint8_t, 16>& key,
        const std::vector<uint8_t>& plaintext,
        const std::vector<uint8_t>& ad,
        const std::vector<uint8_t>& nonce,
        bool aes) {

    auto ciphertext = crypt_gcm(key, plaintext, nonce, aes);
    auto [auth_tag, len_block, auth_key] = ghash(key, nonce, ciphertext, ad, aes);

    return {ciphertext, auth_tag, len_block, auth_key};
}


std::tuple<std::vector<uint8_t>, std::array<uint8_t, 16>, std::array<uint8_t, 16>, std::array<uint8_t, 16>> decrypt_gcm(
        const std::array<uint8_t, 16>& key,
        const std::vector<uint8_t>& ciphertext,
        const std::vector<uint8_t>& ad,
        const std::vector<uint8_t>& nonce,
        bool aes) {

    auto [auth_tag, len_block, auth_key] = ghash(key, nonce, ciphertext, ad, aes);
    auto plaintext = crypt_gcm(key, ciphertext, nonce, aes);

    return {plaintext, auth_tag, len_block, auth_key};
}
