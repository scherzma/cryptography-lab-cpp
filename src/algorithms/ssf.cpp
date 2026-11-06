//
// Created by user on 01/12/24.
//

#include "ssf.h"

#include <set>

std::set<std::tuple<Polynom, int>> sff(const Polynom& fx) {
    Polynom one = Polynom({Element::ONE});
    Polynom f =  fx;
    f.monic();

    Polynom c = f.gcd(f.diff());
    f = f / c;
    std::set<std::tuple<Polynom, int>> z;
    int e = 1;

    while (f != one) {
        Polynom y = f.gcd(c);
        if(f != y){
            z.emplace(f/y, e);
        }
        f = y;
        c = c / y;
        e = e + 1;
    }

    if(c != one) {
        for(const auto& [t, exponent] : sff(c.sqrt())){
            z.emplace(t, exponent + exponent);
        }
    }
    return z;
}
