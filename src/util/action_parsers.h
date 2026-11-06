//
// Created by user on 26/12/24.
//

#ifndef ACTION_PARSERS_H
#define ACTION_PARSERS_H
#include <cstdint>
#include <vector>
#include <array>

#include "semantic.h"


struct add_numbers_struct {
    int number1;
    int number2;
};

struct subtract_numbers_struct {
    int number1;
    int number2;
};

struct poly2block_struct {
    Semantic semantic;
    std::vector<uint8_t> coefficients;
};

struct block2poly_struct {
    Semantic semantic;
    std::array<uint8_t, 24> block;
};

struct gf_struct {
    Semantic semantic;
    std::array<uint8_t, 24> a;
    std::array<uint8_t, 24> b;
};

struct sea128_struct {
    bool encrypt;
    std::array<uint8_t, 24> key;
    std::vector<uint8_t> input;
};

struct xex_struct {
    bool encrypt;
    std::vector<uint8_t> key;
    std::vector<uint8_t> tweak;
    std::vector<uint8_t> input;
};

struct gcm_crypt_struct {
    bool encrypt;
    bool aes;
    std::array<uint8_t, 24> key;
    std::array<uint8_t, 24> tag;
    std::vector<uint8_t> nonce;
    std::vector<uint8_t> ad;
    std::vector<uint8_t> text;
};

struct padding_oracle_struct {
    int port;
    std::vector<uint8_t> hostname;
    std::vector<uint8_t> ciphertext;
    std::array<uint8_t, 24> iv;
};


struct poly_op_struct {
    std::vector<std::array<uint8_t, 24>> A;
    std::vector<std::array<uint8_t, 24>> B;
    int k;
    std::vector<std::array<uint8_t, 24>> M;
};

struct poly_sort_struct {
    std::vector<std::vector<std::array<uint8_t, 24>>> polys;
};

struct poly_factor_struct {
    std::vector<std::array<uint8_t, 24>> F;
    int d;
};

struct gcm_crack_struct {
    GCMMessage m1;
    GCMMessage m2;
    GCMMessage m3;
    GCMMessage forgery;
};

add_numbers_struct parse_add_numbers_input(const std::vector<uint8_t>& input_str, size_t& start);
subtract_numbers_struct parse_subtract_numbers_input(const std::vector<uint8_t>& input_str, size_t& start);
poly2block_struct parse_poly2block_input(const std::vector<uint8_t>& input_str, size_t& start);
block2poly_struct parse_block2poly_input(const std::vector<uint8_t>& input_str, size_t& start);
gf_struct parse_gf_input(const std::vector<uint8_t>& input_str, size_t& start);
sea128_struct parse_sea128_input(const std::vector<uint8_t>& input_str, size_t& start);
xex_struct parse_xex_input(const std::vector<uint8_t>& input_str, size_t& start);
gcm_crypt_struct parse_gcm_crypt_input(const std::vector<uint8_t>& input_str, size_t& start);
padding_oracle_struct parse_padding_oracle_input(const std::vector<uint8_t>& input_str, size_t& start);
poly_op_struct parse_poly_op_input(const std::vector<uint8_t>& input_str, size_t& start);
poly_sort_struct parse_poly_sort_input(const std::vector<uint8_t>& input_str, size_t& start);
poly_factor_struct parse_poly_factor_input(const std::vector<uint8_t>& input_str, size_t& start);
gcm_crack_struct parse_gcm_crack_input(const std::vector<uint8_t>& input_str, size_t& start);

#endif //ACTION_PARSERS_H
