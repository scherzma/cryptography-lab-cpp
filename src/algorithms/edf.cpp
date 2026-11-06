//
// Created by User on 2024-12-12.
//

#include "edf.h"

Polynom powmod_128_div3(const Polynom& A, const Polynom& M, __uint128_t k) {
    Polynom result({Element::ONE});

    for(int i=0; i<((128 * k) - 1) / 2; ++i){
        result *= A;
        result = result % M;
        result *= result;
        result = result % M;
        result *= result; // result = result % M;
    }
    result *= A;
    result = result % M;
    return result;
}


std::vector<Polynom> edf(const Polynom& f, int d) {
    Polynom one = Polynom({Element::ONE});
    Polynom h = Polynom();

    constexpr size_t MAX_SIZE = 20;
    Polynom arr[MAX_SIZE];
    size_t count = 1;
    size_t counter = 1;

    arr[0] = f;
    __uint128_t n = (f.degree()) / d;
    while(count < n) {
        if (counter > 4) {
            h = Polynom::random(f.degree());
        } else {
            h = Polynom::random(2); // f.degree());
        }
        Polynom g = powmod_128_div3(h, f, d) + one;

        for(size_t i = 0; i < count; i++) {
            if (arr[i].degree() > d) {
                Polynom j = arr[i].gcd(g);
                if (j != one && j != arr[i]) {
                    Polynom current = arr[i];
                    arr[i] = arr[count - 1];
                    i--;
                    count--;
                    arr[count++] = j;
                    arr[count++] = current / j;
                }
            }
        }
        counter++;
    }

    // Copy to vector for return
    return std::vector<Polynom>(arr, arr + count);
}