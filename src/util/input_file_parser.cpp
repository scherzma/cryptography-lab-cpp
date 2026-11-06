//
// Created by user on 26/12/24.
//

#include "input_file_parser.h"

#include <iostream>
#include <stdexcept>

#include "action_parsers.h"
#include "OutputPrinter.h"
#include "process_functions.h"

uint8_t file_read_buffer[BUFFER_SIZE];

constexpr uint64_t hash_arr(std::array<char, 17> str, size_t n) {
    uint64_t h = str[1];
    h |= str[n-3] << 7;
    if (n < 9) return h;
    h |= str[7] << 14;
    h |= str[5] << 21;
    return h;
}

constexpr uint64_t hash(const char* str, size_t n) {
    uint64_t h = str[1];
    h |= str[n-3] << 7;
    if (n < 8) return h;
    h |= str[7] << 14;
    h |= str[5] << 21;
    return h;
}


constexpr uint64_t operator""_h(const char* str, size_t n) {
    return hash(str, n);
}


bool fill_buffer(FILE& file, size_t& pos, size_t& bytesRead) {
    bytesRead = fread(file_read_buffer, 1, BUFFER_SIZE, &file);
    pos = 0;
    return bytesRead > 0;
}

void process_block(std::vector<uint8_t>& content, OutputPrinter& output) {
    size_t start = 0;
    int count = 0;

    do {
        ++start;
        if (content[start] == '"') {
            ++count;
        }
    } while (count < 3);
    ++start;

    int action_begin = start;
     while (content[start] != '"') {
        ++start;
    }
    int action_end = start;

    uint64_t action_hash = content[action_begin+1];
    action_hash |= content[action_end-3] << 7;
    if (action_end - action_begin >= 9){
        action_hash |= content[action_begin+7] << 14;
        action_hash |= content[action_begin+5] << 21;
    }

    do {
        ++start;
    } while (content[start] != '{');

    output += "{";
    switch(action_hash) {
        case "add_numbers"_h:
            add_numbers(
                parse_add_numbers_input(content, start),
                output);
            break;
        case "subtract_numbers"_h:
            subtract_numbers(
                parse_add_numbers_input(content, start),
                output);
            break;
        case "poly2block"_h:
            poly2block(
                parse_poly2block_input(content, start),
                output);
            break;
        case "block2poly"_h:
            block2poly(
                parse_block2poly_input(content, start),
                output);
            break;
        case "gfmul"_h:
            gfmul(
                parse_gf_input(content, start),
                output);
            break;
        case "sea128"_h:
            sea128_operation(
                parse_sea128_input(content, start),
                output);
            break;
        case "xex"_h:
            xex_fde_operation(
                parse_xex_input(content, start),
                output);
            break;
        case "gcm_encrypt"_h:
            gcm_encrypt(
                parse_gcm_crypt_input(content, start),
                output);
            break;
        case "gcm_decrypt"_h:
            gcm_decrypt(
                parse_gcm_crypt_input(content, start),
                output);
            break;
        case "padding_oracle"_h:
            padding_oracle_attack(
                parse_padding_oracle_input(content, start),
                output);
            break;
        case "gfpoly_add"_h:
            polynomial_add(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfpoly_mul"_h:
            polynomial_mul(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfpoly_pow"_h:
            polynomial_pow(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfdiv"_h:
            gfdiv(
                parse_gf_input(content, start),
                output);
            break;
        case "gfpoly_divmod"_h:
            polynomial_divmod(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfpoly_powmod"_h:
            polynomial_powmod(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfpoly_sort"_h:
            polynomial_sort(
                parse_poly_sort_input(content, start),
                output);
            break;
        case "gfpoly_make_monic"_h:
            polynomial_make_monic(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfpoly_sqrt"_h:
            polynomial_sqrt(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfpoly_diff"_h:
            polynomial_diff(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfpoly_gcd"_h:
            polynomial_gcd(
                parse_poly_op_input(content, start),
                output);
            break;
        case "gfpoly_factor_sff"_h:
            polynomial_factor_sff(
                parse_poly_factor_input(content, start),
                output);
            break;
        case "gfpoly_factor_ddf"_h:
            polynomial_factor_ddf(
                parse_poly_factor_input(content, start),
                output);
            break;
        case "gfpoly_factor_edf"_h:
            polynomial_factor_edf(
                parse_poly_factor_input(content, start),
                output);
            break;
        case "gcm_crack"_h:
            gcm_crack_process(
                parse_gcm_crack_input(content, start),
                output);
            break;
        default:
            break;
    }
    output += "}";
}

void parse_input_file(const char* filename, OutputPrinter& output) {
    size_t pos = 0;
    size_t bytesRead = 0;
    int depth = 0;

    FILE* file;

    file = fopen(filename, "rb");

    std::vector<uint8_t> block_bytes;
    int startPos = 0;

    bool readId = false;
    std::array<uint8_t, 64> id{};
    size_t id_len = 0;

    output += "{\"responses\": {";

    while (fill_buffer(*file, pos, bytesRead)) {
        for (size_t i = pos; i < pos + bytesRead; ++i) {
            char chr = file_read_buffer[i];
            switch (chr) {
                case '{':
                    ++depth;
                    if (depth == 3) {
                        startPos = i;
                    }
                    break;
                case '}':
                    --depth;
                    if (depth == 2) {
                        size_t count = (i - startPos + 1);
                        block_bytes.reserve(block_bytes.size() + count);
                        block_bytes.insert(block_bytes.end(), &file_read_buffer[startPos], &file_read_buffer[i]);
                        process_block(block_bytes, output);
                        output += ",";
                        block_bytes.clear();
                        block_bytes.reserve(128);
                    } else if (depth < 2) {
                        output.set('}');
                        output += "}";
                        fclose(file);
                        return;
                    }
                    break;
                case '"':
                    if (depth == 2) {
                        if (!readId) {
                            readId = true;
                        } else {

                            output += "\"";
                            output.add(id, id_len);
                            output += "\": ";

                            id_len = 0;
                            readId = false;
                        }
                    }
                    break;
                default:
                    if (readId) {
                        id[id_len] = chr;
                        ++id_len;
                    }
                    break;
            }
        }

        if (depth != 2) {
            size_t a = startPos;
            size_t b = pos + bytesRead - 1;
            startPos = 0;
            size_t count = (b - a + 1);
            block_bytes.reserve(block_bytes.size() + count);
            block_bytes.insert(block_bytes.end(), &file_read_buffer[a], &file_read_buffer[b + 1]);
        }
    }

    output.set(']');
    output += "}";
    output.print();
}