//
// Created by user on 03/12/24.
//

#ifndef GCM_CRACK_H
#define GCM_CRACK_H
#include <cstdint>
#include <string>
#include <vector>
#include <math/Polynom.h>

#include "math/Element.h"


std::tuple<std::string, std::string, std::string> gcm_crack(
    std::vector<uint8_t> nonce,
    const GCMMessage& m1,
    const GCMMessage& m2,
    const GCMMessage& m3,
    const GCMMessage& forgery);


#endif //GCM_CRACK_H
