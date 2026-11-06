//
// Created by user on 26/12/24.
//

#include "action_parsers.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <charconv>

#include "base64_utils.h"

size_t find_next_uint8_t(const std::vector<uint8_t>& input_str, size_t start, uint8_t target) {
    auto it = std::find(input_str.begin() + start, input_str.end(), target);
    return std::distance(input_str.begin(), it);
}

template<typename T>
T extract_between_delimiters(const std::vector<uint8_t>& input_str, size_t& start, uint8_t delimiter) {
    size_t start_pos = find_next_uint8_t(input_str, start, delimiter);
    size_t end_pos = find_next_uint8_t(input_str, start_pos + 1, delimiter);
    start = end_pos + 1;
    return T(input_str.begin() + start_pos + 1, input_str.begin() + end_pos);
}

uint8_t get_next_string_uint8_t(const std::vector<uint8_t>& input_str, size_t& start) {
    uint8_t delimiter = '"';
    size_t start_pos = find_next_uint8_t(input_str, start, delimiter);
    size_t end_pos = find_next_uint8_t(input_str, start_pos + 1, delimiter);
    start = end_pos + 1;
    return input_str[start_pos + 1];
}

std::string get_next_string(const std::vector<uint8_t>& input_str, size_t& start) {
    return extract_between_delimiters<std::string>(input_str, start, '"');
}

std::vector<uint8_t> get_next_vector(const std::vector<uint8_t>& input_str, size_t& start) {
    return extract_between_delimiters<std::vector<uint8_t>>(input_str, start, '"');
}

int get_next_int_value(const std::vector<uint8_t>& input_str, size_t& start) {
    auto it = std::find_if(input_str.begin() + start, input_str.end(), [](uint8_t c){
        return std::isdigit(c) || c == '-';
    });
    start = std::distance(input_str.begin(), it);

    std::string_view str_view(reinterpret_cast<const char*>(input_str.data() + start), input_str.size() - start);

    int value;
    auto [ptr, _] = std::from_chars(str_view.data(), str_view.data() + str_view.size(), value);

    start = std::distance(input_str.data(), reinterpret_cast<const uint8_t*>(ptr));
    return value;
}

Semantic get_next_semantic(const std::vector<uint8_t>& input_str, size_t& start) {
    start = find_next_uint8_t(input_str, start, '"') + 1;

    Semantic semantic = from_char(input_str[start]);

    start = find_next_uint8_t(input_str, start + 1, '"') + 1;
    return semantic;
}

std::vector<uint8_t> get_next_coefficients(const std::vector<uint8_t>& input_str, size_t& start) {
    std::vector<uint8_t> coefficients;

    start = find_next_uint8_t(input_str, start, '[') + 1;

    size_t close_bracket_pos = find_next_uint8_t(input_str, start, ']');

    while (start < close_bracket_pos) {
        int value = get_next_int_value(input_str, start);
        coefficients.push_back(static_cast<uint8_t>(value));

        size_t comma_pos = find_next_uint8_t(input_str, start, ',');
        if (comma_pos > close_bracket_pos) break;
        start = comma_pos + 1;
    }
    ++start;
    return coefficients;
}


std::array<uint8_t, 24> get_next_block(const std::vector<uint8_t>& input_str, size_t& start) {
    std::array<uint8_t, 24> block{};

    size_t start_quote_pos = find_next_uint8_t(input_str, start, '"');
    size_t current_pos = start_quote_pos + 1;

    for (size_t i = 0; i < 24; ++i) {
        block[i] = static_cast<uint8_t>(input_str[current_pos]);
        current_pos++;
    }

    start = current_pos + 1;
    return block;
}



std::vector<std::array<uint8_t, 24>> get_next_blocks(const std::vector<uint8_t>& input_str, size_t& start) {
    std::vector<std::array<uint8_t, 24>> blocks;

    start = find_next_uint8_t(input_str, start, '[') + 1;
    size_t closing_bracket_pos = find_next_uint8_t(input_str, start, ']');


    while (start + 25 < closing_bracket_pos) {
        std::array<uint8_t, 24> block = get_next_block(input_str, start);
        blocks.push_back(block);
        ++start;
    }

    start = closing_bracket_pos + 1;
    return blocks;
}


std::vector<std::vector<std::array<uint8_t, 24>>>
parse_array_of_blocks(const std::vector<uint8_t>& input_str, size_t& start) {
    std::vector<std::vector<std::array<uint8_t, 24>>> array_of_blocks;

    start = find_next_uint8_t(input_str, start, '[');
    ++start;

    while (start < input_str.size())
    {
        array_of_blocks.push_back(get_next_blocks(input_str, start));
    }

    array_of_blocks.resize(array_of_blocks.size() - 1);
    return array_of_blocks;
}



add_numbers_struct parse_add_numbers_input(const std::vector<uint8_t>& input_str, size_t& start) {
    std::string key1 = get_next_string(input_str, start);
    int value1 = get_next_int_value(input_str, start);
    std::string key2 = get_next_string(input_str, start);
    int value2 = get_next_int_value(input_str, start);
    return {value1, value2};
}

subtract_numbers_struct parse_subtract_numbers_input(const std::vector<uint8_t>& input_str, size_t& start) {
    std::string key1 = get_next_string(input_str, start);
    int value1 = get_next_int_value(input_str, start);
    std::string key2 = get_next_string(input_str, start);
    int value2 = get_next_int_value(input_str, start);
    return {value1, value2};
}

poly2block_struct parse_poly2block_input(const std::vector<uint8_t>& input_str, size_t& start) {
    Semantic semantic;
    std::vector<uint8_t> coefficients;

    for (int i = 0; i < 2; ++i) {
        uint8_t key = get_next_string_uint8_t(input_str, start);
        if (key == 's') {
            semantic = get_next_semantic(input_str, start);
        } else {
            coefficients = get_next_coefficients(input_str, start);
        }
    }

    return {semantic, coefficients};
}

block2poly_struct parse_block2poly_input(const std::vector<uint8_t>& input_str, size_t& start) {
    Semantic semantic;
    std::array<uint8_t, 24> block;

    for (int i = 0; i < 2; ++i) {
        uint8_t key = get_next_string_uint8_t(input_str, start);
        if (key == 's') {
            semantic = get_next_semantic(input_str, start);
        } else {
            block = get_next_block(input_str, start);
        }
    }

    return {semantic, block};
}

gf_struct parse_gf_input(const std::vector<uint8_t>& input_str, size_t& start) {
    Semantic semantic;
    std::array<uint8_t, 24> a;
    std::array<uint8_t, 24> b;

    for (int i = 0; i < 3; ++i) {
        uint8_t key = get_next_string_uint8_t(input_str, start);
        if (key == 's') {
            semantic = get_next_semantic(input_str, start);
        } else if (key == 'a') {
            a = get_next_block(input_str, start);
        } else if (key == 'b') {
            b = get_next_block(input_str, start);
        }
    }

    return {semantic, a, b};
}

sea128_struct parse_sea128_input(const std::vector<uint8_t>& input_str, size_t& start) {
    bool encrypt;
    std::array<uint8_t, 24> sea_key;
    std::vector<uint8_t> input;

    for (int i = 0; i < 3; ++i) {
        uint8_t key = get_next_string_uint8_t(input_str, start);
        if (key == 'm') {
            encrypt = get_next_string_uint8_t(input_str, start) == 'e';
        } else if (key == 'k') {
            sea_key = get_next_block(input_str, start);
        } else if (key == 'i') {
            input = get_next_vector(input_str, start);
        }
    }

    return {encrypt, sea_key, input};
}


xex_struct parse_xex_input(const std::vector<uint8_t>& input_str, size_t& start) {
    bool encrypt;
    std::vector<uint8_t> xex_key;
    std::vector<uint8_t> tweak;
    std::vector<uint8_t> input;

    for (int i = 0; i < 4; ++i) {
        uint8_t key = get_next_string_uint8_t(input_str, start);
        if (start > input_str.size()) {
            break;
        }
        if (key == 'm') {
            encrypt = get_next_string_uint8_t(input_str, start) == 'e';
        } else if (key == 'k') {
            xex_key = get_next_vector(input_str, start);
        } else if (key == 't') {
            tweak = get_next_vector(input_str, start);
        } else if (key == 'i') {
            input = get_next_vector(input_str, start);
        }
    }


    return {encrypt, xex_key, tweak, input};
}

gcm_crypt_struct parse_gcm_crypt_input(const std::vector<uint8_t>& input_str, size_t& start) {
    bool encrypt;
    bool aes;
    std::array<uint8_t, 24> gcm_key;
    std::array<uint8_t, 24> tag;
    std::vector<uint8_t> nonce;
    std::vector<uint8_t> ad;
    std::vector<uint8_t> text;

    for (int i = 0; i < 6; ++i) {
        std::string key = get_next_string(input_str, start);
        if (start > input_str.size()) {
            break;
        }
        if (key == "algorithm") {
            aes = get_next_string_uint8_t(input_str, start) == 'a';
        } else if (key == "nonce") {
            nonce = get_next_vector(input_str, start);
        } else if (key == "key") {
            gcm_key = get_next_block(input_str, start);
        } else if (key == "plaintext") {
            encrypt = true;
            ++i;
            text = get_next_vector(input_str, start);
        } else if (key == "ciphertext") {
            encrypt = false;
            text = get_next_vector(input_str, start);
        } else if (key == "ad") {
            ad = get_next_vector(input_str, start);
        } else if (key == "tag") {
            tag = get_next_block(input_str, start);
        }
    }

    if (false) {
        for (int i = 0; i < 24; ++i) {
            std::cout << static_cast<uint8_t>(gcm_key[i]);
        }
        std::cout << std::endl;

        for (int i = 0; i < ad.size(); ++i) {
            std::cout << static_cast<uint8_t>(ad[i]);
        }
        std::cout << std::endl;

        for (int i = 0; i < text.size(); ++i) {
            std::cout << static_cast<uint8_t>(text[i]);
        }
        std::cout << std::endl;

        for (int i = 0; i < nonce.size(); ++i) {
            std::cout << static_cast<uint8_t>(nonce[i]);
        }
        std::cout << std::endl;
    }
    return {encrypt, aes, gcm_key, tag, nonce, ad, text};
}

padding_oracle_struct parse_padding_oracle_input(const std::vector<uint8_t>& input_str, size_t& start) {
    int port;
    std::vector<uint8_t> hostname;
    std::vector<uint8_t> ciphertext;
    std::array<uint8_t, 24> iv;

    for (int i = 0; i < 4; ++i) {
        uint8_t key = get_next_string_uint8_t(input_str, start);
        if (key == 'h') {
            hostname = get_next_vector(input_str, start);
        } else if (key == 'p') {
            port = get_next_int_value(input_str, start);
        } else if (key == 'i') {
            iv = get_next_block(input_str, start);
        } else if (key == 'c') {
            ciphertext = get_next_vector(input_str, start);
        }
    }

    if (false) {
        for (int i = 0; i < iv.size(); ++i) {
            std::cout << static_cast<uint8_t>(iv[i]);
        }
        std::cout << std::endl;

        for (int i = 0; i < ciphertext.size(); ++i) {
            std::cout << static_cast<uint8_t>(ciphertext[i]);
        }
        std::cout << std::endl;
    }
    return {port, hostname, ciphertext, iv};
}


poly_op_struct parse_poly_op_input(const std::vector<uint8_t>& input_str, size_t& start) {
    int k;
    std::vector<std::array<uint8_t, 24>> A;
    std::vector<std::array<uint8_t, 24>> B;

    for (int i = 0; i < 3; ++i) {
        uint8_t key = get_next_string_uint8_t(input_str, start);
        if (start > input_str.size()) {
            break;
        }
        switch (key) {
            case 'A':
            case 'F':
            case 'Q':
                A = get_next_blocks(input_str, start);
            break;
            case 'B':
            case 'M':
                B = get_next_blocks(input_str, start);
            break;
            case 'k':
                k = get_next_int_value(input_str, start);
            break;
        }
    }

    if (false) {
        for (std::array<uint8_t, 24>& block : A) {
            for (int i = 0; i < 24; ++i) {
                std::cout << static_cast<uint8_t>(block[i]);
            }
            std::cout << std::endl;
        }
    }
    return {A, B, k};
}


poly_sort_struct parse_poly_sort_input(const std::vector<uint8_t>& input_str, size_t& start) {
    std::vector<std::vector<std::array<uint8_t, 24>>> polys;

    polys = parse_array_of_blocks(input_str, start);
    ++start;

    if (false) {
        for (std::vector<std::array<uint8_t, 24>>& poly : polys) {
            std::cout << "," << std::endl;
            for (std::array<uint8_t, 24>& block : poly) {
                for (int i = 0; i < 24; ++i) {
                    std::cout << static_cast<uint8_t>(block[i]);
                }
                std::cout << std::endl;
            }
        }
    }
    return {polys};
}

poly_factor_struct parse_poly_factor_input(const std::vector<uint8_t>& input_str, size_t& start) {
    std::vector<std::array<uint8_t, 24>> F;
    int d;

    for (int i = 0; i < 2; ++i) {
        uint8_t key = get_next_string_uint8_t(input_str, start);
        if (start > input_str.size()) {
            break;
        }
        if (key == 'F') {
            F = get_next_blocks(input_str, start);
        } else if (key == 'd') {
            d = get_next_int_value(input_str, start);
        }
    }
    return {F, d};
}


gcm_crack_struct parse_gcm_crack_input(const std::vector<uint8_t>& input_str, size_t& start) {
    GCMMessage m1;
    GCMMessage m2;
    GCMMessage m3;
    GCMMessage forgery;

    get_next_string_uint8_t(input_str, start);
    get_next_string_uint8_t(input_str, start);
    ++start;
    uint8_t key = get_next_string_uint8_t(input_str, start);
    ++start;


    std::array<std::reference_wrapper<GCMMessage>, 3> ms = {std::ref(m1), std::ref(m2), std::ref(m3)};
    int mcount = 0;
    for (int i = 0; i < 4; ++i) {

        if (key == 'm') {
            get_next_string_uint8_t(input_str, start);
            ms[mcount].get().ciphertext = base64_decode_vector(get_next_vector(input_str, start));
            get_next_string_uint8_t(input_str, start);
            ms[mcount].get().associated_data = base64_decode_vector(get_next_vector(input_str, start));
            get_next_string_uint8_t(input_str, start);
            ms[mcount].get().tag = base64_decode_vector(get_next_vector(input_str, start));
            ++start;
            ++mcount;
        } else {
            get_next_string_uint8_t(input_str, start);
            forgery.ciphertext = base64_decode_vector(get_next_vector(input_str, start));
            get_next_string_uint8_t(input_str, start);
            forgery.associated_data = base64_decode_vector(get_next_vector(input_str, start));
            ++start;
        }
        key = get_next_string_uint8_t(input_str, start);
    }


    if (false) {
        for (int i = 0; i < 24; ++i) {
            std::cout << static_cast<uint8_t>(m1.tag[i]);
        }
    }

    return {m1, m2, m3, forgery};
}
