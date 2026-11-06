//
// Created by User on 2024-12-11.
//

#ifndef ELEMENT_H
#define ELEMENT_H
#include <emmintrin.h>

#include "util/Block.h"
#include "util/helper_functions.h"


class Element {
public:
    __m128i value;
    static const Element ONE;
    static const Element ZERO;

    Element() = default;
    Element(__m128i value);
    Element(Block value);
    ~Element() = default;

    bool operator<(const Element& other) const;
    bool operator!=(const Element& other) const;
    bool operator==(const Element& other) const;
    bool operator==(const __m128i& other) const;
    Element operator+(const Element& other) const;
    Element& operator+=(const Element& other);

    [[nodiscard]] Element inverse() const;
    [[nodiscard]] Element power_sqrt() const;
    [[nodiscard]] Element sqrt() const;
    Element operator/(const Element& other) const;

    [[nodiscard]] std::string to_block() const;
    [[nodiscard]] bool is_zero() const;
    static Element random();





    Element& operator*=(__m128i other) {
        this->value = gfmul_m128i(this->value, other);
        return *this;
    }

    Element operator*(const Element& other) const {
        return *this * other.value;
    }

    Element operator*(const __m128i& other) const {
        __m128i a = this->value;
        __m128i b = other;

        a = gfmul_m128i(a, b);

        return {a};
    }

    Element& operator*=(const Element& other) {
        return *this *= other.value;
    }

};



#endif //ELEMENT_H
