#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include <string>
#include <vector>
#include <cstdint>

struct GCMMessage {
    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> associated_data;
    std::vector<uint8_t> tag;
};


enum class Semantic {
    GCM,
    XEX,
};

inline Semantic from_string(const std::string& s) {
    if (s == "gcm") {
        return Semantic::GCM;
    }

    return Semantic::XEX;
}

inline Semantic from_char(char s) {
    if (s == 'g') {
        return Semantic::GCM;
    }

    return Semantic::XEX;
}

#endif