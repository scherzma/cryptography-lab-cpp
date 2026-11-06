//
// Created by User on 2024-12-11.
//

#ifndef BLOCK_H
#define BLOCK_H
#include <cstdint>
#include <vector>
#include <immintrin.h>

#include "semantic.h"


class Block {
public:
    __uint128_t value;
    Semantic sem;
    Block() : value(0), sem(Semantic::GCM) {}

    Block(const std::string& value, Semantic sem);
    Block(const std::array<uint8_t, 24>& value, Semantic sem);
    Block(__m128i value, Semantic sem);
    Block(__uint128_t value, Semantic sem);
    Block(const std::vector<uint8_t>& block, Semantic sem);

    [[nodiscard]] __m128i get_value() const;
    [[nodiscard]] __uint128_t get_int_value() const;
    [[nodiscard]] std::string get_block() const;
    [[nodiscard]] std::vector<uint8_t> get_poly() const;

};



#endif //BLOCK_H
