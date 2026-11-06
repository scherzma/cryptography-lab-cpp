//
// Created by User on 2024-12-11.
//

#ifndef POLYNOM_H
#define POLYNOM_H


#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>

#include "Element.h"



#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

class Polynom {
public:
    std::vector<Element> coefficients;

    Polynom();
    ~Polynom() = default;

    Polynom(std::vector<Element>& coefficients) {
        this->coefficients = coefficients;
    }

    Polynom(std::vector<Element>&& coefficients) {
        this->coefficients = std::move(coefficients);
    }

    Polynom(std::vector<std::string> coefficients);
    Polynom(std::vector<std::array<uint8_t, 24>> coefficients);

    bool operator<(const Polynom& other) const;
    bool operator!=(const Polynom& other) const;
    bool operator==(const Polynom& other) const;

    Polynom operator+(const Polynom& other) const;
    Polynom operator*(const Polynom& other) const;
    Polynom operator^(int exponent) const;
    Polynom operator/(const Polynom& divisor) const;
    Polynom operator%(const Polynom& divisor) const;
    Polynom& operator*=(const Polynom& other);
    Polynom& operator+=(const Polynom& other);

    void monic();
    [[nodiscard]] Polynom diff() const;
    [[nodiscard]] bool is_zero() const;
    [[nodiscard]] int degree() const;
    [[nodiscard]] int power() const;
    [[nodiscard]] std::vector<std::string> repr() const;

    [[nodiscard]] Polynom gcd(const Polynom& other) const;
    [[nodiscard]] Polynom powermod(const Polynom& M, int k) const;
    [[nodiscard]] Polynom sqrt() const;
    [[nodiscard]] std::pair<Polynom, Polynom> divide(const Polynom& divisor) const;

    static Polynom random(int degree);

    inline void reduce() {
        const size_t size = coefficients.size();

        if (unlikely(size == 0)) {
            coefficients = {Element::ZERO};
            return;
        }

        const Element* ptr = coefficients.data() + size - 1;
        size_t new_size = size;

        while (ptr->is_zero() && likely(new_size > 1)) { // The expect literally doesn't do anything
            --new_size;
            --ptr;
        }

        if (new_size != size) {
            coefficients.resize(new_size);
        }
    }


    static void sort(std::vector<Polynom>& polys) {
        std::vector<size_t> indices(polys.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::sort(indices.begin(), indices.end(),
            [&polys](size_t a, size_t b) { return polys[a] < polys[b]; });

        std::vector<Polynom> temp(polys.size());
        for (size_t i = 0; i < indices.size(); i++) {
            temp[i] = std::move(polys[indices[i]]);
        }
        polys = std::move(temp);
    }
};



#endif //POLYNOM_H
