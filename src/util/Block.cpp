//
// Created by User on 2024-12-11.
//

#include <algorithm>

#include "Block.h"
#include "base64_utils.h"
#include "helper_functions.h"

Block::Block(__uint128_t value, Semantic sem) {
    this->value = represent_int(value, sem);
    this->sem = sem;
}

Block::Block(__m128i value, Semantic sem) {
    this->value = m128i_to_int(value);
    this->sem = sem;
}

__m128i Block::get_value() const {
    return int_to_m128i(value);
}


__uint128_t Block::get_int_value() const {
    return represent_int(value, sem);
}


std::string Block::get_block() const {
    const std::vector<uint8_t> bytes = uint128_to_bytes(this->get_int_value());
    return base64_encode(bytes);
}

std::vector<uint8_t> Block::get_poly() const {
    std::vector<uint8_t> poly;
    __uint128_t temp = value;
    uint8_t position = 0;
    while (temp != 0) {
        if (temp & 1)
            poly.push_back(127 - position);
        temp >>= 1;
        ++position;
    }
    std::reverse(poly.begin(), poly.end());
    return poly;
}


Block::Block(const std::vector<uint8_t>& value, Semantic sem) {
    __uint128_t aggregate = 0;
    __uint128_t one = 1;
    for(uint8_t x : value) {
        aggregate |= (one << 127 - x);
    }
    this->value = aggregate;
    this->sem = sem;
}


Block::Block(const std::string& block, const Semantic sem) {
    std::vector<uint8_t> bytes = base64_decode(block);
    std::reverse(bytes.begin(), bytes.end());
    __uint128_t temp = bytes_to_uint128(bytes);
    __uint128_t val = represent_int(temp, sem);
    this->value = val;
    this->sem = sem;
}


Block::Block(const std::array<uint8_t, 24>& block, const Semantic sem) {
    std::array<uint8_t, 16> bytes = base64_decode_16_reversed_array(block);
    __uint128_t temp = bytes_to_uint128(bytes);
    __uint128_t val = represent_int(temp, sem);
    this->value = val;
    this->sem = sem;
}
