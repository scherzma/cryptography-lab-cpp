#include "base64_utils.h"

// Lookup table for encoding
static const char encoding_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

// Lookup table for decoding
static const unsigned char decoding_table[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

std::string base64_encode(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return "";
    }

    size_t input_length = data.size();
    size_t output_length = 4 * ((input_length + 2) / 3);
    std::string encoded_data(output_length, 0);

    const unsigned char* input = data.data();
    size_t i = 0, j = 0;

    // Process 3 bytes at a time
    while (i < input_length) {
        uint32_t octet_a = i < input_length ? input[i++] : 0;
        uint32_t octet_b = i < input_length ? input[i++] : 0;
        uint32_t octet_c = i < input_length ? input[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 18) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 12) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 6) & 0x3F];
        encoded_data[j++] = encoding_table[triple & 0x3F];
    }

    // Add padding if necessary
    size_t mod = input_length % 3;
    if (mod > 0) {
        encoded_data[output_length - 1] = '=';
        if (mod == 1) {
            encoded_data[output_length - 2] = '=';
        }
    }

    return encoded_data;
}

std::vector<uint8_t> base64_decode(const std::string& base64_str) {
    if (base64_str.empty()) {
        return std::vector<uint8_t>();
    }

    const size_t len = base64_str.size();
    const size_t eq_last     = (len > 0 && base64_str[len - 1] == '=');
    const size_t eq_second   = (len > 1 && base64_str[len - 2] == '=');

    // Sum results directly
    const size_t padding = eq_last + eq_second;

    size_t input_length = base64_str.length();
    size_t output_length = ((input_length * 3) / 4) - padding;
    std::vector<uint8_t> decoded_data(output_length);

    size_t i = 0, j = 0;
    uint32_t sextet_a, sextet_b, sextet_c, sextet_d;
    uint32_t triple;

    while (i < input_length) {
        sextet_a = base64_str[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(base64_str[i++])];
        sextet_b = base64_str[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(base64_str[i++])];
        sextet_c = base64_str[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(base64_str[i++])];
        sextet_d = base64_str[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(base64_str[i++])];

        triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < output_length) decoded_data[j++] = triple & 0xFF;
    }

    return decoded_data;
}


std::vector<uint8_t> base64_decode_vector(const std::vector<uint8_t>& base64_vec) {
    if (base64_vec.empty()) {
        return std::vector<uint8_t>();
    }

    const size_t len = base64_vec.size();
    const size_t eq_last     = (len > 0 && base64_vec[len - 1] == '=');
    const size_t eq_second   = (len > 1 && base64_vec[len - 2] == '=');

    // Sum results directly
    const size_t padding = eq_last + eq_second;

    size_t input_length = base64_vec.size();
    size_t output_length = ((input_length * 3) / 4) - padding;
    std::vector<uint8_t> decoded_data(output_length);

    size_t i = 0, j = 0;
    uint32_t sextet_a, sextet_b, sextet_c, sextet_d;
    uint32_t triple;

    while (i < input_length) {
        sextet_a = base64_vec[i] == '=' ? 0 & i++ : decoding_table[base64_vec[i++]];
        sextet_b = base64_vec[i] == '=' ? 0 & i++ : decoding_table[base64_vec[i++]];
        sextet_c = base64_vec[i] == '=' ? 0 & i++ : decoding_table[base64_vec[i++]];
        sextet_d = base64_vec[i] == '=' ? 0 & i++ : decoding_table[base64_vec[i++]];

        triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < output_length) decoded_data[j++] = triple & 0xFF;
    }

    return decoded_data;
}


std::string base64_encode_16(const std::array<uint8_t, 16>& data) {
    std::string result(24, 0);

    // Process 16 bytes in groups of 3 (5 groups of 3 plus 1 byte)
    // No need for bounds checking or padding logic since size is fixed
    const uint8_t* input = data.data();
    char* output = result.data();

    // First 15 bytes (5 groups of 3)
    for (int i = 0; i < 15; i += 3) {
        uint32_t triple = (input[i] << 16) | (input[i+1] << 8) | input[i+2];

        *output++ = encoding_table[(triple >> 18) & 0x3F];
        *output++ = encoding_table[(triple >> 12) & 0x3F];
        *output++ = encoding_table[(triple >> 6) & 0x3F];
        *output++ = encoding_table[triple & 0x3F];
    }

    // Handle last byte
    uint32_t last = (input[15] << 16);
    *output++ = encoding_table[(last >> 18) & 0x3F];
    *output++ = encoding_table[(last >> 12) & 0x3F];
    *output++ = '=';
    *output = '=';

    return result;
}


std::string base64_encode_16_reversed(const std::array<uint8_t, 16>& data) {
    std::string result(24, 0);
    const uint8_t* input = data.data();
    char* output = result.data();

    // First 15 bytes (5 groups of 3), starting from the end
    for (int i = 15; i > 0; i -= 3) {
        uint32_t triple = (input[i] << 16) | (input[i-1] << 8) | input[i-2];

        *output++ = encoding_table[(triple >> 18) & 0x3F];
        *output++ = encoding_table[(triple >> 12) & 0x3F];
        *output++ = encoding_table[(triple >> 6) & 0x3F];
        *output++ = encoding_table[triple & 0x3F];
    }

    // Handle first byte (was last byte in original order)
    uint32_t last = (input[0] << 16);
    *output++ = encoding_table[(last >> 18) & 0x3F];
    *output++ = encoding_table[(last >> 12) & 0x3F];
    *output++ = '=';
    *output = '=';

    return result;
}

// Fast Base64 decoding for exactly 24 characters to 16 bytes
// Input must be exactly 24 characters with proper padding
std::array<uint8_t, 16> base64_decode_16(const std::string& base64_str) {
    std::array<uint8_t, 16> result;
    uint8_t* output = result.data();
    const char* input = base64_str.data();

    // Process first 15 bytes (5 groups of 3)
    for (int i = 0; i < 20; i += 4) {
        uint32_t triple = (decoding_table[input[i]] << 18) |
                         (decoding_table[input[i+1]] << 12) |
                         (decoding_table[input[i+2]] << 6) |
                          decoding_table[input[i+3]];

        *output++ = (triple >> 16) & 0xFF;
        *output++ = (triple >> 8) & 0xFF;
        *output++ = triple & 0xFF;
    }

    // Handle last byte (with padding)
    uint32_t last = (decoding_table[input[20]] << 18) |
                    (decoding_table[input[21]] << 12);
    *output = (last >> 16) & 0xFF;

    return result;
}

std::array<uint8_t, 16> base64_decode_16_array(const std::array<uint8_t, 24>& base64_array) {
    std::array<uint8_t, 16> result;
    uint8_t* output = result.data();
    const uint8_t* input = base64_array.data();

    // Process first 15 bytes (5 groups of 3)
    for (int i = 0; i < 20; i += 4) {
        uint32_t triple = (decoding_table[input[i]] << 18) |
                         (decoding_table[input[i+1]] << 12) |
                         (decoding_table[input[i+2]] << 6) |
                          decoding_table[input[i+3]];

        *output++ = (triple >> 16) & 0xFF;
        *output++ = (triple >> 8) & 0xFF;
        *output++ = triple & 0xFF;
    }

    // Handle last byte (with padding)
    uint32_t last = (decoding_table[input[20]] << 18) |
                    (decoding_table[input[21]] << 12);
    *output = (last >> 16) & 0xFF;

    return result;
}


std::array<uint8_t, 16> base64_decode_16_reversed(const std::string& base64_str) {
    std::array<uint8_t, 16> result;
    const char* input = base64_str.data();

    // Triple #0 (i=0): normal out indices: [0,1,2] reversed: [15,14,13]
    {
        uint32_t triple = (decoding_table[input[0]] << 18) |
                          (decoding_table[input[1]] << 12) |
                          (decoding_table[input[2]] << 6)  |
                           decoding_table[input[3]];
        result[15] = (triple >> 16) & 0xFF;
        result[14] = (triple >> 8) & 0xFF;
        result[13] = triple & 0xFF;
    }

    // Triple #1 (i=4): normal out: [3,4,5] reversed: [12,11,10]
    {
        uint32_t triple = (decoding_table[input[4]] << 18) |
                          (decoding_table[input[5]] << 12) |
                          (decoding_table[input[6]] << 6)  |
                           decoding_table[input[7]];
        result[12] = (triple >> 16) & 0xFF;
        result[11] = (triple >> 8) & 0xFF;
        result[10] = triple & 0xFF;
    }

    // Triple #2 (i=8): normal out: [6,7,8] reversed: [9,8,7]
    {
        uint32_t triple = (decoding_table[input[8]] << 18) |
                          (decoding_table[input[9]] << 12) |
                          (decoding_table[input[10]] << 6) |
                           decoding_table[input[11]];
        result[9] = (triple >> 16) & 0xFF;
        result[8] = (triple >> 8) & 0xFF;
        result[7] = triple & 0xFF;
    }

    // Triple #3 (i=12): normal out: [9,10,11] reversed: [6,5,4]
    {
        uint32_t triple = (decoding_table[input[12]] << 18) |
                          (decoding_table[input[13]] << 12) |
                          (decoding_table[input[14]] << 6) |
                           decoding_table[input[15]];
        result[6] = (triple >> 16) & 0xFF;
        result[5] = (triple >> 8) & 0xFF;
        result[4] = triple & 0xFF;
    }

    // Triple #4 (i=16): normal out: [12,13,14] reversed: [3,2,1]
    {
        uint32_t triple = (decoding_table[input[16]] << 18) |
                          (decoding_table[input[17]] << 12) |
                          (decoding_table[input[18]] << 6) |
                           decoding_table[input[19]];
        result[3] = (triple >> 16) & 0xFF;
        result[2] = (triple >> 8) & 0xFF;
        result[1] = triple & 0xFF;
    }

    // Last partial (i=20): normal out: [15] reversed: [0]
    {
        uint32_t last = (decoding_table[input[20]] << 18) |
                        (decoding_table[input[21]] << 12);
        result[0] = (last >> 16) & 0xFF;
    }

    return result;
}

std::array<uint8_t, 16> base64_decode_16_reversed_array(const std::array<uint8_t, 24>& base64_array) {
    std::array<uint8_t, 16> result;
    const uint8_t* input = base64_array.data();

    // Triple #0 (i=0): normal out indices: [0,1,2] reversed: [15,14,13]
    {
        uint32_t triple = (decoding_table[input[0]] << 18) |
                          (decoding_table[input[1]] << 12) |
                          (decoding_table[input[2]] << 6)  |
                           decoding_table[input[3]];
        result[15] = (triple >> 16) & 0xFF;
        result[14] = (triple >> 8) & 0xFF;
        result[13] = triple & 0xFF;
    }

    // Triple #1 (i=4): normal out: [3,4,5] reversed: [12,11,10]
    {
        uint32_t triple = (decoding_table[input[4]] << 18) |
                          (decoding_table[input[5]] << 12) |
                          (decoding_table[input[6]] << 6)  |
                           decoding_table[input[7]];
        result[12] = (triple >> 16) & 0xFF;
        result[11] = (triple >> 8) & 0xFF;
        result[10] = triple & 0xFF;
    }

    // Triple #2 (i=8): normal out: [6,7,8] reversed: [9,8,7]
    {
        uint32_t triple = (decoding_table[input[8]] << 18) |
                          (decoding_table[input[9]] << 12) |
                          (decoding_table[input[10]] << 6) |
                           decoding_table[input[11]];
        result[9] = (triple >> 16) & 0xFF;
        result[8] = (triple >> 8) & 0xFF;
        result[7] = triple & 0xFF;
    }

    // Triple #3 (i=12): normal out: [9,10,11] reversed: [6,5,4]
    {
        uint32_t triple = (decoding_table[input[12]] << 18) |
                          (decoding_table[input[13]] << 12) |
                          (decoding_table[input[14]] << 6) |
                           decoding_table[input[15]];
        result[6] = (triple >> 16) & 0xFF;
        result[5] = (triple >> 8) & 0xFF;
        result[4] = triple & 0xFF;
    }

    // Triple #4 (i=16): normal out: [12,13,14] reversed: [3,2,1]
    {
        uint32_t triple = (decoding_table[input[16]] << 18) |
                          (decoding_table[input[17]] << 12) |
                          (decoding_table[input[18]] << 6) |
                           decoding_table[input[19]];
        result[3] = (triple >> 16) & 0xFF;
        result[2] = (triple >> 8) & 0xFF;
        result[1] = triple & 0xFF;
    }

    // Last partial (i=20): normal out: [15] reversed: [0]
    {
        uint32_t last = (decoding_table[input[20]] << 18) |
                        (decoding_table[input[21]] << 12);
        result[0] = (last >> 16) & 0xFF;
    }

    return result;
}