// src/util/base64_utils.h
#ifndef BASE64_UTILS_H
#define BASE64_UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <array>

// Encode bytes to base64 string
std::string base64_encode(const std::vector<uint8_t>& data);

// Decode base64 string to bytes
std::vector<uint8_t> base64_decode(const std::string& base64_str);
std::vector<uint8_t> base64_decode_vector(const std::vector<uint8_t>& base64_vec);

std::string base64_encode_16(const std::array<uint8_t, 16>& data);
std::string base64_encode_16_reversed(const std::array<uint8_t, 16>& data);

std::array<uint8_t, 16> base64_decode_16(const std::string& base64_str);
std::array<uint8_t, 16> base64_decode_16_array(const std::array<uint8_t, 24>& base64_array);
std::array<uint8_t, 16> base64_decode_16_reversed(const std::string& base64_str);
std::array<uint8_t, 16> base64_decode_16_reversed_array(const std::array<uint8_t, 24>& base64_array);
#endif // BASE64_UTILS_H
