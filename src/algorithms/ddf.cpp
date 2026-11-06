//
// Created by user on 01/12/24.
//

#include "ddf.h"
#include <set>


Polynom powmod_128(const Polynom A, const Polynom& M, __uint128_t k) {
    Polynom base = A % M;

    for(int i=0; i<128 * k; ++i){
        base *= base;
        base = base % M;
    }

    return base;
}


std::set<std::tuple<Polynom, int>> ddf(const Polynom& f) {
    std::set<std::tuple<Polynom, int>> z;

    __uint128_t d = 1;
    Polynom fstar = f;
    Polynom one = Polynom({Element::ONE});
    Polynom zero = Polynom({Element::ZERO});
    Polynom X = Polynom({Element::ZERO, Element::ONE});

    while(fstar.degree() >= 2*d) {
        Polynom h = zero;
        h = powmod_128(X, fstar, d);
        h += X;
        h = h % fstar;
        Polynom g = h.gcd(fstar);
        if(g != one){
            z.emplace(g, d);
            fstar = fstar / g;
        }
        d += 1;
    }

    if(fstar != one){
        z.emplace(fstar, fstar.degree());
    } else if (z.empty()) {
        z.emplace(f, 1);
    }

    return z;
}
