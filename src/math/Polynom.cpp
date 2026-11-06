//
// Created by User on 2024-12-11.
//

#include "Polynom.h"

#include <iostream>
#include <ostream>

#include "util/base64_utils.h"

#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

Polynom::Polynom() {
    coefficients.reserve(8);
}

bool Polynom::operator!=(const Polynom& other) const {
    return !(*this == other);
}

bool Polynom::operator==(const Polynom& other) const {
    return coefficients == other.coefficients;
}

Polynom::Polynom(std::vector<std::string> coefficients) {
    std::vector<Element> result_coeffs;
    result_coeffs.reserve(coefficients.size());

    for (std::string coefficient : coefficients) {
        std::array<uint8_t, 16> S = base64_decode_16_reversed(coefficient);
        result_coeffs.emplace_back(
            _mm_loadu_si128(reinterpret_cast<const __m128i*>(S.data()))
        );
    }
    this->coefficients = std::move(result_coeffs);
}

Polynom::Polynom(std::vector<std::array<uint8_t, 24>> coefficients) {
    std::vector<Element> result_coeffs;
    result_coeffs.reserve(coefficients.size());

    for (std::array<uint8_t, 24> coefficient : coefficients) {
        std::array<uint8_t, 16> S = base64_decode_16_reversed_array(coefficient);
        result_coeffs.emplace_back(
            _mm_loadu_si128(reinterpret_cast<const __m128i*>(S.data()))
        );
    }
    this->coefficients = std::move(result_coeffs);
}



int Polynom::degree() const {
    return coefficients.size() - 1;
}

int Polynom::power() const {
    return coefficients.size();
}


bool Polynom::operator<(const Polynom& other) const {
    if(this->power() < other.power()){
        return true;
    }
    if(this->power() > other.power()){
        return false;
    }
    for(int i = degree(); i >= 0; --i){
        if(this->coefficients[i] < other.coefficients[i]){
            return true;
        }
        if(other.coefficients[i] < this->coefficients[i]){
            return false;
        }
    }
    return false;
}

Polynom Polynom::operator+(const Polynom& other) const {
    Polynom result = *this;
    result += other;
    return result;
}

Polynom& Polynom::operator+=(const Polynom& other) {

    size_t smaller = std::min(this->coefficients.size(), other.coefficients.size());
    size_t larger  = std::max(this->coefficients.size(), other.coefficients.size());

    std::vector<Element> result;
    result.reserve(larger);

    for (size_t i = 0; i < smaller; ++i) {
        result.emplace_back(this->coefficients[i] + other.coefficients[i]);
    }

    if (this->coefficients.size() > other.coefficients.size()) {
        result.insert(result.end(), this->coefficients.begin() + smaller, this->coefficients.end());
    } else if (other.coefficients.size() > this->coefficients.size()) {
        result.insert(result.end(), other.coefficients.begin() + smaller, other.coefficients.end());
    }

    this->coefficients = std::move(result);
    this->reduce();
    return *this;
}

Polynom Polynom::operator*(const Polynom& other) const {
    Polynom result = *this;
    result *= other;
    return result;
}

Polynom& Polynom::operator*=(const Polynom& other) {
    int t_deg = this->degree();
    int o_deg = other.degree();
    std::vector<Element> result_coeffs(t_deg + o_deg + 1);
    Element temp{};

    for (int i = 0; i <= t_deg; ++i) {
        for (int j = 0; j <= o_deg; ++j) {
            temp = this->coefficients[i];
            temp *= other.coefficients[j];
            result_coeffs[i + j] += temp;
        }
    }

    this->coefficients = std::move(result_coeffs);
    this->reduce();
    return *this;
}

Polynom Polynom::operator^(int exponent) const {
    if(exponent == 0){
        return Polynom({Element::ONE});
    }
    if(exponent == 1)
    {
        return *this;
    }
    Polynom result({Element::ONE});
    Polynom base = *this;
    while(exponent > 0){
        if(exponent & 1){
            result *= base;
        }
        base *= base;
        exponent >>= 1;
    }
    result.reduce();
    return result;
}


Polynom Polynom::sqrt() const {
    int m = this->power() >> 1;
    std::vector<Element> S_coeffs(m + 1);

    for (int i = 0; i <= m; ++i) {
        Element q_2i = coefficients[i << 1];
        S_coeffs[i] = q_2i.sqrt();
    }

    Polynom S(S_coeffs);
    S.reduce();
    return S;
}


Polynom Polynom::gcd(const Polynom& other) const {
    // Initialize with reduced copies
    Polynom a = *this;
    Polynom b = other;
    a.reduce();
    b.reduce();

    // Main GCD loop using divide() to get quotient and remainder together
    while (!b.is_zero()) {
        Polynom r = a % b;    // Remainder
        a.coefficients = std::move(b.coefficients);    // Move the internal vector
        b.coefficients = std::move(r.coefficients);    // Move the internal vector
    }

    a.monic();
    return a;
}

Polynom Polynom::operator/(const Polynom& divisor) const {
    Polynom dividend = *this;
    dividend.reduce();
    const Polynom& b = divisor;

    if (dividend.degree() < b.degree()) {
        return Polynom({Element::ZERO});
    }

    Element b_lead_inv = b.coefficients[b.degree()].inverse();
    std::vector<Element> quotient_coeffs(dividend.degree() - b.degree() + 1, Element::ZERO);

    while (dividend.degree() >= b.degree()) {
        int deg_diff = dividend.degree() - b.degree();
        Element quotient_coeff = dividend.coefficients[dividend.degree()] * b_lead_inv;
        quotient_coeffs[deg_diff] = quotient_coeff;

        // Subtract b * quotient_coeff * x^deg_diff from dividend
        for (int i = 0; i <= b.degree(); ++i) {
            int pos = deg_diff + i;
            dividend.coefficients[pos] += quotient_coeff * b.coefficients[i];
        }

        // Reduce the size of dividend
        dividend.reduce();

        if (dividend.is_zero()) {
            break;
        }
    }
    return Polynom(quotient_coeffs);
}

Polynom Polynom::operator%(const Polynom& divisor) const {
    Polynom remainder = *this;
    remainder.reduce();
    const Polynom& b = divisor;

    if (remainder.degree() < b.degree()) {
        return remainder;
    }

    Element b_lead_inv = b.coefficients[b.degree()].inverse();
    int b_power = b.power();
    int r_power = remainder.power();
    const Element* ptr = remainder.coefficients.data() - 1;


    while (r_power >= b_power) {
        int deg_diff = r_power - b_power;
        Element quotient_coeff = remainder.coefficients[r_power - 1] * b_lead_inv;

        for (int i = 0; i < b_power; ++i) {
            remainder.coefficients[deg_diff + i] += quotient_coeff * b.coefficients[i];
        }
        --r_power;

        while (unlikely((ptr + r_power)->is_zero()) && likely(r_power > 0)) {
            --r_power;
        }

        if (r_power == 0) {
            remainder.coefficients = {Element::ZERO};
            return remainder;
        }
    }
    remainder.coefficients.resize(r_power);
    return remainder;
}


std::pair<Polynom, Polynom> Polynom::divide(const Polynom& divisor) const {
    Polynom remainder = *this;
    remainder.reduce();
    const Polynom& b = divisor;

    if (remainder.degree() < b.degree()) {
        return {Polynom({Element::ZERO}), remainder};
    }

    Element b_lead_inv = b.coefficients[b.degree()].inverse();
    std::vector<Element> quotient_coeffs(remainder.degree() - b.degree() + 1, Element::ZERO);

    while (remainder.degree() >= b.degree()) {
        int deg_diff = remainder.degree() - b.degree();

        Element quotient_coeff = remainder.coefficients[remainder.degree()] * b_lead_inv;
        quotient_coeffs[deg_diff] = quotient_coeff;

        for (int i = 0; i <= b.degree(); ++i) {
            int pos = deg_diff + i;
            remainder.coefficients[pos] += quotient_coeff * b.coefficients[i];
        }

        int new_size = remainder.coefficients.size() - 1;
        while (new_size > 0 && remainder.coefficients[new_size].is_zero()) {
            new_size--;
        }
        remainder.coefficients.resize(new_size + 1);

        if (remainder.is_zero()) {
            break;
        }
    }
    return {Polynom(quotient_coeffs), remainder};
}


Polynom Polynom::powermod(const Polynom& M, int k) const {
    Polynom one_fe({Element::ONE});
    Polynom result = one_fe;

    Polynom base = *this % M;

    while (k > 0) {
        if (k & 1) {
            result *= base;
        }
        base *= base;
        base = base % M;
        k >>= 1;
    }

    result.reduce();
    return result;
}

Polynom Polynom::diff() const {
    std::vector<Element> derivative_coeffs;
    derivative_coeffs.reserve(this->degree());
    for(int i = 1; i < this->power(); ++i){
        if(i % 2 == 1) {
            derivative_coeffs.push_back(this->coefficients[i]);
        }
        else {
            derivative_coeffs.emplace_back(Element::ZERO);
        }
    }
    Polynom derivative(derivative_coeffs);
    derivative.reduce();
    return derivative;
}



bool Polynom::is_zero() const {
    return this->coefficients.size() == 1 && coefficients[0].is_zero();
}

void Polynom::monic() {
    Element lead_coeff_inv = this->coefficients[this->degree()].inverse();
    std::vector<Element> new_coeffs = this->coefficients;
    for(auto& coeff : new_coeffs){
        coeff *= lead_coeff_inv;
    }
    this->coefficients = std::move(new_coeffs);
}



std::vector<std::string> Polynom::repr() const {
    std::vector<std::string> representation;
    for(const auto& fe : coefficients){
        representation.push_back(fe.to_block());
    }
    return representation;
}


Polynom Polynom::random(int degree) {
    Polynom result;
    result.coefficients.reserve(degree);

    for (int i = 0; i < degree; ++i) {
        result.coefficients.push_back(Element::random());
    }

    return result;
}
